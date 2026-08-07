// JobsModConfig.c
//
// Reads the server config from the profile folder, writing a working default
// the first time so an admin has a real file to edit rather than a blank page.
//
// An existing file is never overwritten — an admin's edits outrank our
// defaults, even when a value looks wrong. Values that cannot work are clamped
// at load and the correction is logged, so the mod keeps running instead of
// refusing to start over a typo.

class JobsModConfig
{
	static const string CONFIG_DIR = "$profile:JobsMod";
	static const string CONFIG_FILE = "$profile:JobsMod/janitor.json";

	protected static const int DEFAULT_PLAYER_COOLDOWN = 120;
	protected static const int DEFAULT_PILE_RESPAWN = 300;
	protected static const float MIN_ZONE_RADIUS = 3.0;
	protected static const int MAX_PILES_PER_ZONE = 20;

	protected int m_PlayerCooldownSeconds;
	protected int m_PileRespawnSeconds;
	protected bool m_DebugLogging;
	protected ref array<ref JobsModZoneJson> m_Zones;

	void JobsModConfig()
	{
		m_PlayerCooldownSeconds = DEFAULT_PLAYER_COOLDOWN;
		m_PileRespawnSeconds = DEFAULT_PILE_RESPAWN;
		m_DebugLogging = false;
		m_Zones = new array<ref JobsModZoneJson>();
	}

	int GetPlayerCooldownSeconds() { return m_PlayerCooldownSeconds; }
	int GetPileRespawnSeconds() { return m_PileRespawnSeconds; }
	bool IsDebugLogging() { return m_DebugLogging; }
	array<ref JobsModZoneJson> GetZones() { return m_Zones; }

	bool Load()
	{
		MakeDirectory(CONFIG_DIR);

		if (!FileExist(CONFIG_FILE))
		{
			JobsLog.Info("SERVER/CONFIG: " + CONFIG_FILE + " не найден, создаётся файл с настройками по умолчанию.");

			// Being unable to write is almost always a launch problem, not a mod
			// problem: no -profiles= parameter, or a read-only profile folder.
			// Refusing to start would leave an admin with a mod that does nothing
			// and one line explaining why, so run on in-memory defaults instead
			// and say plainly that edits will not persist.
			if (!WriteDefaults())
			{
				JobsLog.Error("SERVER/CONFIG: записать " + CONFIG_FILE + " не удалось.");
				JobsLog.Error("SERVER/CONFIG: проверьте параметр запуска -profiles= и права на запись в эту папку.");
				JobsLog.Warning("SERVER/CONFIG: мод работает на встроенных настройках; правки конфига сохраняться не будут.");
				ApplyBuiltInDefaults();
				return true;
			}
		}

		JobsModConfigJson data;
		string error;

		// A file that exists but does not parse is an admin's edit gone wrong.
		// Silently replacing it with defaults would move their zones without
		// telling them, so this is the one case that does stop the mod.
		if (!JobsModJsonFileIO.LoadConfig(CONFIG_FILE, data, error) || !data)
		{
			JobsLog.Error("SERVER/CONFIG: не удалось разобрать " + CONFIG_FILE + " (" + error + ").");
			JobsLog.Error("SERVER/CONFIG: исправьте синтаксис JSON или удалите файл, чтобы он создался заново.");
			return false;
		}

		Adopt(data);
		return true;
	}

	// The same values WriteDefaults would have saved, applied straight to the
	// live config so a non-writable profile folder still gives a playable mod.
	protected void ApplyBuiltInDefaults()
	{
		Adopt(BuildDefaults());
	}

	// Copies the parsed file into the live config, correcting anything that
	// cannot work. Each correction is logged with the value that caused it.
	protected void Adopt(JobsModConfigJson data)
	{
		m_PlayerCooldownSeconds = data.player_cooldown_seconds;
		if (m_PlayerCooldownSeconds < 0)
		{
			JobsLog.Warning("SERVER/CONFIG: player_cooldown_seconds отрицательный, принят 0.");
			m_PlayerCooldownSeconds = 0;
		}

		m_PileRespawnSeconds = data.pile_respawn_seconds;
		if (m_PileRespawnSeconds < 1)
		{
			JobsLog.Warning("SERVER/CONFIG: pile_respawn_seconds меньше 1, принято " + DEFAULT_PILE_RESPAWN.ToString() + ".");
			m_PileRespawnSeconds = DEFAULT_PILE_RESPAWN;
		}

		m_DebugLogging = data.debug_logging;

		m_Zones.Clear();

		if (!data.zones || data.zones.Count() == 0)
		{
			JobsLog.Warning("SERVER/CONFIG: в конфиге нет ни одной зоны — кучи мусора не появятся.");
			return;
		}

		for (int i = 0; i < data.zones.Count(); i++)
		{
			JobsModZoneJson zone = data.zones.Get(i);
			if (!zone)
				continue;

			if (zone.name == "")
			{
				JobsLog.Warning("SERVER/CONFIG: зона #" + i.ToString() + " без имени пропущена.");
				continue;
			}

			if (zone.piles < 1)
			{
				JobsLog.Warning("SERVER/CONFIG: зона '" + zone.name + "' с piles<1 пропущена.");
				continue;
			}

			if (zone.piles > MAX_PILES_PER_ZONE)
			{
				JobsLog.Warning("SERVER/CONFIG: зона '" + zone.name + "' запрашивает " + zone.piles.ToString()
					+ " куч, ограничено до " + MAX_PILES_PER_ZONE.ToString() + ".");
				zone.piles = MAX_PILES_PER_ZONE;
			}

			// A radius under a few metres would stack every pile on one spot.
			if (zone.radius < MIN_ZONE_RADIUS)
			{
				JobsLog.Warning("SERVER/CONFIG: радиус зоны '" + zone.name + "' слишком мал, принят "
					+ MIN_ZONE_RADIUS.ToString() + ".");
				zone.radius = MIN_ZONE_RADIUS;
			}

			m_Zones.Insert(zone);
		}

		JobsLog.Info("SERVER/CONFIG: принято зон: " + m_Zones.Count().ToString()
			+ "; кулдаун игрока " + m_PlayerCooldownSeconds.ToString()
			+ " с; респавн кучи " + m_PileRespawnSeconds.ToString() + " с.");

		// Printing the coordinates back is the fastest way to spot a config
		// meant for another map: the numbers either match where you stand or
		// they do not.
		for (int z = 0; z < m_Zones.Count(); z++)
		{
			JobsModZoneJson accepted = m_Zones.Get(z);
			JobsLog.Info("SERVER/CONFIG:   зона '" + accepted.name + "' X " + accepted.x.ToString()
				+ " / Z " + accepted.z.ToString() + ", радиус " + accepted.radius.ToString()
				+ ", куч " + accepted.piles.ToString() + ".");
		}
	}

	// The shipped defaults point at Chernarus towns. On any other map they will
	// be in the wrong place, which is why the startup log always reports where
	// piles were actually spawned.
	//
	// Built in one place so the file written on first run and the fallback used
	// when writing fails can never describe different worlds.
	protected JobsModConfigJson BuildDefaults()
	{
		JobsModConfigJson data = new JobsModConfigJson();
		data.player_cooldown_seconds = DEFAULT_PLAYER_COOLDOWN;
		data.pile_respawn_seconds = DEFAULT_PILE_RESPAWN;
		// Debug is on out of the box: the first thing anyone does with a fresh
		// install is find out whether it works at all.
		data.debug_logging = true;
		data.zones = new array<ref JobsModZoneJson>();

		data.zones.Insert(MakeZone("Черногорск, площадь", 6600.0, 2500.0, 30.0, 2));
		data.zones.Insert(MakeZone("Электрозаводск, набережная", 10400.0, 2200.0, 30.0, 2));
		data.zones.Insert(MakeZone("Березино, порт", 12000.0, 9000.0, 30.0, 2));

		return data;
	}

	protected bool WriteDefaults()
	{
		JobsModConfigJson data = BuildDefaults();

		string error;
		if (!JobsModJsonFileIO.SaveConfig(CONFIG_FILE, data, error))
		{
			JobsLog.Error("SERVER/CONFIG: не удалось записать " + CONFIG_FILE + " (" + error + ").");
			return false;
		}

		JobsLog.Info("SERVER/CONFIG: создан " + CONFIG_FILE
			+ ". Координаты рассчитаны на Chernarus — на другой карте отредактируйте зоны.");
		return true;
	}

	protected JobsModZoneJson MakeZone(string name, float x, float z, float radius, int piles)
	{
		JobsModZoneJson zone = new JobsModZoneJson();
		zone.name = name;
		zone.x = x;
		zone.z = z;
		zone.radius = radius;
		zone.piles = piles;
		return zone;
	}
}
