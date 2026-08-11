// JobsMod_Admin — the VPP Admin Tools bridge for JobsMod.
//
// A separate PBO on purpose. Registering a plugin with VPP means modding its
// PluginManager, and a modded class of a mod that is not loaded does not
// compile — putting this in JobsMod_Server would make VPP Admin Tools a hard
// requirement for everyone running JobsMod. Here it is optional: load this PBO
// only if you run VPP, and JobsMod is unchanged either way.
//
// Loaded by the server only, like JobsMod_Server: everything it does is
// server-side file work.

class CfgPatches
{
	class JobsMod_Admin
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] =
		{
			"DZ_Data",
			"DZ_Scripts",
			"JobsMod_Client",
			"VPPAdminTools"
		};
	};
};

class CfgMods
{
	class JobsMod_Admin
	{
		dir = "JobsMod_Admin";
		picture = "";
		action = "";
		hideName = 1;
		hidePicture = 1;
		name = "JobsMod Admin";
		credits = "";
		author = "";
		authorID = "0";
		version = "1.0";
		extra = 0;
		type = "mod";
		dependencies[] = {"World"};

		class defs
		{
			class worldScriptModule
			{
				value = "";
				files[] = {"JobsMod_Admin/Scripts/4_World"};
			};
		};
	};
};
