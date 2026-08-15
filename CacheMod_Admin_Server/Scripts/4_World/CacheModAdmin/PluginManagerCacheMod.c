// Registers the Cache Mod server-side VPP plugin after VPP's own plugins.
// PermissionManager is registered by VPP inside super.Init(), so the Cache
// Manager plugin has to come after it — hence the call order here.

modded class PluginManager
{
	override void Init()
	{
		super.Init();

		if (GetGame().IsDedicatedServer())
			RegisterPlugin("CacheModAdminPlugin", false, true);
	}
}
