// CacheMod_Client — the part everybody has.
//
// Loaded by every participant through -mod=@CacheMod (clients AND the server).
// It carries what has to physically exist on a client: the item classes, the
// search action, and the shared client/server contract.
//
// All authority — where caches are, which ones rolled active, what a search
// produces — lives in CacheMod_Server, which the server loads separately via
// -serverMod=@CacheMod_Server.
//
// The mod ships no models. Every class below inherits a vanilla parent and
// takes its p3d; what each one is called and how it behaves is ours.

class CfgPatches
{
	class CacheMod_Client
	{
		units[] = {"CacheInteractionProxy", "CacheLootChest", "CacheCreatorBook_Basic", "CacheCreatorBook_Tooled"};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] =
		{
			"DZ_Data",
			"DZ_Scripts",
			"DZ_Gear_Containers",
			"DZ_Gear_Consumables"
		};
	};
};

class CfgMods
{
	class CacheMod_Client
	{
		dir = "CacheMod_Client";
		picture = "";
		action = "";
		hideName = 1;
		hidePicture = 1;
		name = "Cache Mod Client";
		credits = "";
		author = "";
		authorID = "0";
		extra = 0;
		type = "mod";
		dependencies[] = {"Game", "World"};

		class defs
		{
			// Shared contract only: PlayerBase and ItemBase do not exist yet
			// while 3_Game compiles, so nothing here may reference them.
			class gameScriptModule
			{
				value = "";
				files[] = {"CacheMod_Client/Scripts/3_Game"};
			};

			// Items, the search action, the client receiver.
			class worldScriptModule
			{
				value = "";
				files[] = {"CacheMod_Client/Scripts/4_World"};
			};
		};
	};
};

class CfgVehicles
{
	class WoodenCrate;
	class SeaChest;
	class VitaminBottle;

	// The invisible interaction point. See CacheInteractionProxy.c for why it
	// borrows a crate: the cursor raycast needs geometry to hit, and a crate is
	// a body roughly the size of the spot a player is meant to find. The visual
	// is hidden in script; the geometry stays.
	//
	// Everything that could turn it into an ordinary item is removed here as
	// well as refused in script: no cargo, no storage category (so the hive has
	// nothing to persist), and an item size larger than any cargo grid in the
	// game, which is what makes "cannot be carried" true rather than merely
	// inconvenient.
	class CacheInteractionProxy: WoodenCrate
	{
		scope = 2;
		displayName = "";
		descriptionShort = "";
		weight = 100000;
		itemSize[] = {30, 30};
		itemBehaviour = 2;
		canBeSplit = 0;
		rotationFlags = 1;
		hiddenSelections[] = {};

		class Cargo
		{
			itemsCargoSize[] = {0, 0};
			openable = 0;
		};

		class DamageSystem
		{
			class GlobalHealth
			{
				class Health
				{
					hitpoints = 1000000;
				};
			};
		};
	};

	// The prize. A sea chest is a vanilla container with a decent cargo grid and
	// a model that reads as "someone stashed this here", which is the whole
	// look the mod needs. Its lifetime is driven by the server, not by the hive.
	class CacheLootChest: SeaChest
	{
		scope = 2;
		displayName = "Тайник";
		descriptionShort = "Спрятанный кем-то ящик. Содержимое лучше забрать быстро.";
		weight = 40000;
		itemSize[] = {20, 20};
		itemBehaviour = 2;
		rotationFlags = 1;
		canBeSplit = 0;
	};

	// The two administrator books. They differ in one thing only: which cache
	// type the point they create is written as. Neither is ever placed on the
	// ground — see CacheCreatorBook.c.
	//
	// The parent is a vanilla consumable rather than one of the book models,
	// because the mod ships nothing of its own and the consumable classes are
	// the ones already proven to inherit cleanly here. It is an admin-only tool
	// that never leaves an administrator's hands, so what it looks like matters
	// less than the class resolving on every server; a server that prefers a
	// book model changes the parent on these two classes and nothing else.
	class CacheCreatorBook_Basic: VitaminBottle
	{
		scope = 2;
		displayName = "Журнал тайников";
		descriptionShort = "Служебный инструмент администрации. Отмечает обычные тайники.";
		weight = 300;
		itemSize[] = {2, 3};
		rotationFlags = 1;
		canBeSplit = 0;
	};

	class CacheCreatorBook_Tooled: VitaminBottle
	{
		scope = 2;
		displayName = "Журнал тайников (инструмент)";
		descriptionShort = "Служебный инструмент администрации. Отмечает тайники, требующие инструмента.";
		weight = 300;
		itemSize[] = {2, 3};
		rotationFlags = 1;
		canBeSplit = 0;
	};
};
