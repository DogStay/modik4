// MissionGameplayJobsMod.c
//
// Registers the mod's menus with the mission's menu factory, owns the job HUD,
// and is the only place that actually opens a menu.
//
// Opening happens here rather than in JobsModClientContext because the menu
// classes are compiled with 5_Mission and the context with 4_World, which is
// earlier: the context can say "open this id" but cannot name the class behind
// it. The invokers it exposes are that seam.
//
// The session and the offer are handed to their menus during construction
// rather than after EnterScriptedMenu returns: the manager runs Init() and
// OnShow() before the caller ever sees the instance, and OnShow is where each
// menu builds itself from its data.

modded class MissionGameplay
{
	protected ref JobsModJobHud m_JobsModHud;
	protected ref JobsModWorldMarker m_JobsModMarker;

	override void OnInit()
	{
		super.OnInit();

		JobsModClientContext.GetOnOpenMenu().Insert(JobsModOpenMenu);
		JobsModClientContext.GetOnCloseMenu().Insert(JobsModCloseMenu);

		if (!m_JobsModHud)
			m_JobsModHud = new JobsModJobHud();

		if (!m_JobsModMarker)
			m_JobsModMarker = new JobsModWorldMarker();
	}

	override void OnMissionFinish()
	{
		JobsModClientContext.GetOnOpenMenu().Remove(JobsModOpenMenu);
		JobsModClientContext.GetOnCloseMenu().Remove(JobsModCloseMenu);

		m_JobsModHud = null;
		m_JobsModMarker = null;

		super.OnMissionFinish();
	}

	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice);

		// Asked once and handed to both, so the panel and the world marker can
		// never end up showing at different moments.
		bool visible = JobsModJobHud.IsGameplayVisible();

		if (m_JobsModHud)
			m_JobsModHud.Update(timeslice, visible);

		if (m_JobsModMarker)
			m_JobsModMarker.Update(visible);
	}

	override UIScriptedMenu CreateScriptedMenu(int id)
	{
		if (id == JobsModMenuIds.TRASH_SORTING)
		{
			TrashSortingMenu sorting = new TrashSortingMenu();
			sorting.SetSession(JobsModClientContext.ConsumePendingSession());
			return sorting;
		}

		if (id == JobsModMenuIds.NPC_JOBS)
		{
			JobsModNpcMenu hiring = new JobsModNpcMenu();
			hiring.SetOffer(JobsModClientContext.ConsumePendingOffer());
			return hiring;
		}

		return super.CreateScriptedMenu(id);
	}

	// Raised by the client context when the server has sent something worth
	// showing. A failure to create is reported back so the context does not stay
	// convinced the menu is open.
	void JobsModOpenMenu(int menuId)
	{
		UIManager manager = GetGame().GetUIManager();
		if (!manager)
		{
			JobsLog.Error("CLIENT/UI: UIManager недоступен.");
			JobsModClientContext.OnMenuOpenFailed(menuId);
			return;
		}

		if (manager.IsMenuOpen(menuId))
			return;

		if (!manager.EnterScriptedMenu(menuId, null))
		{
			JobsLog.Error("CLIENT/UI: меню " + menuId.ToString() + " не создано.");
			JobsModClientContext.OnMenuOpenFailed(menuId);
		}
	}

	void JobsModCloseMenu(int menuId)
	{
		UIManager manager = GetGame().GetUIManager();
		if (!manager)
			return;

		UIScriptedMenu menu = manager.FindMenu(menuId);
		if (menu)
			menu.Close();
	}
}
