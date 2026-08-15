// Optional VPP Admin Tools server integration for JobsMod.
// Load through -serverMod. It is split from the client menu so ordinary
// clients never compile against JobsMod_Server-only classes.
// No numbered build/version markers are used.

class CfgPatches
{
	class JobsMod_Admin_Server
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] =
		{
			"DZ_Data",
			"DZ_Scripts",
			"JobsMod_Client",
			"JobsMod_Server",
			"DZM_VPPAdminToolsScripts"
		};
	};
};

class CfgMods
{
	class JobsMod_Admin_Server
	{
		dir = "JobsMod_Admin_Server";
		picture = "";
		action = "";
		hideName = 1;
		hidePicture = 1;
		name = "JobsMod VPP Admin Server";
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
				files[] = {"JobsMod_Admin_Server/Scripts/4_World"};
			};
		};
	};
};
