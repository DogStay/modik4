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
		units[] = {"JobsMod_TrashPile", "JobsMod_CargoBox", "JobsMod_Money"};
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
		version = "2.1.0";
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
// Both classes inherit vanilla parents instead of shipping their own .p3d:
// the mod carries no models, so a hand-written model path would point at a
// file that does not exist and creating the item would fail at runtime.
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
	// Its cargo space is removed on purpose: the box is freight to be moved, not
	// a container players could use to smuggle loot across the map.
	class JobsMod_CargoBox: WoodenCrate
	{
		scope = 2;
		displayName = "Грузовой ящик";
		descriptionShort = "Груз для переноски на склад. Ставится на землю в зоне разгрузки.";
		weight = 20000;
		itemSize[] = {10, 10};
		itemsCargoSize[] = {0, 0};
		rotationFlags = 1;
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
