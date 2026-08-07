// JobsModConfigDefaults.c
//
// The configuration a fresh install starts from: two employers, two jobs, three
// trash points and one freight route.
//
// It is written to disk on first run so an admin edits a real, working file
// instead of a blank page, and the same values are used in memory when the
// profile folder cannot be written to. Both paths come from here so a
// non-writable profile can never behave differently from a writable one.
//
// The coordinates are Chernarus. On any other map they are simply wrong, which
// is why the startup log always prints where the piles and NPCs actually ended
// up — the numbers either match where you stand or they do not.

class JobsModConfigDefaults
{
	static const string ZONE_CHERNO = "zone_01";
	static const string ZONE_ELEKTRO = "zone_02";
	static const string ZONE_BEREZINO = "zone_03";

	static const string JOB_SORTER = "job_trash_sorter";
	static const string JOB_LOADER = "job_cargo_loader";

	static const string AREA_ELEKTRO = "loader_area_01";

	static const string NPC_SORTER = "npc_sorter_01";
	static const string NPC_LOADER = "npc_loader_01";

	static JobsModSettingsJson BuildSettings()
	{
		JobsModSettingsJson settings = new JobsModSettingsJson();
		settings.pile_respawn_seconds = 300;
		settings.assignment_timeout_seconds = 3600;
		// On out of the box: the first thing anyone does with a fresh install is
		// find out whether it works at all.
		settings.debug_logging = true;
		return settings;
	}

	static JobsModZoneListJson BuildZones()
	{
		JobsModZoneListJson list = new JobsModZoneListJson();
		list.zones = new array<ref JobsModZoneJson>();
		list.zones.Insert(MakeZone(ZONE_CHERNO, "Черногорск"));
		list.zones.Insert(MakeZone(ZONE_ELEKTRO, "Электрозаводск"));
		list.zones.Insert(MakeZone(ZONE_BEREZINO, "Березино"));
		return list;
	}

	static array<ref JobsModPilePointJson> BuildPilePoints()
	{
		array<ref JobsModPilePointJson> points = new array<ref JobsModPilePointJson>();
		points.Insert(MakePilePoint("pile_01", ZONE_CHERNO, "Куча у площади", 6600.0, 0.0, 2500.0));
		points.Insert(MakePilePoint("pile_02", ZONE_CHERNO, "Куча у здания", 6650.0, 0.0, 2550.0));
		points.Insert(MakePilePoint("pile_03", ZONE_CHERNO, "Куча у дороги", 6560.0, 0.0, 2460.0));
		return points;
	}

	static array<ref JobsModLoaderAreaJson> BuildLoaderAreas()
	{
		array<ref JobsModLoaderAreaJson> areas = new array<ref JobsModLoaderAreaJson>();
		areas.Insert(MakeLoaderArea(AREA_ELEKTRO, "Склад Электрозаводска", ZONE_ELEKTRO,
			10400.0, 2200.0, 12.0,
			10500.0, 2300.0, 10.0));
		return areas;
	}

	static array<ref JobsModJobJson> BuildJobs()
	{
		array<ref JobsModJobJson> jobs = new array<ref JobsModJobJson>();

		JobsModJobJson sorter = new JobsModJobJson();
		sorter.id = JOB_SORTER;
		sorter.name = "Сортировщик мусора";
		sorter.description = "Разберите мусор на трёх точках в Черногорске.";
		sorter.type = JobsModJobType.TEXT_SORTING;
		sorter.zone_id = ZONE_CHERNO;
		sorter.reward = 100;
		sorter.cooldown_seconds = 300;
		sorter.piles_required = 3;
		jobs.Insert(sorter);

		JobsModJobJson loader = new JobsModJobJson();
		loader.id = JOB_LOADER;
		loader.name = "Грузчик";
		loader.description = "Перенесите ящики со склада в зону разгрузки.";
		loader.type = JobsModJobType.TEXT_LOADING;
		loader.zone_id = ZONE_ELEKTRO;
		loader.reward = 150;
		loader.cooldown_seconds = 600;
		loader.loader_area_id = AREA_ELEKTRO;
		loader.cargos_required = 5;
		loader.cargo_class = "JobsMod_CargoBox";
		jobs.Insert(loader);

		return jobs;
	}

	static array<ref JobsModNpcJson> BuildNpcs()
	{
		array<ref JobsModNpcJson> npcs = new array<ref JobsModNpcJson>();

		JobsModNpcJson sorter = new JobsModNpcJson();
		sorter.id = NPC_SORTER;
		sorter.name = "Иван";
		sorter.description = "Нанимаю на уборку. Плачу за разобранный мусор.";
		sorter.position = MakeVector(6620.0, 0.0, 2520.0);
		sorter.rotation = 45.0;
		sorter.player_class = "SurvivorM_Mirek";
		sorter.invulnerable = true;
		sorter.clothing = new array<string>();
		sorter.clothing.Insert("TShirt_Grey");
		sorter.clothing.Insert("Jeans_Blue");
		sorter.clothing.Insert("WorkingBoots_Grey");
		sorter.jobs = new array<string>();
		sorter.jobs.Insert(JOB_SORTER);
		npcs.Insert(sorter);

		JobsModNpcJson loader = new JobsModNpcJson();
		loader.id = NPC_LOADER;
		loader.name = "Пётр";
		loader.description = "Нужны руки на складе. Ящики сами себя не перенесут.";
		loader.position = MakeVector(10420.0, 0.0, 2220.0);
		loader.rotation = 180.0;
		loader.player_class = "SurvivorM_Boris";
		loader.invulnerable = true;
		loader.clothing = new array<string>();
		loader.clothing.Insert("Hoodie_Black");
		loader.clothing.Insert("CargoPants_Grey");
		loader.clothing.Insert("WorkingBoots_Yellow");
		loader.jobs = new array<string>();
		loader.jobs.Insert(JOB_LOADER);
		npcs.Insert(loader);

		return npcs;
	}

	protected static JobsModZoneJson MakeZone(string id, string name)
	{
		JobsModZoneJson zone = new JobsModZoneJson();
		zone.id = id;
		zone.name = name;
		return zone;
	}

	protected static JobsModPilePointJson MakePilePoint(string id, string zoneId, string name, float x, float y, float z)
	{
		JobsModPilePointJson point = new JobsModPilePointJson();
		point.id = id;
		point.zone_id = zoneId;
		point.name = name;
		point.x = x;
		point.y = y;
		point.z = z;
		return point;
	}

	protected static JobsModLoaderAreaJson MakeLoaderArea(
		string id, string name, string zoneId,
		float sourceX, float sourceZ, float sourceRadius,
		float destX, float destZ, float destRadius)
	{
		JobsModLoaderAreaJson area = new JobsModLoaderAreaJson();
		area.id = id;
		area.name = name;
		area.zone_id = zoneId;
		area.source = MakeArea(sourceX, sourceZ, sourceRadius);
		area.destination = MakeArea(destX, destZ, destRadius);
		return area;
	}

	protected static JobsModAreaJson MakeArea(float x, float z, float radius)
	{
		JobsModAreaJson area = new JobsModAreaJson();
		area.x = x;
		area.z = z;
		area.radius = radius;
		return area;
	}

	protected static JobsModVectorJson MakeVector(float x, float y, float z)
	{
		JobsModVectorJson position = new JobsModVectorJson();
		position.x = x;
		position.y = y;
		position.z = z;
		return position;
	}
}
