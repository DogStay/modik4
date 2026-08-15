// Cache Manager button inside VPP Admin Tools.
// Kept in the exact form the official VPP external-module example uses.

modded class VPPAdminHud
{
    override void DefineButtons()
    {
        super.DefineButtons();
        InsertButton("MenuCacheManager", "Cache Manager", "set:dayz_gui_vpp image:vpp_icon_settings", "Cache Mod");
    }
};
