// JobsModJobHud.c
//
// The job panel in the corner: what you took, where it is, and how far along.
//
// It renders one thing — the last job state the server sent — and computes
// nothing of its own. No distances, no timers counting themselves down, no
// guessing at progress between messages: if the number on screen is stale, the
// fix is a message from the server, not a smarter HUD.
//
// It is hidden whenever the player has no job and whenever a menu is open, so
// it never sits on top of the inventory or the map.

class JobsModJobHud
{
	protected static const string LAYOUT_PATH = "JobsMod_Client/GUI/Layouts/JobsModJobHud.layout";

	// How long the panel flashes after progress changes.
	protected static const float FLASH_DURATION = 0.45;

	protected Widget m_Root;
	protected Widget m_Accent;
	protected Widget m_BarFill;
	protected TextWidget m_Title;
	protected TextWidget m_Zone;
	protected TextWidget m_Progress;
	protected TextWidget m_Hint;

	protected int m_LastProgress;
	protected int m_LastStatus;
	protected int m_LastAssignmentId;
	protected float m_FlashRemaining;

	void JobsModJobHud()
	{
		m_Root = GetGame().GetWorkspace().CreateWidgets(LAYOUT_PATH);
		if (!m_Root)
		{
			JobsLog.Error("CLIENT/UI: не удалось загрузить " + LAYOUT_PATH + ".");
			return;
		}

		m_Accent = m_Root.FindAnyWidget("HudAccentBackground");
		m_BarFill = m_Root.FindAnyWidget("HudBarFillBackground");
		m_Title = TextWidget.Cast(m_Root.FindAnyWidget("HudTitle"));
		m_Zone = TextWidget.Cast(m_Root.FindAnyWidget("HudZone"));
		m_Progress = TextWidget.Cast(m_Root.FindAnyWidget("HudProgress"));
		m_Hint = TextWidget.Cast(m_Root.FindAnyWidget("HudHint"));

		if (!m_Accent || !m_BarFill || !m_Title || !m_Zone || !m_Progress || !m_Hint)
		{
			JobsLog.Error("CLIENT/UI: разметка HUD неполная, панель задания отключена.");
			m_Root = null;
			return;
		}

		m_Root.Show(false);
		m_LastStatus = JobsModJobStatus.NONE;
	}

	void ~JobsModJobHud()
	{
		if (m_Root)
			m_Root.Unlink();
	}

	void Update(float timeslice, bool visible)
	{
		if (!m_Root)
			return;

		JobsModJobView view = JobsModClientContext.GetJobView();

		if (!view.HasJob() || !visible)
		{
			m_Root.Show(false);
			return;
		}

		DetectChange(view);
		m_Root.Show(true);

		m_Title.SetText(view.m_JobName);
		m_Zone.SetText("Зона: " + view.m_ZoneName);
		m_Progress.SetText(view.m_Progress.ToString() + " / " + view.m_Required.ToString());
		m_Hint.SetText(view.m_Hint);

		m_BarFill.SetSize(view.GetFraction(), 1.0);

		ApplyColours(view, timeslice);
	}

	// A finished job is the one state the player has to act on, so it is the one
	// state that changes colour rather than just text.
	protected void ApplyColours(JobsModJobView view, float timeslice)
	{
		int accent = JobsModPalette.Accent();

		if (view.m_Status == JobsModJobStatus.READY_TO_HAND_IN)
			accent = JobsModPalette.BorderFilled();

		if (m_FlashRemaining > 0)
		{
			m_FlashRemaining = m_FlashRemaining - timeslice;
			accent = JobsModPalette.TextPrimary();
		}

		m_Accent.SetColor(accent);
		m_BarFill.SetColor(accent);
		m_Progress.SetColor(accent);
	}

	// A new assignment resets the flash rather than triggering one: starting a
	// job is announced by its own notification, and a flash there would only
	// compete with it.
	protected void DetectChange(JobsModJobView view)
	{
		if (view.m_AssignmentId != m_LastAssignmentId)
		{
			m_LastAssignmentId = view.m_AssignmentId;
			m_LastProgress = view.m_Progress;
			m_LastStatus = view.m_Status;
			m_FlashRemaining = 0;
			return;
		}

		if (view.m_Progress != m_LastProgress || view.m_Status != m_LastStatus)
			m_FlashRemaining = FLASH_DURATION;

		m_LastProgress = view.m_Progress;
		m_LastStatus = view.m_Status;
	}

	// The panel and the world marker both belong to the game world, not on top of
	// the inventory, the map or one of the mod's own menus. One rule, asked once
	// per frame, so the two can never disagree about it.
	static bool IsGameplayVisible()
	{
		UIManager manager = GetGame().GetUIManager();
		if (!manager)
			return false;

		// Any open menu hides the panel, not just the mod's own two. Listing the
		// ones to hide behind meant every menu nobody thought of — another mod's
		// window, the map, a trader — kept the HUD floating on top of it. Asking
		// whether anything at all is open needs no list and cannot fall behind.
		if (manager.GetMenu())
			return false;

		// The inventory is not a UIMenuPanel and so is not covered above.
		// IsInventoryOpen() is on MissionGameplay, not on Mission, and casting
		// for one boolean would tie the HUD to a mission class it never names.
		if (manager.IsMenuOpen(MENU_INVENTORY))
			return false;

		Mission mission = GetGame().GetMission();
		if (!mission)
			return false;

		return !mission.IsPaused();
	}
}
