// Registers the JobsMod server-side VPP plugin after VPP's own plugins.
// PermissionManager is registered by VPP inside super.Init().

modded class PluginManager
{
	override void Init()
	{
		super.Init();

		if (GetGame().IsDedicatedServer())
			RegisterPlugin("JobsModAdminPlugin", false, true);
	}
}
