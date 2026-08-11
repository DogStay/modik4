// PluginManagerJobsMod.c
//
// Registers the JobsMod plugin with VPP. This is the file that makes the whole
// PBO depend on VPP Admin Tools, and the reason the PBO is separate: a modded
// class of a mod that is not loaded does not compile.

modded class PluginManager
{
	override void Init()
	{
		super.Init();

		// Server side only. Everything the plugin does is file work in the
		// server's profile folder; a client copy would have nothing to read.
		if (GetGame().IsServer() && GetGame().IsMultiplayer())
			RegisterPlugin("JobsModAdminPlugin", false, true);
	}
}
