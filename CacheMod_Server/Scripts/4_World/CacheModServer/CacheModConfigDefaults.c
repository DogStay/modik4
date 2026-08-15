// CacheModConfigDefaults.c
//
// What lands in $profile:CacheMod on a server that has never run the mod.
//
// The defaults are meant to be playable immediately and obvious to edit: two
// tiers, one example zone, no caches. Caches are the one thing not seeded — a
// point invented by the mod would sit somewhere nobody chose, and placing them
// is the admin's first job anyway.

class CacheModConfigDefaults
{
	static CacheModSettingsJson BuildSettings()
	{
		CacheModSettingsJson settings = new CacheModSettingsJson();

		settings.search_duration_seconds = 7.0;
		settings.default_tier = "civilian";
		settings.tool_in_hands_only = true;
		settings.respawn_seconds = 3600;
		settings.default_spawn_chance = 25.0;
		settings.debug_logging = false;

		settings.chest_settings = new CacheModChestSettingsJson();
		settings.chest_settings.lifetime_seconds = 900;
		settings.chest_settings.despawn_when_empty = true;
		settings.chest_settings.height_offset = 0.0;
		settings.chest_settings.chest_class = "CacheLootChest";

		return settings;
	}

	static CacheModCachesFileJson BuildCaches()
	{
		CacheModCachesFileJson file = new CacheModCachesFileJson();
		file.caches = new array<ref CacheModCacheJson>();
		return file;
	}

	// One zone, placed nowhere in particular and switched off by having a tiny
	// radius: it is there to be edited, not to take effect. An admin who wants
	// it live moves the centre and widens the radius.
	static CacheModZonesFileJson BuildZones()
	{
		CacheModZonesFileJson file = new CacheModZonesFileJson();
		file.zones = new array<ref CacheModZoneJson>();

		CacheModZoneJson example = new CacheModZoneJson();
		example.id = "example_military";
		example.display_name = "Пример: военная зона";
		example.center = "0 0 0";
		example.radius = 1.0;
		example.tier = "military";
		example.priority = 100;
		example.min_items = 3;
		example.max_items = 6;
		file.zones.Insert(example);

		return file;
	}

	// Two tiers with vanilla classnames only, so the defaults work on a server
	// running no other mods.
	static CacheModTiersFileJson BuildTiers()
	{
		CacheModTiersFileJson file = new CacheModTiersFileJson();
		file.tiers = new array<ref CacheModTierJson>();

		CacheModTierJson civilian = new CacheModTierJson();
		civilian.id = "civilian";
		civilian.display_name = "Гражданский";
		civilian.min_items = 2;
		civilian.max_items = 4;
		civilian.items = new array<ref CacheModLootItemJson>();
		AddItem(civilian, "BandageDressing", 30.0, 1, 2);
		AddItem(civilian, "TunaCan", 25.0, 1, 2);
		AddItem(civilian, "WaterBottle", 20.0, 1, 1);
		AddItem(civilian, "Rope", 10.0, 1, 1);
		AddItem(civilian, "Screwdriver", 10.0, 1, 1);
		AddItem(civilian, "Matchbox", 5.0, 1, 1);
		file.tiers.Insert(civilian);

		CacheModTierJson military = new CacheModTierJson();
		military.id = "military";
		military.display_name = "Военный";
		military.min_items = 2;
		military.max_items = 5;
		military.items = new array<ref CacheModLootItemJson>();

		CacheModLootItemJson rifle = AddItem(military, "AKM", 15.0, 1, 1);
		AddAttachment(rifle, "Mag_AKM_30Rnd", 60.0, 2);
		AddAttachment(rifle, "AK_WoodBttstck", 40.0, 1);

		AddItem(military, "AmmoBox_545x39_20Rnd", 20.0, 1, 2);
		AddItem(military, "TacticalBaconCan", 20.0, 1, 2);
		AddItem(military, "FirstAidKit", 20.0, 1, 1);
		AddItem(military, "PlateCarrierVest", 15.0, 1, 1);
		AddItem(military, "NVGoggles", 10.0, 1, 1);
		file.tiers.Insert(military);

		return file;
	}

	protected static CacheModLootItemJson AddItem(CacheModTierJson tier, string className, float weight, int minCount, int maxCount)
	{
		CacheModLootItemJson item = new CacheModLootItemJson();
		item.classname = className;
		item.weight = weight;
		item.min = minCount;
		item.max = maxCount;
		item.attachments = new array<ref CacheModAttachmentJson>();

		tier.items.Insert(item);
		return item;
	}

	protected static void AddAttachment(CacheModLootItemJson item, string className, float chance, int quantity)
	{
		CacheModAttachmentJson attachment = new CacheModAttachmentJson();
		attachment.classname = className;
		attachment.chance = chance;
		attachment.quantity = quantity;

		item.attachments.Insert(attachment);
	}
}
