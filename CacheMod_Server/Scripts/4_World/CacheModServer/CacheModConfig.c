// CacheModConfig.c
//
// Reads $profile:CacheMod and hands the rest of the mod a checked picture of it.
//
// Rules this file follows, and the reasoning behind each:
//
//   * An existing file is never overwritten on load. An admin's edits outrank
//     our defaults even when a value looks wrong.
//   * A single broken entry costs that entry, not the server: a zone pointing
//     at a tier that does not exist is dropped with its id in the log, and the
//     other zones still work.
//   * Everything dropped or corrected says so in the log, with the id that
//     caused it. Silence would leave an admin staring at half a config.
//
// caches.json is the one file the running server writes back, because caches
// are created and emptied during play. Zones, tiers and settings are read-only
// to the game and are written only by the admin plugin.

class CacheModConfig
{
	static const string ROOT_DIR = "$profile:CacheMod";

	static const string FILE_SETTINGS = ROOT_DIR + "/settings.json";
	static const string FILE_CACHES = ROOT_DIR + "/caches.json";
	static const string FILE_ZONES = ROOT_DIR + "/loot_zones.json";
	static const string FILE_TIERS = ROOT_DIR + "/loot_tiers.json";

	protected static const int MIN_RESPAWN_SECONDS = 30;
	protected static const int MIN_CHEST_LIFETIME = 30;

	protected ref CacheModSettingsJson m_Settings;
	protected ref array<ref CacheModCacheJson> m_Caches;
	protected ref array<ref CacheModZoneJson> m_Zones;
	protected ref map<string, ref CacheModTierJson> m_Tiers;

	// False once a write has failed. The mod keeps running on what it has and
	// says once that nothing will survive a restart, rather than repeating the
	// complaint on every save.
	protected bool m_ProfileWritable;

	void CacheModConfig()
	{
		m_Settings = CacheModConfigDefaults.BuildSettings();
		m_Caches = new array<ref CacheModCacheJson>();
		m_Zones = new array<ref CacheModZoneJson>();
		m_Tiers = new map<string, ref CacheModTierJson>();
		m_ProfileWritable = true;
	}

	// =====================================================================
	// Accessors
	// =====================================================================
	float GetSearchDurationSeconds() { return m_Settings.search_duration_seconds; }
	string GetDefaultTier() { return m_Settings.default_tier; }
	bool IsToolInHandsOnly() { return m_Settings.tool_in_hands_only; }
	int GetRespawnSeconds() { return m_Settings.respawn_seconds; }
	float GetDefaultSpawnChance() { return m_Settings.default_spawn_chance; }
	bool IsDebugLogging() { return m_Settings.debug_logging; }

	int GetChestLifetimeSeconds() { return m_Settings.chest_settings.lifetime_seconds; }
	bool IsChestDespawnWhenEmpty() { return m_Settings.chest_settings.despawn_when_empty; }
	float GetChestHeightOffset() { return m_Settings.chest_settings.height_offset; }
	string GetChestClass() { return m_Settings.chest_settings.chest_class; }

	array<ref CacheModCacheJson> GetCaches() { return m_Caches; }
	array<ref CacheModZoneJson> GetZones() { return m_Zones; }

	CacheModTierJson GetTier(string tierId)
	{
		CacheModTierJson tier;
		if (m_Tiers.Find(tierId, tier))
			return tier;

		return null;
	}

	// =====================================================================
	// Loading
	// =====================================================================
	bool Load()
	{
		MakeDirectory(ROOT_DIR);

		LoadSettings();
		LoadTiers();
		LoadZones();
		LoadCaches();

		CacheLog.s_DebugEnabled = m_Settings.debug_logging;

		string summary = "Конфигурация загружена: точек " + m_Caches.Count().ToString();
		summary = summary + ", зон " + m_Zones.Count().ToString();
		summary = summary + ", тиров " + m_Tiers.Count().ToString() + ".";
		CacheLog.Info(CacheLog.ROOT, summary);

		return true;
	}

	// Re-reads everything except the caches, which the running server owns and
	// would lose their runtime state if they were rebuilt from disk mid-session.
	// Zones, tiers and settings are pure data and can change under the game
	// safely — that is what makes hot editing of loot possible at all.
	bool ReloadStaticData()
	{
		LoadSettings();
		LoadTiers();
		LoadZones();

		CacheLog.s_DebugEnabled = m_Settings.debug_logging;
		CacheLog.Info(CacheLog.ROOT, "Зоны, тиры и настройки перечитаны без рестарта.");
		return true;
	}

	protected void LoadSettings()
	{
		if (!FileExist(FILE_SETTINGS))
		{
			m_Settings = CacheModConfigDefaults.BuildSettings();
			SaveSettings();
			CacheLog.Info(CacheLog.ROOT, "settings.json создан со значениями по умолчанию.");
			return;
		}

		CacheModSettingsJson loaded;
		string error;
		if (!CacheModJsonFileIO.LoadSettings(FILE_SETTINGS, loaded, error))
		{
			CacheLog.Error(CacheLog.ROOT, "settings.json не прочитан (" + error + "). Используются значения по умолчанию.");
			m_Settings = CacheModConfigDefaults.BuildSettings();
			return;
		}

		if (!loaded)
		{
			m_Settings = CacheModConfigDefaults.BuildSettings();
			return;
		}

		m_Settings = loaded;
		ClampSettings();
	}

	// A value out of range is corrected rather than refused: a chest lifetime of
	// zero is a typo, and a typo should not take a server's caches offline.
	protected void ClampSettings()
	{
		if (!m_Settings.chest_settings)
		{
			m_Settings.chest_settings = CacheModConfigDefaults.BuildSettings().chest_settings;
			CacheLog.Warning(CacheLog.ROOT, "chest_settings отсутствует — взяты значения по умолчанию.");
		}

		if (m_Settings.search_duration_seconds < 1.0)
		{
			CacheLog.Warning(CacheLog.SEARCH, "search_duration_seconds слишком мал — поднят до 1 c.");
			m_Settings.search_duration_seconds = 1.0;
		}

		if (m_Settings.search_duration_seconds > 120.0)
		{
			CacheLog.Warning(CacheLog.SEARCH, "search_duration_seconds слишком велик — снижен до 120 c.");
			m_Settings.search_duration_seconds = 120.0;
		}

		if (m_Settings.respawn_seconds < MIN_RESPAWN_SECONDS)
		{
			CacheLog.Warning(CacheLog.RESPAWN, "respawn_seconds поднят до " + MIN_RESPAWN_SECONDS.ToString() + " c.");
			m_Settings.respawn_seconds = MIN_RESPAWN_SECONDS;
		}

		if (m_Settings.chest_settings.lifetime_seconds < MIN_CHEST_LIFETIME)
		{
			CacheLog.Warning(CacheLog.CHEST, "lifetime_seconds поднят до " + MIN_CHEST_LIFETIME.ToString() + " c.");
			m_Settings.chest_settings.lifetime_seconds = MIN_CHEST_LIFETIME;
		}

		if (m_Settings.chest_settings.chest_class == "")
		{
			CacheLog.Warning(CacheLog.CHEST, "chest_class пуст — используется CacheLootChest.");
			m_Settings.chest_settings.chest_class = "CacheLootChest";
		}

		m_Settings.default_spawn_chance = ClampChance(m_Settings.default_spawn_chance);

		if (m_Settings.default_tier == "")
		{
			CacheLog.Warning(CacheLog.LOOT, "default_tier пуст — используется civilian.");
			m_Settings.default_tier = "civilian";
		}
	}

	protected void LoadTiers()
	{
		m_Tiers.Clear();

		if (!FileExist(FILE_TIERS))
		{
			CacheModTiersFileJson defaults = CacheModConfigDefaults.BuildTiers();
			SaveTiersFile(defaults);
			IndexTiers(defaults);
			CacheLog.Info(CacheLog.LOOT, "loot_tiers.json создан со значениями по умолчанию.");
			return;
		}

		CacheModTiersFileJson loaded;
		string error;
		if (!CacheModJsonFileIO.LoadTiers(FILE_TIERS, loaded, error))
		{
			CacheLog.Error(CacheLog.LOOT, "loot_tiers.json не прочитан (" + error + "). Лут будет пустым.");
			return;
		}

		IndexTiers(loaded);
	}

	protected void IndexTiers(CacheModTiersFileJson file)
	{
		if (!file || !file.tiers)
			return;

		for (int i = 0; i < file.tiers.Count(); i++)
		{
			CacheModTierJson tier = file.tiers.Get(i);
			if (!tier)
				continue;

			if (tier.id == "")
			{
				CacheLog.Warning(CacheLog.LOOT, "Тир без id пропущен.");
				continue;
			}

			if (GetTier(tier.id))
			{
				CacheLog.Warning(CacheLog.LOOT, "Тир '" + tier.id + "' объявлен дважды — вторая запись пропущена.");
				continue;
			}

			if (!tier.items)
				tier.items = new array<ref CacheModLootItemJson>();

			SanitizeTier(tier);
			m_Tiers.Insert(tier.id, tier);
		}
	}

	protected void SanitizeTier(CacheModTierJson tier)
	{
		if (tier.min_items < 0)
			tier.min_items = 0;

		if (tier.max_items < tier.min_items)
			tier.max_items = tier.min_items;

		for (int i = tier.items.Count() - 1; i >= 0; i--)
		{
			CacheModLootItemJson item = tier.items.Get(i);

			if (!item || item.classname == "")
			{
				CacheLog.Warning(CacheLog.LOOT, "Тир '" + tier.id + "': запись без classname удалена.");
				tier.items.Remove(i);
				continue;
			}

			// A non-positive weight can never be drawn and would only skew the
			// total. Dropping it says so once instead of leaving an admin
			// wondering why an item never appears.
			if (item.weight <= 0)
			{
				CacheLog.Warning(CacheLog.LOOT, "Тир '" + tier.id + "': '" + item.classname + "' имеет weight <= 0 и пропущен.");
				tier.items.Remove(i);
				continue;
			}

			if (item.min < 1)
				item.min = 1;

			if (item.max < item.min)
				item.max = item.min;

			if (!item.attachments)
				item.attachments = new array<ref CacheModAttachmentJson>();

			for (int a = item.attachments.Count() - 1; a >= 0; a--)
			{
				CacheModAttachmentJson attachment = item.attachments.Get(a);

				if (!attachment || attachment.classname == "")
				{
					item.attachments.Remove(a);
					continue;
				}

				attachment.chance = ClampChance(attachment.chance);

				if (attachment.quantity < 1)
					attachment.quantity = 1;
			}
		}
	}

	protected void LoadZones()
	{
		m_Zones.Clear();

		if (!FileExist(FILE_ZONES))
		{
			CacheModZonesFileJson defaults = CacheModConfigDefaults.BuildZones();
			SaveZonesFile(defaults);
			IndexZones(defaults);
			CacheLog.Info(CacheLog.ZONE, "loot_zones.json создан со значениями по умолчанию.");
			return;
		}

		CacheModZonesFileJson loaded;
		string error;
		if (!CacheModJsonFileIO.LoadZones(FILE_ZONES, loaded, error))
		{
			CacheLog.Error(CacheLog.ZONE, "loot_zones.json не прочитан (" + error + "). Все тайники будут использовать default_tier.");
			return;
		}

		IndexZones(loaded);
	}

	protected void IndexZones(CacheModZonesFileJson file)
	{
		if (!file || !file.zones)
			return;

		for (int i = 0; i < file.zones.Count(); i++)
		{
			CacheModZoneJson zone = file.zones.Get(i);
			if (!zone)
				continue;

			if (zone.id == "")
			{
				CacheLog.Warning(CacheLog.ZONE, "Зона без id пропущена.");
				continue;
			}

			if (zone.radius <= 0)
			{
				CacheLog.Warning(CacheLog.ZONE, "Зона '" + zone.id + "' имеет radius <= 0 и пропущена.");
				continue;
			}

			// A zone naming a tier nobody defined would silently pay out
			// nothing. It is dropped instead, so the caches inside it fall back
			// to default_tier and keep working.
			if (!GetTier(zone.tier))
			{
				CacheLog.Warning(CacheLog.ZONE, "Зона '" + zone.id + "' ссылается на несуществующий тир '" + zone.tier + "' и пропущена.");
				continue;
			}

			vector center;
			if (!CacheModCoords.Parse(zone.center, center))
			{
				CacheLog.Warning(CacheLog.ZONE, "Зона '" + zone.id + "': координата '" + zone.center + "' не прочитана, зона пропущена.");
				continue;
			}

			if (zone.min_items < 0)
				zone.min_items = 0;

			if (zone.max_items < zone.min_items)
				zone.max_items = zone.min_items;

			m_Zones.Insert(zone);
		}
	}

	protected void LoadCaches()
	{
		m_Caches.Clear();

		if (!FileExist(FILE_CACHES))
		{
			CacheModCachesFileJson empty = CacheModConfigDefaults.BuildCaches();
			SaveCachesFile(empty);
			CacheLog.Info(CacheLog.ROOT, "caches.json создан пустым — разместите тайники книгой администратора.");
			return;
		}

		CacheModCachesFileJson loaded;
		string error;
		if (!CacheModJsonFileIO.LoadCaches(FILE_CACHES, loaded, error))
		{
			CacheLog.Error(CacheLog.ROOT, "caches.json не прочитан (" + error + "). Тайники не загружены.");
			return;
		}

		if (!loaded || !loaded.caches)
			return;

		for (int i = 0; i < loaded.caches.Count(); i++)
		{
			CacheModCacheJson cache = loaded.caches.Get(i);
			if (!cache)
				continue;

			if (cache.id == "")
			{
				CacheLog.Warning(CacheLog.ROOT, "Тайник без id пропущен.");
				continue;
			}

			if (FindCache(cache.id))
			{
				CacheLog.Warning(CacheLog.ROOT, "Тайник '" + cache.id + "' объявлен дважды — вторая запись пропущена.");
				continue;
			}

			vector position;
			if (!CacheModCoords.Parse(cache.position, position))
			{
				CacheLog.Warning(CacheLog.ROOT, "Тайник '" + cache.id + "': координата '" + cache.position + "' не прочитана, пропущен.");
				continue;
			}

			cache.radius = ClampRadius(cache.radius);
			cache.spawn_chance = ClampChance(cache.spawn_chance);

			// An unknown tool name degrades to "no tool needed" rather than to
			// an unsearchable point: the table of tools is what the client is
			// told about, and a name outside it can never be matched.
			if (cache.required_tool != "")
			{
				int toolIndex = CacheModTools.GetIndex(cache.required_tool);
				if (toolIndex == CacheModTools.NONE)
				{
					CacheLog.Warning(CacheLog.ROOT, "Тайник '" + cache.id + "': инструмент '" + cache.required_tool + "' не входит в таблицу и сброшен.");
					cache.required_tool = "";
				}
			}

			m_Caches.Insert(cache);
		}
	}

	// =====================================================================
	// Writing
	// =====================================================================
	CacheModCacheJson FindCache(string cacheId)
	{
		for (int i = 0; i < m_Caches.Count(); i++)
		{
			CacheModCacheJson cache = m_Caches.Get(i);
			if (cache && cache.id == cacheId)
				return cache;
		}

		return null;
	}

	void AddCache(CacheModCacheJson cache)
	{
		m_Caches.Insert(cache);
	}

	bool RemoveCache(string cacheId)
	{
		for (int i = 0; i < m_Caches.Count(); i++)
		{
			CacheModCacheJson cache = m_Caches.Get(i);
			if (cache && cache.id == cacheId)
			{
				m_Caches.Remove(i);
				return true;
			}
		}

		return false;
	}

	// Builds an id nothing else is using. Sequential rather than random so the
	// file reads in the order the admin placed them.
	string BuildNextCacheId()
	{
		int index = m_Caches.Count() + 1;

		while (index < 100000)
		{
			string candidate = "cache_" + PadNumber(index);
			if (!FindCache(candidate))
				return candidate;

			index = index + 1;
		}

		return "cache_" + CacheModClock.GetUtcSeconds().ToString();
	}

	bool SaveCaches()
	{
		CacheModCachesFileJson file = new CacheModCachesFileJson();
		file.caches = m_Caches;
		return SaveCachesFile(file);
	}

	protected bool SaveCachesFile(CacheModCachesFileJson file)
	{
		string error;
		if (CacheModJsonFileIO.SaveCaches(FILE_CACHES, file, error))
			return true;

		ReportWriteFailure("caches.json", error);
		return false;
	}

	protected bool SaveSettings()
	{
		string error;
		if (CacheModJsonFileIO.SaveSettings(FILE_SETTINGS, m_Settings, error))
			return true;

		ReportWriteFailure("settings.json", error);
		return false;
	}

	protected bool SaveZonesFile(CacheModZonesFileJson file)
	{
		string error;
		if (CacheModJsonFileIO.SaveZones(FILE_ZONES, file, error))
			return true;

		ReportWriteFailure("loot_zones.json", error);
		return false;
	}

	protected bool SaveTiersFile(CacheModTiersFileJson file)
	{
		string error;
		if (CacheModJsonFileIO.SaveTiers(FILE_TIERS, file, error))
			return true;

		ReportWriteFailure("loot_tiers.json", error);
		return false;
	}

	// Said once. A profile folder that cannot be written stays unwritable, and
	// repeating it on every cache placement would bury the line that matters.
	protected void ReportWriteFailure(string fileName, string error)
	{
		if (!m_ProfileWritable)
			return;

		m_ProfileWritable = false;
		CacheLog.Error(CacheLog.ROOT, "Запись " + fileName + " не удалась (" + error + "). Изменения не переживут рестарт.");
	}

	// =====================================================================
	// Small shared helpers
	// =====================================================================
	static float ClampChance(float value)
	{
		if (value < 0.0)
			return 0.0;

		if (value > 100.0)
			return 100.0;

		return value;
	}

	static float ClampRadius(float value)
	{
		if (value < CacheModRPC.MIN_CACHE_RADIUS)
			return CacheModRPC.MIN_CACHE_RADIUS;

		if (value > CacheModRPC.MAX_CACHE_RADIUS)
			return CacheModRPC.MAX_CACHE_RADIUS;

		return value;
	}

	protected static string PadNumber(int value)
	{
		string text = value.ToString();

		while (text.Length() < 3)
			text = "0" + text;

		return text;
	}
}
