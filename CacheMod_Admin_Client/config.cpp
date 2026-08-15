// Optional VPP Admin Tools client integration for Cache Mod.
// Load with the normal -mod list together with CacheMod_Client and VPP.
//
// It carries the Cache Manager menu and the debug spheres. Neither is needed to
// play: an ordinary client without this PBO still finds and searches caches,
// and never receives the point list that the spheres are drawn from.

class CfgPatches
{
	class CacheMod_Admin_Client
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] =
		{
			"DZ_Data",
			"DZ_Scripts",
			"CacheMod_Client",
			"DZM_VPPAdminToolsScripts"
		};
	};
};

class CfgMods
{
	class CacheMod_Admin_Client
	{
		dir = "CacheMod_Admin_Client";
		picture = "";
		action = "";
		hideName = 1;
		hidePicture = 1;
		name = "Cache Mod VPP Admin Client";
		credits = "";
		author = "";
		authorID = "0";
		extra = 0;
		type = "mod";
		dependencies[] = {"Mission"};

		class defs
		{
			class missionScriptModule
			{
				value = "";
				files[] = {"CacheMod_Admin_Client/Scripts/5_Mission"};
			};
		};
	};
};
