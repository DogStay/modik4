// JobsMod button in VPP Admin Tools.
// Kept in the exact form used by the official VPP external-module example.

modded class VPPAdminHud
{
    override void DefineButtons()
    {
        super.DefineButtons();
        InsertButton("MenuJobsModAdmin", "JobsMod", "set:dayz_gui_vpp image:vpp_icon_settings", "JobsMod configuration");
    }
};
