// JobsMod_Server — server part of the mod.
//
// Loaded ONLY by the server via -serverMod=@JobsMod_Server. Players never
// download it, so nothing here may be required to render anything client-side.
//
// It owns all gameplay authority: job assignment, minigame sessions, result
// validation and rewards. It depends on JobsMod_Client because the shared
// contract (RPC ids, trash catalog) and the item classes live there.

class CfgPatches
{
	class JobsMod_Server
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"DZ_Data", "DZ_Scripts", "JobsMod_Client"};
	};
};

class CfgMods
{
	class JobsMod_Server
	{
		dir = "JobsMod_Server";
		picture = "";
		action = "";
		hideName = 1;
		hidePicture = 1;
		name = "JobsMod Server";
		credits = "";
		author = "";
		authorID = "0";
		version = "2.0.0";
		extra = 0;
		type = "mod";
		dependencies[] = {"Game", "World", "Mission"};

		class defs
		{
			// JSON data classes and the JsonFileLoader wrapper live in 3_Game on
			// purpose: instantiating the generic loader from 4_World has been
			// observed to fail compilation under a heavy mod load, while the
			// same call compiles cleanly in its native Game module.
			class gameScriptModule
			{
				value = "";
				files[] = {"JobsMod_Server/Scripts/3_Game"};
			};

			// Server core. PlayerBase and ItemBase exist from 4_World onwards,
			// so every class touching them must live here and not in 3_Game.
			class worldScriptModule
			{
				value = "";
				files[] = {"JobsMod_Server/Scripts/4_World"};
			};

			// Mission bootstrap: creates and owns the server services.
			class missionScriptModule
			{
				value = "";
				files[] = {"JobsMod_Server/Scripts/5_Mission"};
			};
		};
	};
};
