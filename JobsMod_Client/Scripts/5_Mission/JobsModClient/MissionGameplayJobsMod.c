// MissionGameplayJobsMod.c
//
// Registers the mod's menus with the mission's menu factory.
//
// The session is handed to the menu here rather than after EnterScriptedMenu
// returns: the manager runs Init() and OnShow() before the caller ever sees the
// instance, and OnShow is where the menu builds itself from the session.

modded class MissionGameplay
{
	override UIScriptedMenu CreateScriptedMenu(int id)
	{
		if (id == JobsModMenuIds.TRASH_SORTING)
		{
			TrashSortingMenu menu = new TrashSortingMenu();
			menu.SetSession(JobsModClientContext.ConsumePendingSession());
			return menu;
		}

		return super.CreateScriptedMenu(id);
	}
}
