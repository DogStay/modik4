// JobsModNpcMenu.c
//
// The hiring window: what this employer offers, and the one button that hands a
// finished job back to them.
//
// The menu holds no authority. Every row it draws — including whether a row can
// be clicked at all — was decided by the server and sent with the offer list.
// Clicking sends a request and nothing else; the window closes when the server
// answers with a new job state, not when the button is pressed. That way what
// the player sees after clicking is always what actually happened.

class JobsModNpcMenu extends UIScriptedMenu
{
	protected static const string LAYOUT_PATH = "JobsMod_Client/GUI/Layouts/JobsModNpcMenu.layout";

	// The layout carries this many rows. The server caps its offer list to the
	// same number, so a row is never dropped silently.
	protected static const int ROW_COUNT = 6;

	protected ref JobsModNpcOffer m_Offer;

	protected TextWidget m_NpcNameText;
	protected TextWidget m_NpcDescText;

	protected ref array<Widget> m_RowFrames;
	protected ref array<TextWidget> m_RowNames;
	protected ref array<TextWidget> m_RowDescs;
	protected ref array<TextWidget> m_RowStatuses;
	protected ref array<ButtonWidget> m_RowButtons;

	protected Widget m_HandInPanel;
	protected TextWidget m_HandInText;
	protected ButtonWidget m_HandInButton;

	protected Widget m_AbandonPanel;
	protected TextWidget m_AbandonText;
	protected ButtonWidget m_AbandonButton;

	protected TextWidget m_CloseText;
	protected ButtonWidget m_CloseButton;

	// Set once a request has gone out, so a second click cannot send a second
	// one while the first is still travelling.
	protected bool m_RequestSent;

	void JobsModNpcMenu()
	{
		m_RowFrames = new array<Widget>();
		m_RowNames = new array<TextWidget>();
		m_RowDescs = new array<TextWidget>();
		m_RowStatuses = new array<TextWidget>();
		m_RowButtons = new array<ButtonWidget>();
	}

	// The mission passes the offer in while the menu is being constructed,
	// because Init() and OnShow() both run before the caller sees the instance.
	void SetOffer(JobsModNpcOffer offer)
	{
		m_Offer = offer;
	}

	// =====================================================================
	// Widget binding
	// =====================================================================
	override Widget Init()
	{
		layoutRoot = GetGame().GetWorkspace().CreateWidgets(LAYOUT_PATH);
		if (!layoutRoot)
		{
			JobsLog.Error("CLIENT/UI: не удалось загрузить " + LAYOUT_PATH + ".");
			return null;
		}

		if (!BindWidgets())
			JobsLog.Error("CLIENT/UI: разметка меню найма неполная, меню не будет работать.");

		return layoutRoot;
	}

	// Every lookup is checked. A layout edit that renames or drops a widget then
	// fails here with the name in the log, instead of throwing a null pointer in
	// the middle of a click.
	protected bool BindWidgets()
	{
		bool ok = true;

		ok = BindText("NpcNameText", m_NpcNameText) && ok;
		ok = BindText("NpcDescText", m_NpcDescText) && ok;

		// Declared once rather than per iteration: Enforce scopes these to the
		// whole function either way, and a redeclaration inside the loop is a
		// compile error on some toolchains.
		string prefix;
		Widget frame;
		TextWidget name;
		TextWidget description;
		TextWidget status;
		ButtonWidget button;

		for (int i = 0; i < ROW_COUNT; i++)
		{
			prefix = "JobRow" + i.ToString();
			frame = layoutRoot.FindAnyWidget(prefix);

			ok = Report(frame, prefix) && ok;
			ok = BindText(prefix + "Name", name) && ok;
			ok = BindText(prefix + "Desc", description) && ok;
			ok = BindText(prefix + "Status", status) && ok;
			ok = BindButton(prefix + "Button", button) && ok;

			m_RowFrames.Insert(frame);
			m_RowNames.Insert(name);
			m_RowDescs.Insert(description);
			m_RowStatuses.Insert(status);
			m_RowButtons.Insert(button);
		}

		m_HandInPanel = layoutRoot.FindAnyWidget("HandInPanel");
		ok = Report(m_HandInPanel, "HandInPanel") && ok;
		ok = BindText("HandInText", m_HandInText) && ok;
		ok = BindButton("HandInButton", m_HandInButton) && ok;

		m_AbandonPanel = layoutRoot.FindAnyWidget("AbandonPanel");
		ok = Report(m_AbandonPanel, "AbandonPanel") && ok;
		ok = BindText("AbandonText", m_AbandonText) && ok;
		ok = BindButton("AbandonButton", m_AbandonButton) && ok;

		ok = BindText("CloseText", m_CloseText) && ok;
		ok = BindButton("CloseButton", m_CloseButton) && ok;

		return ok;
	}

	protected bool BindText(string name, out TextWidget widget)
	{
		widget = TextWidget.Cast(layoutRoot.FindAnyWidget(name));
		return Report(widget, name);
	}

	protected bool BindButton(string name, out ButtonWidget widget)
	{
		widget = ButtonWidget.Cast(layoutRoot.FindAnyWidget(name));
		return Report(widget, name);
	}

	protected bool Report(Widget widget, string name)
	{
		if (widget)
			return true;

		JobsLog.Error("CLIENT/UI: виджет '" + name + "' не найден в " + LAYOUT_PATH + ".");
		return false;
	}

	// =====================================================================
	// Show / hide
	// =====================================================================
	override void OnShow()
	{
		super.OnShow();

		GetGame().GetInput().ChangeGameFocus(1);
		GetGame().GetUIManager().ShowUICursor(true);

		m_RequestSent = false;
		Draw();
	}

	override void OnHide()
	{
		super.OnHide();

		GetGame().GetInput().ChangeGameFocus(-1);
		GetGame().GetUIManager().ShowUICursor(false);

		JobsModClientContext.OnNpcMenuClosed();
	}

	override bool UseMouse()
	{
		return true;
	}

	override bool UseKeyboard()
	{
		return true;
	}

	// =====================================================================
	// Drawing
	// =====================================================================
	protected void Draw()
	{
		if (!m_Offer || !m_NpcNameText)
		{
			JobsLog.Error("CLIENT/UI: меню найма открыто без данных, закрывается.");
			Close();
			return;
		}

		m_NpcNameText.SetText(m_Offer.m_NpcName);
		m_NpcDescText.SetText(m_Offer.m_NpcDescription);

		for (int i = 0; i < ROW_COUNT; i++)
			DrawRow(i);

		// Handing in is only ever possible at the employer that issued the job,
		// so the button is drawn from what the server said about this NPC — not
		// from the job the player happens to be holding.
		m_HandInPanel.Show(m_Offer.m_HandInAvailable);
		if (m_Offer.m_HandInAvailable)
			m_HandInText.SetText("СДАТЬ РАБОТУ: " + m_Offer.m_HeldJobName);

		bool holdsJob = m_Offer.m_HeldStatus != JobsModJobStatus.NONE;
		m_AbandonPanel.Show(holdsJob);
		if (holdsJob)
			m_AbandonText.SetText("ОТКАЗАТЬСЯ");

		m_CloseText.SetText("ЗАКРЫТЬ");
	}

	protected void DrawRow(int index)
	{
		Widget frame = m_RowFrames.Get(index);
		if (!frame)
			return;

		if (index >= m_Offer.m_Offers.Count())
		{
			frame.Show(false);
			return;
		}

		JobsModJobOffer offer = m_Offer.m_Offers.Get(index);

		frame.Show(true);
		m_RowNames.Get(index).SetText(offer.m_Name);
		m_RowDescs.Get(index).SetText(offer.m_Description + "   [" + offer.m_ZoneName + "]");
		m_RowStatuses.Get(index).SetText(offer.GetStatusText());

		// A row that cannot be taken is dimmed rather than hidden: the player
		// still needs to see the job exists and read why it is not available.
		if (offer.IsSelectable())
		{
			m_RowNames.Get(index).SetColor(JobsModPalette.TextPrimary());
			m_RowStatuses.Get(index).SetColor(JobsModPalette.Accent());
			frame.SetColor(JobsModPalette.BorderBase());
		}
		else
		{
			m_RowNames.Get(index).SetColor(JobsModPalette.TextMuted());
			m_RowStatuses.Get(index).SetColor(JobsModPalette.TextFaint());
			frame.SetColor(JobsModPalette.BorderDim());
		}
	}

	// =====================================================================
	// Input
	// =====================================================================
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (w == m_CloseButton)
		{
			Close();
			return true;
		}

		// Every path below sends something. Once one has, the menu is waiting on
		// the server and stops accepting input rather than queuing more of it.
		if (m_RequestSent || !m_Offer)
			return true;

		if (w == m_HandInButton && m_Offer.m_HandInAvailable)
		{
			m_RequestSent = true;
			JobsModClientContext.SendJobComplete(m_Offer.m_NpcId, m_Offer.m_AssignmentId);
			return true;
		}

		if (w == m_AbandonButton && m_Offer.m_HeldStatus != JobsModJobStatus.NONE)
		{
			m_RequestSent = true;
			JobsModClientContext.SendJobAbandon(m_Offer.m_AssignmentId);
			return true;
		}

		for (int i = 0; i < ROW_COUNT; i++)
		{
			if (w != m_RowButtons.Get(i))
				continue;

			if (i >= m_Offer.m_Offers.Count())
				return true;

			JobsModJobOffer offer = m_Offer.m_Offers.Get(i);
			if (!offer.IsSelectable())
				return true;

			m_RequestSent = true;
			JobsModClientContext.SendJobAccept(m_Offer.m_NpcId, offer.m_JobId);
			return true;
		}

		return super.OnClick(w, x, y, button);
	}

	override bool OnMouseEnter(Widget w, int x, int y)
	{
		if (!m_Offer)
			return false;

		for (int i = 0; i < ROW_COUNT; i++)
		{
			if (w != m_RowButtons.Get(i) || i >= m_Offer.m_Offers.Count())
				continue;

			if (m_Offer.m_Offers.Get(i).IsSelectable())
				m_RowFrames.Get(i).SetColor(JobsModPalette.BorderBright());

			return true;
		}

		return false;
	}

	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		if (!m_Offer)
			return false;

		for (int i = 0; i < ROW_COUNT; i++)
		{
			if (w != m_RowButtons.Get(i) || i >= m_Offer.m_Offers.Count())
				continue;

			if (m_Offer.m_Offers.Get(i).IsSelectable())
				m_RowFrames.Get(i).SetColor(JobsModPalette.BorderBase());

			return true;
		}

		return false;
	}
}
