// Server-authoritative form backend for the JobsMod VPP administration menu.
// The administrator never edits raw JSON. This plugin loads the real JobsMod
// JSON data classes, exposes selected fields to the client and writes them back
// through JobsModJsonFileIO.

class JobsModAdminPlugin extends PluginBase
{
	protected static const string ROOT = "$profile:JobsMod";
	protected static const string SETTINGS_FILE = ROOT + "/settings.json";
	protected static const string JOBS_DIR = ROOT + "/Jobs";
	protected static const string NPC_DIR = ROOT + "/NPC";
	protected static const string BACKUP_ROOT = ROOT + "/AdminBackups";

	protected static const string PERMISSION_MENU = "MenuJobsModAdmin";
	protected static const string PERMISSION_SAVE = "MenuJobsModAdmin:Save";
	protected static const string PERMISSION_APPLY = "MenuJobsModAdmin:Apply";

	protected static const int JOBSMOD_CF_RPC_ID = 10042;

	protected void SendJobsModAdminClientRpc(PlayerIdentity recipient, string functionName, Param payload)
	{
		if (!recipient)
			return;

		array<ref Param> rpcData = new array<ref Param>;
		rpcData.Insert(new Param2<string, string>("RPC_JobsModAdminClient", functionName));

		if (payload)
			rpcData.Insert(payload);

		GetGame().RPC(NULL, JOBSMOD_CF_RPC_ID, rpcData, true, recipient);
	}

	void JobsModAdminPlugin()
	{
		MakeDirectory(BACKUP_ROOT);
		MakeDirectory(JOBS_DIR);
		MakeDirectory(NPC_DIR);

		PermissionManager permissionManager = GetPermissionManager();
		if (permissionManager)
		{
			ref array<string> jobsModPermissions = new array<string>;
			jobsModPermissions.Insert(PERMISSION_MENU);
			jobsModPermissions.Insert(PERMISSION_SAVE);
			jobsModPermissions.Insert(PERMISSION_APPLY);
			permissionManager.AddPermissionType(jobsModPermissions);
		}

		GetRPCManager().AddRPC("RPC_JobsModAdmin", "GetCatalog", this, SingeplayerExecutionType.Server);
		GetRPCManager().AddRPC("RPC_JobsModAdmin", "LoadSettingsForm", this, SingeplayerExecutionType.Server);
		GetRPCManager().AddRPC("RPC_JobsModAdmin", "LoadJobForm", this, SingeplayerExecutionType.Server);
		GetRPCManager().AddRPC("RPC_JobsModAdmin", "LoadNpcForm", this, SingeplayerExecutionType.Server);
		GetRPCManager().AddRPC("RPC_JobsModAdmin", "LoadWorldForm", this, SingeplayerExecutionType.Server);
		GetRPCManager().AddRPC("RPC_JobsModAdmin", "SaveSettingsForm", this, SingeplayerExecutionType.Server);
		GetRPCManager().AddRPC("RPC_JobsModAdmin", "SaveJobForm", this, SingeplayerExecutionType.Server);
		GetRPCManager().AddRPC("RPC_JobsModAdmin", "SaveNpcForm", this, SingeplayerExecutionType.Server);
		GetRPCManager().AddRPC("RPC_JobsModAdmin", "SaveWorldForm", this, SingeplayerExecutionType.Server);
		GetRPCManager().AddRPC("RPC_JobsModAdmin", "CreateRecord", this, SingeplayerExecutionType.Server);
		GetRPCManager().AddRPC("RPC_JobsModAdmin", "ApplyConfig", this, SingeplayerExecutionType.Server);

		Print("[JobsMod/Admin] VPP forms plugin registered.");
	}

	protected bool HasPermission(PlayerIdentity sender, string permission)
	{
		if (!sender)
			return false;

		PermissionManager manager = GetPermissionManager();
		if (!manager)
			return false;

		return manager.VerifyPermission(sender.GetPlainId(), permission, "", false);
	}

	protected bool IsAllowedFileName(string fileName)
	{
		if (fileName == "")
			return false;

		if (fileName.IndexOf("..") != -1)
			return false;

		if (fileName.IndexOf("/") != -1)
			return false;

		if (fileName.IndexOf("\\") != -1)
			return false;

		if (fileName.IndexOf(":") != -1)
			return false;

		int extensionIndex = fileName.IndexOf(".json");
		if (extensionIndex == -1)
			return false;

		return extensionIndex == fileName.Length() - 5;
	}

	protected bool IsSafeRecordId(string id)
	{
		if (id == "")
			return false;

		if (id.Length() > 64)
			return false;

		string allowed = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-";
		for (int i = 0; i < id.Length(); i++)
		{
			if (allowed.IndexOf(id.Get(i)) < 0)
				return false;
		}

		return true;
	}

	protected string GetField(array<string> fields, string key)
	{
		for (int i = 0; i + 1 < fields.Count(); i = i + 2)
		{
			if (fields.Get(i) == key)
				return fields.Get(i + 1);
		}

		return "";
	}

	protected void GetRepeated(array<string> fields, string key, out array<string> values)
	{
		values = new array<string>;

		for (int i = 0; i + 1 < fields.Count(); i = i + 2)
		{
			if (fields.Get(i) == key)
				values.Insert(fields.Get(i + 1));
		}
	}

	protected bool ToBool(string value)
	{
		return value == "1" || value == "true";
	}

	protected void PushField(out array<string> fields, string key, string value)
	{
		fields.Insert(key);
		fields.Insert(value);
	}

	protected void PushBool(out array<string> fields, string key, bool value)
	{
		if (value)
			PushField(fields, key, "true");
		else
			PushField(fields, key, "false");
	}

	protected void PushInt(out array<string> fields, string key, int value)
	{
		PushField(fields, key, value.ToString());
	}

	protected void PushFloat(out array<string> fields, string key, float value)
	{
		PushField(fields, key, value.ToString());
	}

	protected void PushList(out array<string> fields, string key, array<string> values)
	{
		if (!values)
			return;

		foreach (string value : values)
			PushField(fields, key, value);
	}

	protected void Answer(PlayerIdentity sender, string text)
	{
		if (!sender)
			return;

		SendJobsModAdminClientRpc(
			sender,
			"OnAdminMessage",
			new Param1<string>(text)
		);
	}

	protected void SendForm(PlayerIdentity sender, string formType, string key, array<string> fields)
	{
		SendJobsModAdminClientRpc(
			sender,
			"OnForm",
			new Param3<string, string, ref array<string>>(formType, key, fields)
		);
	}

	protected void BackupFile(string path, string label)
	{
		int year;
		int month;
		int day;
		int hour;
		int minute;
		int second;

		GetYearMonthDayUTC(year, month, day);
		GetHourMinuteSecondUTC(hour, minute, second);

		string stamp = year.ToString();
		stamp = stamp + "-" + month.ToString();
		stamp = stamp + "-" + day.ToString();
		stamp = stamp + "_" + hour.ToString();
		stamp = stamp + "-" + minute.ToString();
		stamp = stamp + "-" + second.ToString();

		PluginFileHandler.FileDuplicate(path, BACKUP_ROOT + "/" + stamp + "_" + label);
	}

	void GetCatalog(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server)
			return;

		if (!HasPermission(sender, PERMISSION_MENU))
			return;

		array<string> catalog = new array<string>;
		array<string> files;

		JobsModJsonFileIO.ListJsonFiles(JOBS_DIR, files);
		foreach (string jobFile : files)
		{
			catalog.Insert("job");
			catalog.Insert(jobFile);
		}

		JobsModJsonFileIO.ListJsonFiles(NPC_DIR, files);
		foreach (string npcFile : files)
		{
			catalog.Insert("npc");
			catalog.Insert(npcFile);
		}

		JobsModSettingsJson settings;
		string error;
		if (JobsModJsonFileIO.LoadSettings(SETTINGS_FILE, settings, error) && settings)
			AppendWorldCatalog(settings, catalog);

		SendJobsModAdminClientRpc(
			sender,
			"OnCatalog",
			new Param1<ref array<string>>(catalog)
		);
	}

	protected void AppendWorldCatalog(JobsModSettingsJson settings, out array<string> catalog)
	{
		if (settings.zones)
		{
			foreach (JobsModZoneJson zone : settings.zones)
				AppendWorldCatalogEntry(catalog, "zone", zone.id, zone.name);
		}

		if (settings.pile_points)
		{
			foreach (JobsModPilePointJson pile : settings.pile_points)
				AppendWorldCatalogEntry(catalog, "pile", pile.id, pile.name);
		}

		if (settings.loader_areas)
		{
			foreach (JobsModLoaderAreaJson area : settings.loader_areas)
				AppendWorldCatalogEntry(catalog, "loader", area.id, area.name);
		}

		if (settings.guard_posts)
		{
			foreach (JobsModGuardPostJson post : settings.guard_posts)
				AppendWorldCatalogEntry(catalog, "guard", post.id, post.name);
		}

		if (settings.equipment_lockers)
		{
			foreach (JobsModLockerJson locker : settings.equipment_lockers)
				AppendWorldCatalogEntry(catalog, "locker", locker.id, locker.name);
		}
	}

	protected void AppendWorldCatalogEntry(out array<string> catalog, string worldType, string id, string name)
	{
		catalog.Insert("world");
		catalog.Insert(worldType);
		catalog.Insert(id);
		catalog.Insert(name);
	}

	void LoadSettingsForm(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server)
			return;

		if (!HasPermission(sender, PERMISSION_MENU))
			return;

		JobsModSettingsJson settings;
		string error;
		if (!JobsModJsonFileIO.LoadSettings(SETTINGS_FILE, settings, error) || !settings)
		{
			Answer(sender, "settings.json не загружен: " + error);
			return;
		}

		array<string> fields = new array<string>;
		PushInt(fields, "pile_respawn_seconds", settings.pile_respawn_seconds);
		PushInt(fields, "assignment_timeout_seconds", settings.assignment_timeout_seconds);
		PushBool(fields, "debug_logging", settings.debug_logging);
		PushField(fields, "reward_class", settings.reward_class);
		SendForm(sender, "settings", "settings", fields);
	}

	void LoadJobForm(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server)
			return;

		if (!HasPermission(sender, PERMISSION_MENU))
			return;

		Param1<string> request;
		if (!ctx.Read(request))
			return;

		if (!IsAllowedFileName(request.param1))
			return;

		JobsModJobJson job;
		string error;
		if (!JobsModJsonFileIO.LoadJob(JOBS_DIR + "/" + request.param1, job, error) || !job)
		{
			Answer(sender, "Работа не загружена: " + error);
			return;
		}

		array<string> fields = JobToFields(job);
		SendForm(sender, "job", request.param1, fields);
	}

	protected array<string> JobToFields(JobsModJobJson job)
	{
		array<string> fields = new array<string>;
		PushField(fields, "id", job.id);
		PushField(fields, "name", job.name);
		PushField(fields, "description", job.description);
		PushField(fields, "type", job.type);
		PushField(fields, "zone_id", job.zone_id);
		PushInt(fields, "reward", job.reward);
		PushInt(fields, "cooldown_seconds", job.cooldown_seconds);
		PushInt(fields, "piles_required", job.piles_required);
		PushField(fields, "loader_area_id", job.loader_area_id);
		PushInt(fields, "cargos_required", job.cargos_required);
		PushField(fields, "cargo_class", job.cargo_class);
		PushField(fields, "target_npc_id", job.target_npc_id);
		PushField(fields, "package_class", job.package_class);
		PushField(fields, "guard_post_id", job.guard_post_id);
		PushInt(fields, "guard_seconds", job.guard_seconds);
		PushList(fields, "equipment", job.equipment);
		PushList(fields, "collect_classes", job.collect_classes);
		PushInt(fields, "collect_required", job.collect_required);
		PushField(fields, "collect_label", job.collect_label);
		PushField(fields, "equipment_locker_id", job.equipment_locker_id);
		PushInt(fields, "equipment_fine", job.equipment_fine);
		return fields;
	}

	void LoadNpcForm(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server)
			return;

		if (!HasPermission(sender, PERMISSION_MENU))
			return;

		Param1<string> request;
		if (!ctx.Read(request))
			return;

		if (!IsAllowedFileName(request.param1))
			return;

		JobsModNpcJson npc;
		string error;
		if (!JobsModJsonFileIO.LoadNpc(NPC_DIR + "/" + request.param1, npc, error) || !npc)
		{
			Answer(sender, "NPC не загружен: " + error);
			return;
		}

		array<string> fields = NpcToFields(npc);
		SendForm(sender, "npc", request.param1, fields);
	}

	protected array<string> NpcToFields(JobsModNpcJson npc)
	{
		array<string> fields = new array<string>;
		PushField(fields, "id", npc.id);
		PushField(fields, "name", npc.name);
		PushField(fields, "description", npc.description);
		PushField(fields, "position", npc.position);
		PushFloat(fields, "rotation", npc.rotation);
		PushField(fields, "player_class", npc.player_class);
		PushBool(fields, "invulnerable", npc.invulnerable);
		PushList(fields, "clothing", npc.clothing);
		PushList(fields, "jobs", npc.jobs);
		return fields;
	}

	void LoadWorldForm(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server)
			return;

		if (!HasPermission(sender, PERMISSION_MENU))
			return;

		Param2<string, string> request;
		if (!ctx.Read(request))
			return;

		JobsModSettingsJson settings;
		string error;
		if (!JobsModJsonFileIO.LoadSettings(SETTINGS_FILE, settings, error) || !settings)
		{
			Answer(sender, "settings.json не загружен: " + error);
			return;
		}

		array<string> fields = WorldToFields(settings, request.param1, request.param2);
		if (!fields)
		{
			Answer(sender, "Запись мира не найдена: " + request.param1 + "/" + request.param2);
			return;
		}

		SendForm(sender, "world", request.param1 + "/" + request.param2, fields);
	}

	protected array<string> WorldToFields(JobsModSettingsJson settings, string worldType, string id)
	{
		array<string> fields = new array<string>;
		PushField(fields, "world_type", worldType);
		PushField(fields, "id", id);

		if (worldType == "zone" && settings.zones)
		{
			foreach (JobsModZoneJson zone : settings.zones)
			{
				if (zone.id == id)
				{
					PushField(fields, "name", zone.name);
					return fields;
				}
			}
		}

		if (worldType == "pile" && settings.pile_points)
		{
			foreach (JobsModPilePointJson pile : settings.pile_points)
			{
				if (pile.id == id)
				{
					PushField(fields, "name", pile.name);
					PushField(fields, "zone_id", pile.zone_id);
					PushField(fields, "position", pile.position);
					return fields;
				}
			}
		}

		if (worldType == "loader" && settings.loader_areas)
		{
			foreach (JobsModLoaderAreaJson area : settings.loader_areas)
			{
				if (area.id == id)
				{
					PushField(fields, "name", area.name);
					PushField(fields, "zone_id", area.zone_id);
					PushField(fields, "source", area.source);
					PushFloat(fields, "source_radius", area.source_radius);
					PushField(fields, "destination", area.destination);
					PushFloat(fields, "destination_radius", area.destination_radius);
					return fields;
				}
			}
		}

		if (worldType == "guard" && settings.guard_posts)
		{
			foreach (JobsModGuardPostJson post : settings.guard_posts)
			{
				if (post.id == id)
				{
					PushField(fields, "name", post.name);
					PushField(fields, "zone_id", post.zone_id);
					PushField(fields, "position", post.position);
					PushFloat(fields, "radius", post.radius);
					return fields;
				}
			}
		}

		if (worldType == "locker" && settings.equipment_lockers)
		{
			foreach (JobsModLockerJson locker : settings.equipment_lockers)
			{
				if (locker.id == id)
				{
					PushField(fields, "name", locker.name);
					PushField(fields, "position", locker.position);
					PushFloat(fields, "rotation", locker.rotation);
					PushField(fields, "class_name", locker.class_name);
					return fields;
				}
			}
		}

		return null;
	}

	void SaveSettingsForm(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server)
			return;

		if (!HasPermission(sender, PERMISSION_SAVE))
			return;

		Param1<ref array<string>> request;
		if (!ctx.Read(request) || !request.param1)
			return;

		JobsModSettingsJson settings;
		string error;
		if (!JobsModJsonFileIO.LoadSettings(SETTINGS_FILE, settings, error) || !settings)
		{
			Answer(sender, "settings.json не загружен: " + error);
			return;
		}

		settings.pile_respawn_seconds = GetField(request.param1, "pile_respawn_seconds").ToInt();
		settings.assignment_timeout_seconds = GetField(request.param1, "assignment_timeout_seconds").ToInt();
		settings.debug_logging = ToBool(GetField(request.param1, "debug_logging"));
		settings.reward_class = GetField(request.param1, "reward_class");

		BackupFile(SETTINGS_FILE, "settings.json");
		if (!JobsModJsonFileIO.SaveSettings(SETTINGS_FILE, settings, error))
		{
			Answer(sender, "Не удалось сохранить settings.json: " + error);
			return;
		}

		HotApplySaved(sender, "settings.json");
	}

	void SaveJobForm(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server)
			return;

		if (!HasPermission(sender, PERMISSION_SAVE))
			return;

		Param2<string, ref array<string>> request;
		if (!ctx.Read(request) || !request.param2)
			return;

		if (!IsAllowedFileName(request.param1))
			return;

		string path = JOBS_DIR + "/" + request.param1;
		JobsModJobJson job;
		string error;
		if (!JobsModJsonFileIO.LoadJob(path, job, error) || !job)
		{
			Answer(sender, "Работа не загружена: " + error);
			return;
		}

		array<string> fields = request.param2;
		job.name = GetField(fields, "name");
		job.description = GetField(fields, "description");
		job.type = GetField(fields, "type");
		job.zone_id = GetField(fields, "zone_id");
		job.reward = GetField(fields, "reward").ToInt();
		job.cooldown_seconds = GetField(fields, "cooldown_seconds").ToInt();
		job.equipment_locker_id = GetField(fields, "equipment_locker_id");
		job.equipment_fine = GetField(fields, "equipment_fine").ToInt();
		GetRepeated(fields, "equipment", job.equipment);

		if (job.type == "sorting")
			job.piles_required = GetField(fields, "piles_required").ToInt();
		else if (job.type == "loading")
		{
			job.loader_area_id = GetField(fields, "loader_area_id");
			job.cargos_required = GetField(fields, "cargos_required").ToInt();
			job.cargo_class = GetField(fields, "cargo_class");
		}
		else if (job.type == "messenger")
		{
			job.target_npc_id = GetField(fields, "target_npc_id");
			job.package_class = GetField(fields, "package_class");
		}
		else if (job.type == "guard")
		{
			job.guard_post_id = GetField(fields, "guard_post_id");
			job.guard_seconds = GetField(fields, "guard_seconds").ToInt();
		}
		else if (job.type == "collect")
		{
			job.collect_required = GetField(fields, "collect_required").ToInt();
			job.collect_label = GetField(fields, "collect_label");
			GetRepeated(fields, "collect_classes", job.collect_classes);
		}

		BackupFile(path, request.param1);
		if (!JobsModJsonFileIO.SaveJob(path, job, error))
		{
			Answer(sender, "Не удалось сохранить работу: " + error);
			return;
		}

		HotApplySaved(sender, "Jobs/" + request.param1);
	}

	void SaveNpcForm(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server)
			return;

		if (!HasPermission(sender, PERMISSION_SAVE))
			return;

		Param2<string, ref array<string>> request;
		if (!ctx.Read(request) || !request.param2)
			return;

		if (!IsAllowedFileName(request.param1))
			return;

		string path = NPC_DIR + "/" + request.param1;
		JobsModNpcJson npc;
		string error;
		if (!JobsModJsonFileIO.LoadNpc(path, npc, error) || !npc)
		{
			Answer(sender, "NPC не загружен: " + error);
			return;
		}

		array<string> fields = request.param2;
		npc.name = GetField(fields, "name");
		npc.description = GetField(fields, "description");
		npc.position = GetField(fields, "position");
		npc.rotation = GetField(fields, "rotation").ToFloat();
		npc.player_class = GetField(fields, "player_class");
		npc.invulnerable = ToBool(GetField(fields, "invulnerable"));
		GetRepeated(fields, "clothing", npc.clothing);
		GetRepeated(fields, "jobs", npc.jobs);

		BackupFile(path, request.param1);
		if (!JobsModJsonFileIO.SaveNpc(path, npc, error))
		{
			Answer(sender, "Не удалось сохранить NPC: " + error);
			return;
		}

		HotApplySaved(sender, "NPC/" + request.param1);
	}

	void SaveWorldForm(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server)
			return;

		if (!HasPermission(sender, PERMISSION_SAVE))
			return;

		Param3<string, string, ref array<string>> request;
		if (!ctx.Read(request) || !request.param3)
			return;

		JobsModSettingsJson settings;
		string error;
		if (!JobsModJsonFileIO.LoadSettings(SETTINGS_FILE, settings, error) || !settings)
		{
			Answer(sender, "settings.json не загружен: " + error);
			return;
		}

		if (!UpdateWorldRecord(settings, request.param1, request.param2, request.param3))
		{
			Answer(sender, "Запись мира не найдена: " + request.param1 + "/" + request.param2);
			return;
		}

		BackupFile(SETTINGS_FILE, "settings.json");
		if (!JobsModJsonFileIO.SaveSettings(SETTINGS_FILE, settings, error))
		{
			Answer(sender, "Не удалось сохранить settings.json: " + error);
			return;
		}

		HotApplySaved(sender, "settings.json → " + request.param1 + "/" + request.param2);
	}

	protected bool UpdateWorldRecord(JobsModSettingsJson settings, string worldType, string id, array<string> fields)
	{
		if (worldType == "zone" && settings.zones)
		{
			foreach (JobsModZoneJson zone : settings.zones)
			{
				if (zone.id == id)
				{
					zone.name = GetField(fields, "name");
					return true;
				}
			}
		}

		if (worldType == "pile" && settings.pile_points)
		{
			foreach (JobsModPilePointJson pile : settings.pile_points)
			{
				if (pile.id == id)
				{
					pile.name = GetField(fields, "name");
					pile.zone_id = GetField(fields, "zone_id");
					pile.position = GetField(fields, "position");
					return true;
				}
			}
		}

		if (worldType == "loader" && settings.loader_areas)
		{
			foreach (JobsModLoaderAreaJson area : settings.loader_areas)
			{
				if (area.id == id)
				{
					area.name = GetField(fields, "name");
					area.zone_id = GetField(fields, "zone_id");
					area.source = GetField(fields, "source");
					area.source_radius = GetField(fields, "source_radius").ToFloat();
					area.destination = GetField(fields, "destination");
					area.destination_radius = GetField(fields, "destination_radius").ToFloat();
					return true;
				}
			}
		}

		if (worldType == "guard" && settings.guard_posts)
		{
			foreach (JobsModGuardPostJson post : settings.guard_posts)
			{
				if (post.id == id)
				{
					post.name = GetField(fields, "name");
					post.zone_id = GetField(fields, "zone_id");
					post.position = GetField(fields, "position");
					post.radius = GetField(fields, "radius").ToFloat();
					return true;
				}
			}
		}

		if (worldType == "locker" && settings.equipment_lockers)
		{
			foreach (JobsModLockerJson locker : settings.equipment_lockers)
			{
				if (locker.id == id)
				{
					locker.name = GetField(fields, "name");
					locker.position = GetField(fields, "position");
					locker.rotation = GetField(fields, "rotation").ToFloat();
					locker.class_name = GetField(fields, "class_name");
					return true;
				}
			}
		}

		return false;
	}

	protected void HotApplySaved(PlayerIdentity sender, string target)
	{
		JobsLog.Info("SERVER/ADMIN: '" + sender.GetName() + "' сохранил " + target + ".");

		if (!JobsModServerRuntime.ReloadConfig())
		{
			Answer(sender, "Сохранено: " + target + ", но горячее применение отклонено. Старая runtime-конфигурация продолжает работать; смотри RPT.");
			return;
		}

		JobsLog.Info("SERVER/ADMIN: '" + target + "' горячо применён без рестарта сервера.");
		Answer(sender, "Сохранено и сразу применено: " + target + ".");
	}

	void CreateRecord(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server)
			return;

		if (!HasPermission(sender, PERMISSION_SAVE))
			return;

		Param4<int, string, string, string> request;
		if (!ctx.Read(request))
			return;

		string id = request.param3;
		if (!IsSafeRecordId(id))
		{
			Answer(sender, "ID разрешает только A-Z, a-z, 0-9, _ и - (до 64 символов).");
			return;
		}

		if (request.param1 == 1)
		{
			CreateJobRecord(sender, id);
			return;
		}

		if (request.param1 == 2)
		{
			CreateNpcRecord(sender, id, request.param4);
			return;
		}

		if (request.param1 == 3)
			CreateWorldRecord(sender, request.param2, id, request.param4);
	}

	protected string FirstZoneId(JobsModSettingsJson settings)
	{
		if (!settings || !settings.zones)
			return "";

		if (settings.zones.Count() == 0)
			return "";

		return settings.zones.Get(0).id;
	}

	protected void CreateJobRecord(PlayerIdentity sender, string id)
	{
		string fileName = id + ".json";
		string path = JOBS_DIR + "/" + fileName;
		if (FileExist(path))
		{
			Answer(sender, "Работа уже существует: " + fileName);
			return;
		}

		JobsModSettingsJson settings;
		string settingsError;
		JobsModJsonFileIO.LoadSettings(SETTINGS_FILE, settings, settingsError);

		JobsModJobJson job = new JobsModJobJson();
		job.id = id;
		job.name = id;
		job.description = "";
		job.type = "sorting";
		job.zone_id = FirstZoneId(settings);
		job.reward = 0;
		job.cooldown_seconds = 0;
		job.piles_required = 1;
		job.equipment = new array<string>;
		job.collect_classes = new array<string>;

		string error;
		if (!JobsModJsonFileIO.SaveJob(path, job, error))
		{
			Answer(sender, "Не удалось создать работу: " + error);
			return;
		}

		SendForm(sender, "job", fileName, JobToFields(job));
		HotApplySaved(sender, "Jobs/" + fileName);
	}

	protected void CreateNpcRecord(PlayerIdentity sender, string id, string position)
	{
		string fileName = id + ".json";
		string path = NPC_DIR + "/" + fileName;
		if (FileExist(path))
		{
			Answer(sender, "NPC уже существует: " + fileName);
			return;
		}

		JobsModNpcJson npc = new JobsModNpcJson();
		npc.id = id;
		npc.name = id;
		npc.description = "";
		npc.position = position;
		npc.rotation = 0.0;
		npc.player_class = "SurvivorM_Mirek";
		npc.invulnerable = true;
		npc.clothing = new array<string>;
		npc.jobs = new array<string>;

		string error;
		if (!JobsModJsonFileIO.SaveNpc(path, npc, error))
		{
			Answer(sender, "Не удалось создать NPC: " + error);
			return;
		}

		SendForm(sender, "npc", fileName, NpcToFields(npc));
		HotApplySaved(sender, "NPC/" + fileName);
	}

	protected void CreateWorldRecord(PlayerIdentity sender, string worldType, string id, string position)
	{
		JobsModSettingsJson settings;
		string error;
		if (!JobsModJsonFileIO.LoadSettings(SETTINGS_FILE, settings, error) || !settings)
		{
			Answer(sender, "settings.json не загружен: " + error);
			return;
		}

		array<string> existingFields = WorldToFields(settings, worldType, id);
		if (existingFields)
		{
			Answer(sender, "Запись мира уже существует: " + worldType + "/" + id);
			return;
		}

		string defaultZone = FirstZoneId(settings);

		if (worldType == "zone")
		{
			if (!settings.zones)
				settings.zones = new array<ref JobsModZoneJson>;

			JobsModZoneJson zone = new JobsModZoneJson();
			zone.id = id;
			zone.name = id;
			settings.zones.Insert(zone);
		}
		else if (worldType == "pile")
		{
			if (!settings.pile_points)
				settings.pile_points = new array<ref JobsModPilePointJson>;

			JobsModPilePointJson pile = new JobsModPilePointJson();
			pile.id = id;
			pile.zone_id = defaultZone;
			pile.name = id;
			pile.position = position;
			settings.pile_points.Insert(pile);
		}
		else if (worldType == "loader")
		{
			if (!settings.loader_areas)
				settings.loader_areas = new array<ref JobsModLoaderAreaJson>;

			JobsModLoaderAreaJson area = new JobsModLoaderAreaJson();
			area.id = id;
			area.name = id;
			area.zone_id = defaultZone;
			area.source = position;
			area.source_radius = 5.0;
			area.destination = position;
			area.destination_radius = 5.0;
			settings.loader_areas.Insert(area);
		}
		else if (worldType == "guard")
		{
			if (!settings.guard_posts)
				settings.guard_posts = new array<ref JobsModGuardPostJson>;

			JobsModGuardPostJson post = new JobsModGuardPostJson();
			post.id = id;
			post.name = id;
			post.zone_id = defaultZone;
			post.position = position;
			post.radius = 5.0;
			settings.guard_posts.Insert(post);
		}
		else if (worldType == "locker")
		{
			if (!settings.equipment_lockers)
				settings.equipment_lockers = new array<ref JobsModLockerJson>;

			JobsModLockerJson locker = new JobsModLockerJson();
			locker.id = id;
			locker.name = id;
			locker.position = position;
			locker.rotation = 0.0;
			locker.class_name = "Wardrobe";
			settings.equipment_lockers.Insert(locker);
		}
		else
		{
			Answer(sender, "Неизвестный тип мира: " + worldType);
			return;
		}

		BackupFile(SETTINGS_FILE, "settings.json");
		if (!JobsModJsonFileIO.SaveSettings(SETTINGS_FILE, settings, error))
		{
			Answer(sender, "Не удалось создать запись мира: " + error);
			return;
		}

		array<string> fields = WorldToFields(settings, worldType, id);
		if (fields)
			SendForm(sender, "world", worldType + "/" + id, fields);

		HotApplySaved(sender, "settings.json → " + worldType + "/" + id);
	}

	void ApplyConfig(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server)
			return;

		if (!HasPermission(sender, PERMISSION_APPLY))
			return;

		if (!JobsModServerRuntime.ReloadConfig())
		{
			Answer(sender, "JobsMod отклонил конфигурацию. Действует прежняя; подробности в RPT.");
			return;
		}

		JobsLog.Info("SERVER/ADMIN: конфигурация применена по запросу '" + sender.GetName() + "'.");
		Answer(sender, "Конфигурация применена. NPC, мусор, шкафчики и зоны пересозданы.");
	}
}
