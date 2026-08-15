// CacheMod_Server — the authority.
//
// Loaded ONLY by the server through -serverMod=@CacheMod_Server. Players never
// download it, so nothing here may be needed to render anything client-side.
//
// It owns every decision the mod makes: which caches rolled active this run,
// where the proxies are, whether a finished search counts, which zone and tier
// a position resolves to, what the chest holds and when it goes away. It
// depends on CacheMod_Client because the item classes, the search action and
// the shared contract live there.

class CfgPatches
{
	class CacheMod_Server
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"DZ_Data", "DZ_Scripts", "CacheMod_Client"};
	};
};

class CfgMods
{
	class CacheMod_Server
	{
		dir = "CacheMod_Server";
		picture = "";
		action = "";
		hideName = 1;
		hidePicture = 1;
		name = "Cache Mod Server";
		credits = "";
		author = "";
		authorID = "0";
		extra = 0;
		type = "mod";
		dependencies[] = {"Game", "World", "Mission"};

		class defs
		{
			// JSON data classes and the JsonFileLoader wrapper live in 3_Game
			// deliberately: instantiating the generic loader from 4_World has
			// been seen to fail compilation under a heavy mod load, while the
			// same call compiles cleanly in its native Game module.
			class gameScriptModule
			{
				value = "";
				files[] = {"CacheMod_Server/Scripts/3_Game"};
			};

			// The manager, the services and the RPC receiver. PlayerBase exists
			// from 4_World onwards, so everything touching it lives here.
			class worldScriptModule
			{
				value = "";
				files[] = {"CacheMod_Server/Scripts/4_World"};
			};

			// Mission bootstrap: starts, ticks and stops the runtime.
			class missionScriptModule
			{
				value = "";
				files[] = {"CacheMod_Server/Scripts/5_Mission"};
			};
		};
	};
};
