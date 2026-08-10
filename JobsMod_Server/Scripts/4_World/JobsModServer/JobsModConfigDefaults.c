// JobsModConfigDefaults.c
//
// The configuration a fresh install starts from: three employers and one
// recipient, three jobs, three trash points and one freight route.
//
// It is written to disk on first run so an admin edits a real, working file
// instead of a blank page, and the same values are used in memory when the
// profile folder cannot be written to. Both paths come from here so a
// non-writable profile can never behave differently from a writable one.
//
// The coordinates are Chernarus. On any other map they are simply wrong, which
// is why the startup log always prints where the piles and NPCs actually ended
// up — the numbers either match where you stand or they do not.
//
// Positions are written as strings in the same shape the config reads back, so
// the first thing an admin sees in the file is the format they are meant to
// paste #position output into.

class JobsModConfigDefaults
{
	static const string ZONE_CHERNO = "zone_01";
	static const string ZONE_ELEKTRO = "zone_02";
	static const string ZONE_BEREZINO = "zone_03";

	static const string JOB_SORTER = "job_trash_sorter";
	static const string JOB_LOADER = "job_cargo_loader";
	static const string JOB_MESSENGER = "job_courier";
	static const string JOB_GUARD = "job_guard";
	static const string JOB_HUNTER = "job_hunter";

	static const string AREA_ELEKTRO = "loader_area_01";
	static const string POST_ELEKTRO = "guard_post_01";

	// The currency a fresh install pays in.
	static const string DEFAULT_REWARD_CLASS = "MAODev_Money_Dollar1";

	static const string NPC_SORTER = "npc_sorter_01";
	static const string NPC_LOADER = "npc_loader_01";
	static const string NPC_DISPATCHER = "npc_dispatcher_01";
	static const string NPC_RECIPIENT = "npc_recipient_01";

	// settings.json in full: the switches and everything jobs and NPCs point at.
	static JobsModSettingsJson BuildSettings()
	{
		JobsModSettingsJson settings = new JobsModSettingsJson();

		settings.pile_respawn_seconds = 300;
		settings.assignment_timeout_seconds = 3600;
		// On out of the box: the first thing anyone does with a fresh install is
		// find out whether it works at all.
		settings.debug_logging = true;
		settings.reward_class = DEFAULT_REWARD_CLASS;

		settings.zones = new array<ref JobsModZoneJson>();
		settings.zones.Insert(MakeZone(ZONE_CHERNO, "Черногорск"));
		settings.zones.Insert(MakeZone(ZONE_ELEKTRO, "Электрозаводск"));
		settings.zones.Insert(MakeZone(ZONE_BEREZINO, "Березино"));

		settings.pile_points = new array<ref JobsModPilePointJson>();
		settings.pile_points.Insert(MakePilePoint("pile_01", ZONE_CHERNO, "Куча у площади", "6600 0 2500"));
		settings.pile_points.Insert(MakePilePoint("pile_02", ZONE_CHERNO, "Куча у здания", "6650 0 2550"));
		settings.pile_points.Insert(MakePilePoint("pile_03", ZONE_CHERNO, "Куча у дороги", "6560 0 2460"));

		settings.loader_areas = new array<ref JobsModLoaderAreaJson>();
		settings.loader_areas.Insert(MakeLoaderArea(AREA_ELEKTRO, "Склад Электрозаводска", ZONE_ELEKTRO,
			"10400 0 2200", 12.0,
			"10500 0 2300", 10.0));

		settings.guard_posts = new array<ref JobsModGuardPostJson>();
		settings.guard_posts.Insert(MakeGuardPost(POST_ELEKTRO, "Пост у склада", ZONE_ELEKTRO,
			"10450 0 2250", 15.0));

		return settings;
	}

	static JobsModGuardPostJson MakeGuardPost(string id, string name, string zoneId, string position, float radius)
	{
		JobsModGuardPostJson post = new JobsModGuardPostJson();
		post.id = id;
		post.name = name;
		post.zone_id = zoneId;
		post.position = position;
		post.radius = radius;
		return post;
	}

	static array<ref JobsModJobJson> BuildJobs()
	{
		array<ref JobsModJobJson> jobs = new array<ref JobsModJobJson>();

		JobsModJobJson sorter = new JobsModJobJson();
		sorter.id = JOB_SORTER;
		sorter.name = "Сортировщик мусора";
		sorter.description = "Найдите один предмет мусора в Черногорске и отсортируйте его.";
		sorter.type = JobsModJobType.TEXT_SORTING;
		sorter.zone_id = ZONE_CHERNO;
		sorter.reward = 100;
		sorter.cooldown_seconds = 300;
		sorter.piles_required = 1;
		sorter.equipment = new array<string>();
		sorter.collect_classes = new array<string>();
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
		loader.equipment = new array<string>();
		loader.collect_classes = new array<string>();
		jobs.Insert(loader);

		// The zone of a courier job is where it ends, not where it starts: it is
		// what the HUD names, and the only place the player has to get to.
		//
		// Half an hour of cooldown against roughly four kilometres of walking is
		// the intended shape of this job — a long errand you do once in a while,
		// not a loop to farm.
		JobsModJobJson courier = new JobsModJobJson();
		courier.id = JOB_MESSENGER;
		courier.name = "Посыльный";
		courier.description = "Доставьте опечатанный пакет в Электрозаводск. Пакет не выложить и не открыть.";
		courier.type = JobsModJobType.TEXT_MESSENGER;
		courier.zone_id = ZONE_ELEKTRO;
		courier.reward = 250;
		courier.cooldown_seconds = 1800;
		courier.target_npc_id = NPC_RECIPIENT;
		courier.package_class = "JobsMod_Parcel";
		courier.equipment = new array<string>();
		courier.collect_classes = new array<string>();
		jobs.Insert(courier);

		// Fifteen minutes of standing still, which is long enough to be a
		// commitment and short enough to fit in one session. The kit is
		// deliberately plain vanilla gear: it is taken back when the contract
		// ends, so nothing here needs to be worth keeping.
		JobsModJobJson guard = new JobsModJobJson();
		guard.id = JOB_GUARD;
		guard.name = "Охранник";
		guard.description = "Держите пост у склада до конца смены.";
		guard.type = JobsModJobType.TEXT_GUARD;
		guard.zone_id = ZONE_ELEKTRO;
		guard.reward = 200;
		guard.cooldown_seconds = 900;
		guard.guard_post_id = POST_ELEKTRO;
		guard.guard_seconds = 900;
		guard.equipment = new array<string>();
		guard.equipment.Insert("PoliceCap");
		guard.equipment.Insert("BaseballBat");
		guard.collect_classes = new array<string>();
		jobs.Insert(guard);

		// The one job the mod supplies nothing for. Where the meat comes from —
		// hunting, looting, a stash, another player — is deliberately not asked,
		// which is what makes it the job that pays for what someone was doing
		// anyway. Several classes so it reads as "meat" and not as one animal.
		JobsModJobJson hunter = new JobsModJobJson();
		hunter.id = JOB_HUNTER;
		hunter.name = "Заготовщик мяса";
		hunter.description = "Принесите 30 кусков мяса. Откуда — ваше дело.";
		hunter.type = JobsModJobType.TEXT_COLLECT;
		hunter.zone_id = ZONE_CHERNO;
		hunter.reward = 300;
		hunter.cooldown_seconds = 1200;
		hunter.collect_required = 30;
		hunter.collect_label = "30 кусков мяса";
		hunter.collect_classes = new array<string>();
		hunter.collect_classes.Insert("DeerSteakMeat");
		hunter.collect_classes.Insert("PigSteakMeat");
		hunter.collect_classes.Insert("CowSteakMeat");
		hunter.collect_classes.Insert("GoatSteakMeat");
		hunter.collect_classes.Insert("SheepSteakMeat");
		hunter.collect_classes.Insert("BoarSteakMeat");
		hunter.equipment = new array<string>();
		jobs.Insert(hunter);

		return jobs;
	}

	static array<ref JobsModNpcJson> BuildNpcs()
	{
		array<ref JobsModNpcJson> npcs = new array<ref JobsModNpcJson>();

		JobsModNpcJson sorter = new JobsModNpcJson();
		sorter.id = NPC_SORTER;
		sorter.name = "Иван";
		sorter.description = "Нанимаю на уборку. Плачу за разобранный мусор.";
		sorter.position = "6620 0 2520";
		sorter.rotation = 45.0;
		sorter.player_class = "SurvivorM_Mirek";
		sorter.invulnerable = true;
		sorter.clothing = new array<string>();
		sorter.clothing.Insert("TShirt_Grey");
		sorter.clothing.Insert("Jeans_Blue");
		sorter.clothing.Insert("WorkingBoots_Grey");
		sorter.jobs = new array<string>();
		sorter.jobs.Insert(JOB_HUNTER);
		sorter.jobs.Insert(JOB_SORTER);
		npcs.Insert(sorter);

		JobsModNpcJson loader = new JobsModNpcJson();
		loader.id = NPC_LOADER;
		loader.name = "Пётр";
		loader.description = "Нужны руки на складе. Ящики сами себя не перенесут.";
		loader.position = "10420 0 2220";
		loader.rotation = 180.0;
		loader.player_class = "SurvivorM_Boris";
		loader.invulnerable = true;
		loader.clothing = new array<string>();
		loader.clothing.Insert("Hoodie_Black");
		loader.clothing.Insert("CargoPants_Grey");
		loader.clothing.Insert("WorkingBoots_Yellow");
		loader.jobs = new array<string>();
		loader.jobs.Insert(JOB_LOADER);
		// The same foreman hires the guard for the yard he runs, so a fresh
		// install has both jobs reachable from one person.
		loader.jobs.Insert(JOB_GUARD);
		npcs.Insert(loader);

		JobsModNpcJson dispatcher = new JobsModNpcJson();
		dispatcher.id = NPC_DISPATCHER;
		dispatcher.name = "Марина";
		dispatcher.description = "Отправляю бумаги по побережью. Нужен человек с ногами.";
		dispatcher.position = "6644 0 2544";
		dispatcher.rotation = 270.0;
		dispatcher.player_class = "SurvivorF_Judy";
		dispatcher.invulnerable = true;
		dispatcher.clothing = new array<string>();
		dispatcher.clothing.Insert("RaincoatJacket_Yellow");
		dispatcher.clothing.Insert("CanvasPants_Beige");
		dispatcher.clothing.Insert("AthleticShoes_Black");
		dispatcher.jobs = new array<string>();
		dispatcher.jobs.Insert(JOB_MESSENGER);
		npcs.Insert(dispatcher);

		// Hands out nothing at all: this one exists to be walked to. An empty
		// jobs list is legal precisely because a courier job names him, and the
		// config drops him again the day that job goes away.
		JobsModNpcJson recipient = new JobsModNpcJson();
		recipient.id = NPC_RECIPIENT;
		recipient.name = "Гриша";
		recipient.description = "Жду посылку от Марины. Больше ничего не жду.";
		recipient.position = "10484 0 2264";
		recipient.rotation = 90.0;
		recipient.player_class = "SurvivorM_Denis";
		recipient.invulnerable = true;
		recipient.clothing = new array<string>();
		recipient.clothing.Insert("Hoodie_Blue");
		recipient.clothing.Insert("Jeans_Black");
		recipient.clothing.Insert("WorkingBoots_Brown");
		recipient.jobs = new array<string>();
		npcs.Insert(recipient);

		return npcs;
	}

	protected static JobsModZoneJson MakeZone(string id, string name)
	{
		JobsModZoneJson zone = new JobsModZoneJson();
		zone.id = id;
		zone.name = name;
		return zone;
	}

	protected static JobsModPilePointJson MakePilePoint(string id, string zoneId, string name, string position)
	{
		JobsModPilePointJson point = new JobsModPilePointJson();
		point.id = id;
		point.zone_id = zoneId;
		point.name = name;
		point.position = position;
		return point;
	}

	protected static JobsModLoaderAreaJson MakeLoaderArea(
		string id, string name, string zoneId,
		string source, float sourceRadius,
		string destination, float destinationRadius)
	{
		JobsModLoaderAreaJson area = new JobsModLoaderAreaJson();
		area.id = id;
		area.name = name;
		area.zone_id = zoneId;
		area.source = source;
		area.source_radius = sourceRadius;
		area.destination = destination;
		area.destination_radius = destinationRadius;
		return area;
	}
}
