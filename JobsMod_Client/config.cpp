// JobsMod_Client — client part of the mod.
//
// Loaded by every participant via -mod=@JobsMod_Client (clients AND the server).
// Holds everything the player must physically have: item classes, GUI layouts
// and the shared client/server contract (RPC ids, trash catalog).
//
// The gameplay authority itself lives in JobsMod_Server, which the server
// loads separately via -serverMod=@JobsMod_Server.

class CfgPatches
{
	class JobsMod_Client
	{
		units[] = {"JobsMod_TrashPile", "JobsMod_CargoBox", "JobsMod_Parcel", "JobsMod_Money"};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] =
		{
			"DZ_Data",
			"DZ_Scripts",
			"DZ_Gear_Consumables",
			"DZ_Gear_Containers"
		};
	};
};

class CfgMods
{
	class JobsMod_Client
	{
		dir = "JobsMod_Client";
		picture = "";
		action = "";
		hideName = 1;
		hidePicture = 1;
		name = "JobsMod Client";
		credits = "";
		author = "";
		authorID = "0";
		version = "2.3.0";
		extra = 0;
		type = "mod";
		dependencies[] = {"Game", "World", "Mission"};

		class defs
		{
			// Shared contract: engine-agnostic only. PlayerBase/ItemBase do not
			// exist yet while 3_Game compiles, so nothing here may reference them.
			class gameScriptModule
			{
				value = "";
				files[] = {"JobsMod_Client/Scripts/3_Game"};
			};

			// Client runtime: modded PlayerBase, user actions, client-side state.
			class worldScriptModule
			{
				value = "";
				files[] = {"JobsMod_Client/Scripts/4_World"};
			};

			// Menus and mission wiring.
			class missionScriptModule
			{
				value = "";
				files[] = {"JobsMod_Client/Scripts/5_Mission"};
			};
		};
	};
};

// ---------------------------------------------------------------------------
// Items
//
// Every class here inherits a vanilla parent instead of shipping its own .p3d:
// the mod carries no models, so a hand-written model path would point at a
// file that does not exist and creating the item would fail at runtime. What
// each one is called and what it does is ours; what it looks like is borrowed.
// ---------------------------------------------------------------------------
class CfgVehicles
{
	class WoodenCrate;
	class VitaminBottle;

	// Work point placed by the server inside a janitor zone. Interacting with
	// it opens the sorting minigame for the player who owns the job.
	class JobsMod_TrashPile: WoodenCrate
	{
		scope = 2;
		displayName = "Куча мусора";
		descriptionShort = "Рабочая точка уборки. Доступна сотруднику, взявшему смену.";
		weight = 12000;
		itemSize[] = {10, 10};
		itemsCargoSize[] = {0, 0};
		rotationFlags = 1;
	};

	// Freight for the loader job. Inherits WoodenCrate because that is a vanilla
	// item the engine already carries in both hands with the heavy-item stance —
	// which is exactly the "carrying a generator" look the job is meant to have,
	// with no animation work of our own.
	//
	// The box must live in one of two places and nowhere else: on the ground, or
	// in the player's hands. Three settings enforce that, and it is worth being
	// explicit about which does what:
	//
	//   itemSize     20x20 is larger than any cargo grid in the game, so the fit
	//                check fails everywhere — backpack, vest, tent, car, barrel.
	//                This is what makes "hands only" true rather than merely
	//                inconvenient, and it costs no script class to enforce.
	//   itemsCargoSize
	//                no cargo of its own: freight to be moved, not a container
	//                players could use to smuggle loot across the map.
	//   weight       60 kg. Loaded like that a player cannot sprint and their
	//                stamina drains at once, which is the whole point of the job
	//                being carried rather than driven.
	//   itemBehaviour
	//                2 = heavy: two-handed carry stance, no running with it.
	class JobsMod_CargoBox: WoodenCrate
	{
		scope = 2;
		displayName = "Грузовой ящик";
		descriptionShort = "Тяжёлый груз. Носится только в руках. Ставится на землю в зоне разгрузки.";
		weight = 60000;
		itemSize[] = {20, 20};
		itemsCargoSize[] = {0, 0};
		itemBehaviour = 2;
		rotationFlags = 1;
		canBeSplit = 0;
	};

	// The courier's parcel. Handed out by the server when the job is taken and
	// deleted the moment it ends, however it ends.
	//
	// Everything that makes it undroppable is in the script class of the same
	// name, not here: inventory rules are not expressible in a config, and the
	// item has to refuse a drag rather than merely be awkward to carry. The
	// config's only job is to make it small and light — it is papers in an
	// envelope, and a courier who cannot run is a loader.
	//
	// The parent is a vanilla item because the mod ships no models of its own;
	// this is a placeholder look, and a server that wants a different one sets
	// package_class in the job to any item class it likes. Only JobsMod_Parcel
	// and its descendants carry the inventory rules, though, so anything else
	// is droppable and relies on the server sweep alone.
	//
	// The script class of the same name extends ItemBase rather than whatever
	// the parent's does, which is deliberate: it takes the model and drops the
	// behaviour. A parcel is not something to swallow.
	class JobsMod_Parcel: VitaminBottle
	{
		scope = 2;
		displayName = "Опечатанный пакет";
		descriptionShort = "Курьерское отправление. Вскрывать и передавать третьим лицам запрещено.";
		weight = 300;
		itemSize[] = {2, 2};
		rotationFlags = 1;
		canBeSplit = 0;
		varQuantityInit = 1;
		varQuantityMin = 0;
		varQuantityMax = 1;
		varQuantityDestroyOnMin = 0;
	};

	// Payment token handed out by the server on job completion.
	class JobsMod_Money: VitaminBottle
	{
		scope = 2;
		displayName = "Расчётный жетон";
		descriptionShort = "Выдаётся за выполненную смену.";
		weight = 5;
		itemSize[] = {1, 1};
		rotationFlags = 1;
		varQuantityInit = 1;
		varQuantityMin = 0;
		varQuantityMax = 500;
		varQuantityDestroyOnMin = 1;
		canBeSplit = 1;
		stackedUnitMaxAmount = 500;
	};
};
