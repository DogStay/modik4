// Optional VPP Admin Tools server integration for Cache Mod.
//
// Loaded through -serverMod. It is split from the client menu so ordinary
// clients never compile against CacheMod_Server-only classes, and split from
// CacheMod_Server itself so a server can run the caches without VPP at all —
// in that setup the Steam64 whitelist in admins.json is what grants placement.

class CfgPatches
{
	class CacheMod_Admin_Server
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] =
		{
			"DZ_Data",
			"DZ_Scripts",
			"CacheMod_Client",
			"CacheMod_Server",
			"DZM_VPPAdminToolsScripts"
		};
	};
};

class CfgMods
{
	class CacheMod_Admin_Server
	{
		dir = "CacheMod_Admin_Server";
		picture = "";
		action = "";
		hideName = 1;
		hidePicture = 1;
		name = "Cache Mod VPP Admin Server";
		credits = "";
		author = "";
		authorID = "0";
		extra = 0;
		type = "mod";
		dependencies[] = {"World"};

		class defs
		{
			class worldScriptModule
			{
				value = "";
				files[] = {"CacheMod_Admin_Server/Scripts/4_World"};
			};
		};
	};
};
