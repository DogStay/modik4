// JobsModAdminPlugin.c
//
// Serves the JobsMod config to VPP Admin Tools: list the files, read one, save
// one, apply the lot.
//
// It holds no config knowledge of its own and parses nothing. Validation lives
// in JobsModConfig and has to stay there — it is what decides whether a config
// is usable, and a second opinion in the admin path would be a second set of
// rules to keep in step. This plugin moves text and then asks the runtime to
// reload; if the text was bad, the reload refuses it and says so, exactly as it
// would for a file edited over FTP.
//
// Every entry point is permission-gated and path-checked. An admin RPC that
// takes a file name is an arbitrary file write on the host if it is not, and
// "the caller is an admin" is not enough — an admin should not be able to
// overwrite serverDZ.cfg by typing a path with ".." in it either.

class JobsModAdminPlugin extends PluginBase
{
	// The one folder this plugin will touch. Everything else is refused no
	// matter who asks.
	protected static const string ROOT = "$profile:JobsMod";

	protected static const string PERMISSION = "MenuJobsModAdmin";

	void JobsModAdminPlugin()
	{
		GetRPCManager().AddRPC("RPC_JobsModAdmin", "ListFiles", this, SingeplayerExecutionType.Server);
		GetRPCManager().AddRPC("RPC_JobsModAdmin", "ReadFile", this, SingeplayerExecutionType.Server);
		GetRPCManager().AddRPC("RPC_JobsModAdmin", "SaveFile", this, SingeplayerExecutionType.Server);
		GetRPCManager().AddRPC("RPC_JobsModAdmin", "ApplyConfig", this, SingeplayerExecutionType.Server);
	}

	// =====================================================================
	// Path safety
	// =====================================================================
	// Accepts only the shapes the config actually has: settings.json,
	// Jobs/<name>.json, NPC/<name>.json. Anything with a drive letter, a
	// backslash, a leading slash or a ".." in it is refused outright rather
	// than normalised — normalising is where these checks usually go wrong.
	protected bool IsAllowed(string relative)
	{
		if (relative == "")
			return false;

		if (relative.IndexOf("..") != -1)
			return false;

		if (relative.IndexOf("\\") != -1 || relative.IndexOf(":") != -1)
			return false;

		if (relative.IndexOf("/") == 0)
			return false;

		if (relative == "settings.json")
			return true;

		if (relative.IndexOf("Jobs/") == 0 || relative.IndexOf("NPC/") == 0)
			return relative.IndexOf(".json") == relative.Length() - 5;

		return false;
	}

	protected string FullPath(string relative)
	{
		return ROOT + "/" + relative;
	}

	// =====================================================================
	// RPCs
	// =====================================================================
	void ListFiles(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target)
	{
		if (type != CallType.Server || !GetPermissionManager().VerifyPermission(sender.GetPlainId(), PERMISSION))
			return;

		array<string> found = new array<string>();
		found.Insert("settings.json");

		CollectFolder("Jobs", found);
		CollectFolder("NPC", found);

		GetRPCManager().SendRPC("RPC_JobsModAdminClient", "OnFileList", new Param1<ref array<string>>(found), true, sender);
	}

	protected void CollectFolder(string folder, out array<string> found)
	{
		string pattern = ROOT + "/" + folder + "/*.json";
		string name;
		FileAttr attr;

		FindFileHandle handle = FindFile(pattern, name, attr, FindFileFlags.ALL);
		if (!handle)
			return;

		while (name != "")
		{
			found.Insert(folder + "/" + name);

			if (!FindNextFile(handle, name, attr))
				break;
		}

		CloseFindFile(handle);
	}

	void ReadFile(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target)
	{
		if (type != CallType.Server || !GetPermissionManager().VerifyPermission(sender.GetPlainId(), PERMISSION))
			return;

		Param1<string> request;
		if (!ctx.Read(request) || !IsAllowed(request.param1))
			return;

		string text = "";
		string line;

		FileHandle file = OpenFile(FullPath(request.param1), FileMode.READ);
		if (!file)
			return;

		while (FGets(file, line) >= 0)
			text = text + line + "\n";

		CloseFile(file);

		GetRPCManager().SendRPC("RPC_JobsModAdminClient", "OnFileRead", new Param2<string, string>(request.param1, text), true, sender);
	}

	void SaveFile(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target)
	{
		if (type != CallType.Server || !GetPermissionManager().VerifyPermission(sender.GetPlainId(), PERMISSION))
			return;

		Param2<string, string> request;
		if (!ctx.Read(request) || !IsAllowed(request.param1))
			return;

		FileHandle file = OpenFile(FullPath(request.param1), FileMode.WRITE);
		if (!file)
		{
			Answer(sender, "Не удалось открыть файл на запись: " + request.param1);
			return;
		}

		FPrint(file, request.param2);
		CloseFile(file);

		JobsLog.Info("SERVER/ADMIN: '" + sender.GetName() + "' сохранил " + request.param1 + ".");
		Answer(sender, "Сохранено: " + request.param1 + ". Нажмите «Применить», чтобы конфигурация вступила в силу.");
	}

	// Saving and applying are separate on purpose. An admin editing three files
	// would otherwise reload the world after each one, closing every player's
	// job three times over.
	void ApplyConfig(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target)
	{
		if (type != CallType.Server || !GetPermissionManager().VerifyPermission(sender.GetPlainId(), PERMISSION))
			return;

		if (JobsModServerRuntime.ReloadConfig())
		{
			JobsLog.Info("SERVER/ADMIN: конфигурация применена по запросу '" + sender.GetName() + "'.");
			Answer(sender, "Конфигурация применена. NPC, точки и шкафчики пересозданы.");
			return;
		}

		Answer(sender, "Конфигурация отклонена — в ней не осталось работ или NPC. Действует прежняя. Подробности в RPT по строкам [JobsMod].");
	}

	protected void Answer(PlayerIdentity sender, string text)
	{
		GetRPCManager().SendRPC("RPC_JobsModAdminClient", "OnAdminMessage", new Param1<string>(text), true, sender);
	}
}
