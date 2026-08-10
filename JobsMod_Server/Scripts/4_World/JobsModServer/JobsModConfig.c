// JobsModConfig.c
//
// Reads the server config out of $profile:JobsMod and hands the rest of the mod
// a checked, cross-referenced picture of it.
//
// The layout is three things:
//
//   settings.json    the world — switches, zones, trash points, freight routes
//   Jobs/<id>.json   one file per job
//   NPC/<id>.json    one file per employer
//
// Jobs and NPCs are one file each because adding or removing one is a decision
// on its own: dropping a file in adds it, deleting the file removes it, and the
// file name is the id everything else refers to. Everything those two merely
// point at lives in settings.json, because a zone or a trash point is a line,
// and a folder of one-line files is more to manage than it is worth.
//
// Rules this file follows:
//
//   * An existing file is never overwritten. An admin's edits outrank our
//     defaults even when a value looks wrong.
//   * A single broken entry costs that entry, not the server. A pile pointing at
//     a zone that does not exist is dropped with a line in the log; the other
//     piles still spawn.
//   * Everything that is dropped or corrected says so in the log, with the id
//     that caused it. Silence would leave an admin staring at a world where
//     half their config did not happen.
//
// First run is decided by settings.json alone. If it is missing, the whole
// default tree is written; if it is there, whatever else exists is what gets
// loaded — so deleting an NPC file really removes that NPC instead of having it
// reappear on the next restart.

class JobsModConfig
{
	static const string ROOT_DIR = "$profile:JobsMod";
	static const string DIR_NPC = ROOT_DIR + "/NPC";
	static const string DIR_JOBS = ROOT_DIR + "/Jobs";

	static const string FILE_SETTINGS = ROOT_DIR + "/settings.json";

	protected static const int MIN_PILE_RESPAWN = 10;
	protected static const int MIN_ASSIGNMENT_TIMEOUT = 60;
	protected static const float MIN_AREA_RADIUS = 3.0;
	// Below this a guard shift is over before the player has walked to the post.
	protected static const int MIN_GUARD_SECONDS = 30;
	// A contract nobody could carry is a contract nobody can finish.
	protected static const int MAX_COLLECT_REQUIRED = 200;
	protected static const int MAX_CARGOS_PER_JOB = 60;
	protected static const int MAX_PILES_PER_JOB = 30;

	// The only parcel class that refuses to be dropped. A job that names
	// nothing gets this one rather than a droppable substitute.
	protected static const string DEFAULT_PACKAGE_CLASS = "JobsMod_Parcel";

	protected ref JobsModSettingsJson m_Settings;

	// Built from m_Settings at load. The file is a list because that is what is
	// pleasant to edit; these are the shapes the rest of the mod actually asks
	// questions of, and only entries that passed their checks are in them.
	protected ref map<string, ref JobsModZoneJson> m_Zones;
	protected ref map<string, ref JobsModLoaderAreaJson> m_LoaderAreas;
	protected ref map<string, ref JobsModGuardPostJson> m_GuardPosts;
	protected ref array<ref JobsModPilePointJson> m_PilePoints;

	protected ref map<string, ref JobsModJobJson> m_Jobs;
	protected ref map<string, ref JobsModNpcJson> m_Npcs;

	// False once a write has failed: the mod keeps running on what it has, and
	// says once that nothing an admin edits will survive a restart.
	protected bool m_ProfileWritable;

	void JobsModConfig()
	{
		m_Settings = JobsModConfigDefaults.BuildSettings();
		m_Zones = new map<string, ref JobsModZoneJson>();
		m_LoaderAreas = new map<string, ref JobsModLoaderAreaJson>();
		m_GuardPosts = new map<string, ref JobsModGuardPostJson>();
		m_PilePoints = new array<ref JobsModPilePointJson>();
		m_Jobs = new map<string, ref JobsModJobJson>();
		m_Npcs = new map<string, ref JobsModNpcJson>();
		m_ProfileWritable = true;
	}

	// =====================================================================
	// Accessors
	// =====================================================================
	int GetPileRespawnSeconds() { return m_Settings.pile_respawn_seconds; }
	int GetAssignmentTimeoutSeconds() { return m_Settings.assignment_timeout_seconds; }
	bool IsDebugLogging() { return m_Settings.debug_logging; }

	array<ref JobsModPilePointJson> GetPilePoints() { return m_PilePoints; }
	map<string, ref JobsModNpcJson> GetNpcs() { return m_Npcs; }
	map<string, ref JobsModJobJson> GetJobs() { return m_Jobs; }

	string GetZoneName(string zoneId)
	{
		JobsModZoneJson zone;
		if (m_Zones.Find(zoneId, zone) && zone)
			return zone.name;

		return zoneId;
	}

	JobsModJobJson GetJob(string jobId)
	{
		JobsModJobJson job;
		if (m_Jobs.Find(jobId, job))
			return job;

		return null;
	}

	JobsModNpcJson GetNpc(string npcId)
	{
		JobsModNpcJson npc;
		if (m_Npcs.Find(npcId, npc))
			return npc;

		return null;
	}

	JobsModGuardPostJson GetGuardPost(string postId)
	{
		JobsModGuardPostJson post;
		if (m_GuardPosts.Find(postId, post))
			return post;

		return null;
	}

	JobsModLoaderAreaJson GetLoaderArea(string areaId)
	{
		JobsModLoaderAreaJson area;
		if (m_LoaderAreas.Find(areaId, area))
			return area;

		return null;
	}

	// =====================================================================
	// Loading
	// =====================================================================
	// Returns false only when the config is unusable as a whole. Individual bad
	// entries are dropped instead, so one typo cannot take the server down.
	bool Load()
	{
		EnsureDirectories();

		if (!FileExist(FILE_SETTINGS))
		{
			JobsLog.Info("SERVER/CONFIG: " + ROOT_DIR + " пуст, создаётся конфигурация по умолчанию.");
			WriteDefaults();
		}

		LoadSettings();
		LoadJobs();
		LoadNpcs();
		ResolveCrossReferences();

		// A profile folder we could not write to leaves the entity folders empty
		// no matter what we intended to put there. Running on the built-in tree
		// gives a playable server; the write failure was already reported with
		// the -profiles= hint that explains it.
		if (!m_ProfileWritable && (m_Jobs.Count() == 0 || m_Npcs.Count() == 0))
			ApplyBuiltInDefaults();

		Report();

		if (m_Jobs.Count() == 0)
		{
			JobsLog.Error("SERVER/CONFIG: не осталось ни одной корректной работы — выдавать нечего.");
			return false;
		}

		if (m_Npcs.Count() == 0)
		{
			JobsLog.Error("SERVER/CONFIG: не осталось ни одного корректного NPC — работу выдавать некому.");
			return false;
		}

		return true;
	}

	protected void EnsureDirectories()
	{
		// The root has to exist before the engine will create anything under it,
		// so the order here is not cosmetic.
		MakeDirectory(ROOT_DIR);
		MakeDirectory(DIR_NPC);
		MakeDirectory(DIR_JOBS);
	}

	// The same tree WriteDefaults would have saved, applied straight to the live
	// config. Every entry still goes through the ordinary accept checks, so the
	// built-in path cannot accidentally allow something the file path rejects.
	protected void ApplyBuiltInDefaults()
	{
		JobsLog.Warning("SERVER/CONFIG: конфигурация с диска пуста — применяются встроенные настройки.");

		m_Jobs.Clear();
		m_Npcs.Clear();

		AcceptSettings(JobsModConfigDefaults.BuildSettings());

		array<ref JobsModJobJson> jobs = JobsModConfigDefaults.BuildJobs();
		int i;
		for (i = 0; i < jobs.Count(); i++)
		{
			if (AcceptJob(jobs.Get(i)))
				m_Jobs.Set(jobs.Get(i).id, jobs.Get(i));
		}

		array<ref JobsModNpcJson> npcs = JobsModConfigDefaults.BuildNpcs();
		for (i = 0; i < npcs.Count(); i++)
		{
			if (AcceptNpc(npcs.Get(i)))
				m_Npcs.Set(npcs.Get(i).id, npcs.Get(i));
		}

		ResolveCrossReferences();
	}

	protected void WriteDefaults()
	{
		if (!m_ProfileWritable)
			return;

		WriteSettings(JobsModConfigDefaults.BuildSettings());

		array<ref JobsModJobJson> jobs = JobsModConfigDefaults.BuildJobs();
		int i;
		for (i = 0; i < jobs.Count(); i++)
			WriteJob(jobs.Get(i));

		array<ref JobsModNpcJson> npcs = JobsModConfigDefaults.BuildNpcs();
		for (i = 0; i < npcs.Count(); i++)
			WriteNpc(npcs.Get(i));

		if (m_ProfileWritable)
		{
			JobsLog.Info("SERVER/CONFIG: конфигурация по умолчанию создана в " + ROOT_DIR + ".");
			JobsLog.Info("SERVER/CONFIG: координаты рассчитаны на Chernarus — на другой карте отредактируйте их.");
		}
	}

	// ---------------------------------------------------------------------
	// settings.json
	// ---------------------------------------------------------------------
	protected void LoadSettings()
	{
		JobsModSettingsJson loaded;
		string error;

		if (!FileExist(FILE_SETTINGS) || !JobsModJsonFileIO.LoadSettings(FILE_SETTINGS, loaded, error) || !loaded)
		{
			JobsLog.Warning("SERVER/CONFIG: settings.json не прочитан (" + error + "), приняты встроенные значения.");
			loaded = JobsModConfigDefaults.BuildSettings();
		}

		AcceptSettings(loaded);
	}

	// Takes a freshly read settings file apart: the switches are clamped, and
	// each list is checked entry by entry into the map the rest of the mod uses.
	protected void AcceptSettings(JobsModSettingsJson settings)
	{
		m_Settings = settings;

		m_Zones.Clear();
		m_PilePoints.Clear();
		m_LoaderAreas.Clear();
		m_GuardPosts.Clear();

		if (m_Settings.pile_respawn_seconds < MIN_PILE_RESPAWN)
		{
			JobsLog.Warning("SERVER/CONFIG: pile_respawn_seconds слишком мал, принято " + MIN_PILE_RESPAWN.ToString() + ".");
			m_Settings.pile_respawn_seconds = MIN_PILE_RESPAWN;
		}

		if (m_Settings.assignment_timeout_seconds < MIN_ASSIGNMENT_TIMEOUT)
		{
			JobsLog.Warning("SERVER/CONFIG: assignment_timeout_seconds слишком мал, принято " + MIN_ASSIGNMENT_TIMEOUT.ToString() + ".");
			m_Settings.assignment_timeout_seconds = MIN_ASSIGNMENT_TIMEOUT;
		}

		// A missing list is an empty one, not a broken file. An admin who has no
		// freight routes simply deletes the section.
		if (!m_Settings.zones)
			m_Settings.zones = new array<ref JobsModZoneJson>();

		if (!m_Settings.pile_points)
			m_Settings.pile_points = new array<ref JobsModPilePointJson>();

		if (!m_Settings.loader_areas)
			m_Settings.loader_areas = new array<ref JobsModLoaderAreaJson>();

		if (!m_Settings.guard_posts)
			m_Settings.guard_posts = new array<ref JobsModGuardPostJson>();

		// Zones first: the other two are checked against them.
		AcceptZones();
		AcceptPilePoints();
		AcceptLoaderAreas();
		AcceptGuardPosts();

		WarnAboutOldLayout();
	}

	// Before 2.3 the zones, trash points and freight routes lived in folders of
	// their own, and settings.json held only the three switches. Such a file
	// still parses — the new lists simply come back empty — and the result is
	// every job being dropped for naming a zone that does not exist, which is a
	// pile of warnings that never says what actually happened.
	//
	// A settings file with no zones at all is that case in practice: a config
	// worth running has at least one. Saying so once, plainly, is worth more
	// than the twenty lines it saves.
	protected void WarnAboutOldLayout()
	{
		if (m_Zones.Count() > 0)
			return;

		JobsLog.Error("SERVER/CONFIG: в settings.json нет ни одной зоны.");
		JobsLog.Error("SERVER/CONFIG: с версии 2.3 зоны, точки мусора и маршруты лежат в settings.json, а не в папках Zones/ PilePoints/ LoaderAreas/, и координаты пишутся строкой «X Y Z».");
		JobsLog.Error("SERVER/CONFIG: перенесите их в settings.json — или удалите его, и мод создаст файл нового формата заново.");
	}

	protected void AcceptZones()
	{
		for (int i = 0; i < m_Settings.zones.Count(); i++)
		{
			JobsModZoneJson zone = m_Settings.zones.Get(i);

			if (!zone || zone.id == "")
			{
				JobsLog.Warning("SERVER/CONFIG: зона #" + i.ToString() + " без id пропущена.");
				continue;
			}

			if (zone.name == "")
				zone.name = zone.id;

			if (m_Zones.Contains(zone.id))
			{
				JobsLog.Warning("SERVER/CONFIG: зона '" + zone.id + "' объявлена дважды, вторая пропущена.");
				continue;
			}

			m_Zones.Set(zone.id, zone);
		}
	}

	protected void AcceptPilePoints()
	{
		for (int i = 0; i < m_Settings.pile_points.Count(); i++)
		{
			JobsModPilePointJson point = m_Settings.pile_points.Get(i);

			if (!point || point.id == "")
			{
				JobsLog.Warning("SERVER/CONFIG: точка мусора #" + i.ToString() + " без id пропущена.");
				continue;
			}

			if (!m_Zones.Contains(point.zone_id))
			{
				JobsLog.Warning("SERVER/CONFIG: точка '" + point.id + "' ссылается на неизвестную зону '" + point.zone_id + "', пропущена.");
				continue;
			}

			vector position;
			if (!JobsModCoords.Parse(point.position, position))
			{
				JobsLog.Warning("SERVER/CONFIG: у точки '" + point.id + "' не разобрана position='" + point.position + "', пропущена. Ожидается «X Y Z», например «6600 0 2500».");
				continue;
			}

			if (position[0] == 0 && position[2] == 0)
			{
				JobsLog.Warning("SERVER/CONFIG: точка '" + point.id + "' стоит в начале координат, пропущена.");
				continue;
			}

			m_PilePoints.Insert(point);
		}
	}

	protected void AcceptLoaderAreas()
	{
		for (int i = 0; i < m_Settings.loader_areas.Count(); i++)
		{
			JobsModLoaderAreaJson area = m_Settings.loader_areas.Get(i);

			if (!area || area.id == "")
			{
				JobsLog.Warning("SERVER/CONFIG: маршрут #" + i.ToString() + " без id пропущен.");
				continue;
			}

			if (m_LoaderAreas.Contains(area.id))
			{
				JobsLog.Warning("SERVER/CONFIG: маршрут '" + area.id + "' объявлен дважды, второй пропущен.");
				continue;
			}

			vector source;
			vector destination;

			if (!JobsModCoords.Parse(area.source, source) || !JobsModCoords.Parse(area.destination, destination))
			{
				JobsLog.Warning("SERVER/CONFIG: у маршрута '" + area.id + "' не разобраны source/destination, пропущен. Ожидается «X Y Z» в каждом.");
				continue;
			}

			ClampRadius(area, true);
			ClampRadius(area, false);

			// Overlapping circles would let a box count as delivered where it
			// was picked up, and the job would finish itself.
			if (Distance2D(source, destination) < area.source_radius + area.destination_radius)
			{
				JobsLog.Warning("SERVER/CONFIG: у маршрута '" + area.id + "' зоны погрузки и разгрузки пересекаются — груз будет засчитываться сразу.");
			}

			m_LoaderAreas.Set(area.id, area);
		}
	}

	protected void AcceptGuardPosts()
	{
		for (int i = 0; i < m_Settings.guard_posts.Count(); i++)
		{
			JobsModGuardPostJson post = m_Settings.guard_posts.Get(i);

			if (!post || post.id == "")
			{
				JobsLog.Warning("SERVER/CONFIG: пост охраны #" + i.ToString() + " без id пропущен.");
				continue;
			}

			if (m_GuardPosts.Contains(post.id))
			{
				JobsLog.Warning("SERVER/CONFIG: пост охраны '" + post.id + "' объявлен дважды, второй пропущен.");
				continue;
			}

			vector centre;
			if (!JobsModCoords.Parse(post.position, centre))
			{
				JobsLog.Warning("SERVER/CONFIG: у поста '" + post.id + "' не разобрана position='" + post.position + "', пропущен. Ожидается «X Y Z».");
				continue;
			}

			// A post the player cannot stand still inside would stop the clock
			// every time they shifted their feet.
			if (post.radius < MIN_AREA_RADIUS)
			{
				JobsLog.Warning("SERVER/CONFIG: радиус поста '" + post.id + "' слишком мал, принят " + MIN_AREA_RADIUS.ToString() + ".");
				post.radius = MIN_AREA_RADIUS;
			}

			if (post.zone_id != "" && !m_Zones.Contains(post.zone_id))
				JobsLog.Warning("SERVER/CONFIG: пост '" + post.id + "' ссылается на неизвестную зону '" + post.zone_id + "'.");

			m_GuardPosts.Set(post.id, post);
		}
	}

	protected void ClampRadius(JobsModLoaderAreaJson area, bool isSource)
	{
		if (isSource)
		{
			if (area.source_radius >= MIN_AREA_RADIUS)
				return;

			JobsLog.Warning("SERVER/CONFIG: source_radius маршрута '" + area.id + "' слишком мал, принят " + MIN_AREA_RADIUS.ToString() + ".");
			area.source_radius = MIN_AREA_RADIUS;
			return;
		}

		if (area.destination_radius >= MIN_AREA_RADIUS)
			return;

		JobsLog.Warning("SERVER/CONFIG: destination_radius маршрута '" + area.id + "' слишком мал, принят " + MIN_AREA_RADIUS.ToString() + ".");
		area.destination_radius = MIN_AREA_RADIUS;
	}

	protected void WriteSettings(JobsModSettingsJson settings)
	{
		string error;
		if (JobsModJsonFileIO.SaveSettings(FILE_SETTINGS, settings, error))
			return;

		ReportWriteFailure(FILE_SETTINGS, error);
	}

	// ---------------------------------------------------------------------
	// Jobs/*.json
	// ---------------------------------------------------------------------
	protected void LoadJobs()
	{
		array<string> files;
		JobsModJsonFileIO.ListJsonFiles(DIR_JOBS, files);

		for (int i = 0; i < files.Count(); i++)
		{
			string fileName = files.Get(i);
			string path = DIR_JOBS + "/" + fileName;

			JobsModJobJson job;
			string error;

			if (!JobsModJsonFileIO.LoadJob(path, job, error) || !job)
			{
				JobsLog.Error("SERVER/CONFIG: " + path + " не разобран (" + error + "), файл пропущен.");
				continue;
			}

			if (!CheckId(job.id, fileName, path))
				continue;

			if (m_Jobs.Contains(job.id))
			{
				JobsLog.Warning("SERVER/CONFIG: работа '" + job.id + "' объявлена дважды, вторая пропущена.");
				continue;
			}

			if (!AcceptJob(job))
				continue;

			m_Jobs.Set(job.id, job);
		}
	}

	// Everything a job needs to be runnable is checked here rather than at the
	// moment a player takes it: a broken job that only fails in front of a
	// player is a bug report, a broken job that fails at startup is a log line.
	protected bool AcceptJob(JobsModJobJson job)
	{
		if (job.name == "")
			job.name = job.id;

		int type = JobsModJobType.FromText(job.type);
		if (type == JobsModJobType.UNKNOWN)
		{
			JobsLog.Warning("SERVER/CONFIG: у работы '" + job.id + "' неизвестный type='" + job.type + "'; допустимо '" + JobsModJobType.TEXT_SORTING + "', '" + JobsModJobType.TEXT_LOADING + "', '" + JobsModJobType.TEXT_MESSENGER + "' или '" + JobsModJobType.TEXT_GUARD + "' или '" + JobsModJobType.TEXT_COLLECT + "'.");
			return false;
		}

		if (!m_Zones.Contains(job.zone_id))
		{
			JobsLog.Warning("SERVER/CONFIG: работа '" + job.id + "' ссылается на неизвестную зону '" + job.zone_id + "', пропущена.");
			return false;
		}

		if (job.reward < 0)
		{
			JobsLog.Warning("SERVER/CONFIG: у работы '" + job.id + "' отрицательная награда, принят 0.");
			job.reward = 0;
		}

		if (job.cooldown_seconds < 0)
			job.cooldown_seconds = 0;

		if (type == JobsModJobType.SORTING)
			return AcceptSortingJob(job);

		if (type == JobsModJobType.MESSENGER)
			return AcceptMessengerJob(job);

		if (type == JobsModJobType.GUARD)
			return AcceptGuardJob(job);

		if (type == JobsModJobType.COLLECT)
			return AcceptCollectJob(job);

		return AcceptLoadingJob(job);
	}

	// Whether the recipient actually exists cannot be answered here: NPCs are
	// loaded after jobs, because an NPC's offer list is checked against the
	// jobs. The reference the other way round is settled in
	// ResolveCrossReferences once both halves are in.
	protected bool AcceptMessengerJob(JobsModJobJson job)
	{
		if (job.target_npc_id == "")
		{
			JobsLog.Warning("SERVER/CONFIG: у работы '" + job.id + "' не задан target_npc_id — некому вручить пакет, пропущена.");
			return false;
		}

		if (job.package_class == "")
		{
			job.package_class = DEFAULT_PACKAGE_CLASS;
			JobsLog.Warning("SERVER/CONFIG: у работы '" + job.id + "' не задан package_class, принят " + DEFAULT_PACKAGE_CLASS + ".");
		}

		return true;
	}

	protected bool AcceptSortingJob(JobsModJobJson job)
	{
		// Normalize old jobs.json files automatically so an existing value of 3
		// cannot leave the player waiting for the single object to respawn.
		if (job.piles_required != 1)
		{
			JobsLog.Warning("SERVER/CONFIG: у работы '" + job.id + "' piles_required=" + job.piles_required.ToString() + "; для режима одного предмета принято 1.");
			job.piles_required = 1;
		}

		return true;
	}

	protected bool AcceptCollectJob(JobsModJobJson job)
	{
		if (!job.collect_classes || job.collect_classes.Count() == 0)
		{
			JobsLog.Warning("SERVER/CONFIG: у работы '" + job.id + "' пустой collect_classes — непонятно, что собирать, пропущена.");
			return false;
		}

		if (job.collect_required < 1)
		{
			JobsLog.Warning("SERVER/CONFIG: у работы '" + job.id + "' collect_required < 1, пропущена.");
			return false;
		}

		if (job.collect_required > MAX_COLLECT_REQUIRED)
		{
			JobsLog.Warning("SERVER/CONFIG: у работы '" + job.id + "' collect_required больше " + MAX_COLLECT_REQUIRED.ToString() + ", принято " + MAX_COLLECT_REQUIRED.ToString() + ".");
			job.collect_required = MAX_COLLECT_REQUIRED;
		}

		// Without a label the HUD would print a class name at the player, so the
		// first class is a better default than nothing.
		if (job.collect_label == "")
			job.collect_label = job.collect_classes.Get(0);

		job.piles_required = job.collect_required;
		return true;
	}

	protected bool AcceptGuardJob(JobsModJobJson job)
	{
		if (!m_GuardPosts.Contains(job.guard_post_id))
		{
			JobsLog.Warning("SERVER/CONFIG: работа '" + job.id + "' ссылается на неизвестный пост '" + job.guard_post_id + "', пропущена.");
			return false;
		}

		if (job.guard_seconds < MIN_GUARD_SECONDS)
		{
			JobsLog.Warning("SERVER/CONFIG: у работы '" + job.id + "' guard_seconds меньше " + MIN_GUARD_SECONDS.ToString() + ", принято " + MIN_GUARD_SECONDS.ToString() + ".");
			job.guard_seconds = MIN_GUARD_SECONDS;
		}

		if (!job.equipment)
			job.equipment = new array<string>();

		// The shift is the required amount, counted in seconds, so the HUD and
		// every progress check work the same way they do for every other job.
		job.piles_required = job.guard_seconds;
		return true;
	}

	protected bool AcceptLoadingJob(JobsModJobJson job)
	{
		if (!m_LoaderAreas.Contains(job.loader_area_id))
		{
			JobsLog.Warning("SERVER/CONFIG: работа '" + job.id + "' ссылается на неизвестный маршрут '" + job.loader_area_id + "', пропущена.");
			return false;
		}

		if (job.cargos_required < 1)
		{
			JobsLog.Warning("SERVER/CONFIG: у работы '" + job.id + "' cargos_required < 1, пропущена.");
			return false;
		}

		if (job.cargos_required > MAX_CARGOS_PER_JOB)
		{
			JobsLog.Warning("SERVER/CONFIG: у работы '" + job.id + "' cargos_required ограничено до " + MAX_CARGOS_PER_JOB.ToString() + ".");
			job.cargos_required = MAX_CARGOS_PER_JOB;
		}

		if (job.cargo_class == "")
		{
			job.cargo_class = "JobsMod_CargoBox";
			JobsLog.Warning("SERVER/CONFIG: у работы '" + job.id + "' не задан cargo_class, принят JobsMod_CargoBox.");
		}

		return true;
	}

	protected int CountPilePointsInZone(string zoneId)
	{
		int count = 0;

		for (int i = 0; i < m_PilePoints.Count(); i++)
		{
			if (m_PilePoints.Get(i).zone_id == zoneId)
				count++;
		}

		return count;
	}

	protected void WriteJob(JobsModJobJson job)
	{
		string path = DIR_JOBS + "/" + job.id + ".json";
		string error;
		if (JobsModJsonFileIO.SaveJob(path, job, error))
			return;

		ReportWriteFailure(path, error);
	}

	// ---------------------------------------------------------------------
	// NPC/*.json
	// ---------------------------------------------------------------------
	protected void LoadNpcs()
	{
		array<string> files;
		JobsModJsonFileIO.ListJsonFiles(DIR_NPC, files);

		for (int i = 0; i < files.Count(); i++)
		{
			string fileName = files.Get(i);
			string path = DIR_NPC + "/" + fileName;

			JobsModNpcJson npc;
			string error;

			if (!JobsModJsonFileIO.LoadNpc(path, npc, error) || !npc)
			{
				JobsLog.Error("SERVER/CONFIG: " + path + " не разобран (" + error + "), файл пропущен.");
				continue;
			}

			if (!CheckId(npc.id, fileName, path))
				continue;

			if (m_Npcs.Contains(npc.id))
			{
				JobsLog.Warning("SERVER/CONFIG: NPC '" + npc.id + "' объявлен дважды, второй пропущен.");
				continue;
			}

			if (!AcceptNpc(npc))
				continue;

			m_Npcs.Set(npc.id, npc);
		}
	}

	// Only the NPC's own fields are judged here. Its offer list is left alone:
	// an NPC that hands out nothing is still a legitimate part of the config if
	// some courier job names it as the recipient, and whether that is the case
	// is not knowable until every job has been read.
	protected bool AcceptNpc(JobsModNpcJson npc)
	{
		if (npc.name == "")
			npc.name = npc.id;

		if (npc.player_class == "")
		{
			JobsLog.Warning("SERVER/CONFIG: у NPC '" + npc.id + "' не задан player_class, пропущен.");
			return false;
		}

		vector position;
		if (!JobsModCoords.Parse(npc.position, position))
		{
			JobsLog.Warning("SERVER/CONFIG: у NPC '" + npc.id + "' не разобрана position='" + npc.position + "', пропущен. Ожидается «X Y Z», например «6620 0 2520».");
			return false;
		}

		if (position[0] == 0 && position[2] == 0)
		{
			JobsLog.Warning("SERVER/CONFIG: NPC '" + npc.id + "' стоит в начале координат, пропущен.");
			return false;
		}

		if (!npc.clothing)
			npc.clothing = new array<string>();

		if (!npc.jobs)
			npc.jobs = new array<string>();

		return true;
	}

	protected void WriteNpc(JobsModNpcJson npc)
	{
		string path = DIR_NPC + "/" + npc.id + ".json";
		string error;
		if (JobsModJsonFileIO.SaveNpc(path, npc, error))
			return;

		ReportWriteFailure(path, error);
	}

	// =====================================================================
	// Cross-references
	// =====================================================================
	// Jobs point at NPCs and NPCs point at jobs, so neither can be fully judged
	// while the other is still being read. Everything that needs both sides is
	// settled here, in one pass, after both folders are in.
	//
	// The order matters and is not arbitrary: jobs are dropped first, then the
	// offer lists are cleaned against what survived, and only then is an NPC
	// judged on whether it has anything left to do. Doing it the other way
	// round would keep an employer whose only job had just been thrown out.
	protected void ResolveCrossReferences()
	{
		DropMessengerJobsWithoutRecipient();
		CleanOfferLists();
		DropIdleNpcs();
	}

	// A parcel addressed to somebody who is not in the world cannot be handed
	// over, so the job would be taken and then never finish.
	protected void DropMessengerJobsWithoutRecipient()
	{
		array<string> orphans = new array<string>();

		int i;
		for (i = 0; i < m_Jobs.Count(); i++)
		{
			JobsModJobJson job = m_Jobs.GetElement(i);
			if (JobsModJobType.FromText(job.type) != JobsModJobType.MESSENGER)
				continue;

			if (m_Npcs.Contains(job.target_npc_id))
				continue;

			JobsLog.Warning("SERVER/CONFIG: работа '" + job.id + "' адресована неизвестному NPC '" + job.target_npc_id + "', пропущена.");
			orphans.Insert(job.id);
		}

		for (i = 0; i < orphans.Count(); i++)
			m_Jobs.Remove(orphans.Get(i));
	}

	protected void CleanOfferLists()
	{
		for (int i = 0; i < m_Npcs.Count(); i++)
		{
			JobsModNpcJson npc = m_Npcs.GetElement(i);

			for (int j = npc.jobs.Count() - 1; j >= 0; j--)
			{
				string jobId = npc.jobs.Get(j);
				JobsModJobJson job = GetJob(jobId);

				// A reference to a job that never loaded, or that was dropped
				// above. Removing it is what keeps the menu from ever offering
				// something that cannot be created.
				if (!job)
				{
					JobsLog.Warning("SERVER/CONFIG: NPC '" + npc.id + "' ссылается на неизвестную работу '" + jobId + "', ссылка убрана.");
					npc.jobs.Remove(j);
					continue;
				}

				// An employer handing out a parcel addressed to himself is a
				// job that completes where it started. That is a typo in the
				// config every time, not a design.
				if (JobsModJobType.FromText(job.type) == JobsModJobType.MESSENGER && job.target_npc_id == npc.id)
				{
					JobsLog.Warning("SERVER/CONFIG: NPC '" + npc.id + "' выдаёт работу '" + jobId + "', адресованную самому себе — ссылка убрана.");
					npc.jobs.Remove(j);
				}
			}

			if (npc.jobs.Count() <= JobsModRPC.MAX_JOBS_PER_NPC)
				continue;

			JobsLog.Warning("SERVER/CONFIG: NPC '" + npc.id + "' выдаёт больше " + JobsModRPC.MAX_JOBS_PER_NPC.ToString() + " работ, лишние не поместятся в меню и убраны.");

			while (npc.jobs.Count() > JobsModRPC.MAX_JOBS_PER_NPC)
				npc.jobs.Remove(npc.jobs.Count() - 1);
		}
	}

	// An NPC earns its place by handing work out or by receiving a parcel.
	// One that does neither is a survivor standing in a field that the mod
	// would otherwise keep respawning forever.
	protected void DropIdleNpcs()
	{
		array<string> idle = new array<string>();

		int i;
		for (i = 0; i < m_Npcs.Count(); i++)
		{
			JobsModNpcJson npc = m_Npcs.GetElement(i);

			if (npc.jobs.Count() > 0 || IsMessengerRecipient(npc.id))
				continue;

			JobsLog.Warning("SERVER/CONFIG: NPC '" + npc.id + "' не выдаёт работ и не принимает посылок, пропущен.");
			idle.Insert(npc.id);
		}

		for (i = 0; i < idle.Count(); i++)
			m_Npcs.Remove(idle.Get(i));
	}

	protected bool IsMessengerRecipient(string npcId)
	{
		for (int i = 0; i < m_Jobs.Count(); i++)
		{
			JobsModJobJson job = m_Jobs.GetElement(i);

			if (JobsModJobType.FromText(job.type) == JobsModJobType.MESSENGER && job.target_npc_id == npcId)
				return true;
		}

		return false;
	}

	// =====================================================================
	// Helpers
	// =====================================================================
	// The file name is the id an admin reads in the folder listing; the id field
	// is what every reference in the config uses. When they disagree, one of the
	// two is a lie, and it is worth one line to say which file it is.
	protected bool CheckId(string id, string fileName, string path)
	{
		if (id == "")
		{
			JobsLog.Warning("SERVER/CONFIG: " + path + " без поля id, файл пропущен.");
			return false;
		}

		if (fileName != id + ".json")
		{
			JobsLog.Warning("SERVER/CONFIG: имя файла " + fileName + " не совпадает с id '" + id + "'. Загружен id из файла.");
		}

		return true;
	}

	protected void ReportWriteFailure(string path, string error)
	{
		if (m_ProfileWritable)
		{
			JobsLog.Error("SERVER/CONFIG: записать " + path + " не удалось (" + error + ").");
			JobsLog.Error("SERVER/CONFIG: проверьте параметр запуска -profiles= и права на запись в эту папку.");
			JobsLog.Warning("SERVER/CONFIG: мод работает на встроенных настройках; правки конфига сохраняться не будут.");
		}

		m_ProfileWritable = false;
	}

	// Height is ignored: routes are circles on the map, and one end being on a
	// pier does not make it further from the other.
	protected float Distance2D(vector a, vector b)
	{
		float dx = a[0] - b[0];
		float dz = a[2] - b[2];
		return Math.Sqrt(dx * dx + dz * dz);
	}

	// Printing everything back is the fastest way to spot a config meant for
	// another map or an entry that quietly failed to load.
	protected void Report()
	{
		JobsLog.Info("SERVER/CONFIG: зон " + m_Zones.Count().ToString() + ", точек мусора " + m_PilePoints.Count().ToString() + ", маршрутов " + m_LoaderAreas.Count().ToString() + ", работ " + m_Jobs.Count().ToString() + ", NPC " + m_Npcs.Count().ToString() + ".");

		JobsLog.Info("SERVER/CONFIG: респавн кучи " + m_Settings.pile_respawn_seconds.ToString() + " с; срок работы " + m_Settings.assignment_timeout_seconds.ToString() + " с.");

		int i;
		vector position;

		for (i = 0; i < m_PilePoints.Count(); i++)
		{
			JobsModPilePointJson point = m_PilePoints.Get(i);
			position = point.GetPosition();
			JobsLog.Info("SERVER/CONFIG:   куча '" + point.id + "' в зоне '" + point.zone_id + "' на " + position.ToString() + ".");
		}

		vector source;
		vector destination;

		for (i = 0; i < m_LoaderAreas.Count(); i++)
		{
			JobsModLoaderAreaJson area = m_LoaderAreas.GetElement(i);
			source = area.GetSource();
			destination = area.GetDestination();

			JobsLog.Info("SERVER/CONFIG:   маршрут '" + area.id + "': погрузка " + source.ToString() + " r" + area.source_radius.ToString() + ", разгрузка " + destination.ToString() + " r" + area.destination_radius.ToString() + ".");
		}

		for (i = 0; i < m_Jobs.Count(); i++)
		{
			JobsModJobJson job = m_Jobs.GetElement(i);

			// The recipient is the half of a courier job that is easiest to get
			// wrong and impossible to see in the world, so it is printed.
			string addressed = "";
			if (JobsModJobType.FromText(job.type) == JobsModJobType.MESSENGER)
				addressed = ", получатель '" + job.target_npc_id + "', пакет '" + job.package_class + "'";

			JobsLog.Info("SERVER/CONFIG:   работа '" + job.id + "' (" + job.type + ") в зоне '" + job.zone_id + "', награда " + job.reward.ToString() + ", кулдаун " + job.cooldown_seconds.ToString() + " с" + addressed + ".");
		}

		for (i = 0; i < m_Npcs.Count(); i++)
		{
			JobsModNpcJson npc = m_Npcs.GetElement(i);
			position = npc.GetPosition();

			string receives = "";
			if (IsMessengerRecipient(npc.id))
				receives = ", принимает посылки";

			JobsLog.Info("SERVER/CONFIG:   NPC '" + npc.id + "' (" + npc.name + ") на " + position.ToString() + ", работ " + npc.jobs.Count().ToString() + receives + ".");
		}
	}
}
