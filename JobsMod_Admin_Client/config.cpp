// Optional VPP Admin Tools client integration for JobsMod.
// Load with the normal -mod list together with JobsMod_Client and VPP Admin Tools.
// No numbered build/version markers are used.

class CfgPatches
{
	class JobsMod_Admin_Client
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] =
		{
			"DZ_Data",
			"DZ_Scripts",
			"JobsMod_Client",
			"DZM_VPPAdminToolsScripts"
		};
	};
};

class CfgMods
{
	class JobsMod_Admin_Client
	{
		dir = "JobsMod_Admin_Client";
		picture = "";
		action = "";
		hideName = 1;
		hidePicture = 1;
		name = "JobsMod VPP Admin Client";
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
				files[] = {"JobsMod_Admin_Client/Scripts/5_Mission"};
			};
		};
	};
};
