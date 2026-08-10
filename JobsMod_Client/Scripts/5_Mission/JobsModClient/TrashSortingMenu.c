// TrashSortingMenu.c
//
// The sorting minigame, built to DESIGN_REFERENCE/TRASH_SORTING_FINAL:
// a 3x3 grid of found trash, three bins, drag and drop, per-bin feedback
// flashes, a transient toast, a confirm-on-abort dialog and a result window.
//
// The menu holds no authority. It renders the order the server granted and
// reports back what the player did; whether that counts as a finished job is
// decided server-side. Closing the menu for any reason (finish, abort, Escape,
// death) always tells the server, so a session can never be left dangling.

class TrashSortingMenu extends UIScriptedMenu
{
	protected static const string LAYOUT_PATH = "JobsMod_Client/GUI/Layouts/TrashSortingMenu.layout";

	// Timings taken from the design prototype.
	protected static const float FLASH_DURATION = 0.52;
	protected static const float SHAKE_DURATION = 0.38;
	protected static const float TOAST_DURATION = 1.60;
	protected static const float DONE_DELAY = 0.65;

	// The ghost is 150x120 design px and is carried centred on the cursor.
	protected static const int GHOST_OFFSET_X = 75;
	protected static const int GHOST_OFFSET_Y = 60;

	protected static const int ITEM_COUNT = 9;
	protected static const int PER_BIN = 3;

	// --- session ---
	protected ref JobsModSortingSession m_Session;

	// --- header ---
	protected TextWidget m_TitleText;
	protected TextWidget m_ZoneText;
	protected TextWidget m_ProgressText;
	protected ref array<Widget> m_ProgressPips;
	protected ButtonWidget m_CloseButton;
	protected TextWidget m_CloseButtonText;

	// --- static copy ---
	protected TextWidget m_InstructionText;
	protected TextWidget m_HintText;
	protected TextWidget m_WorkAreaTitle;
	protected TextWidget m_BinAreaTitle;

	// --- cards ---
	protected ref array<Widget> m_CardFrames;
	protected ref array<Widget> m_CardFills;
	protected ref array<Widget> m_CardPreviewHosts;
	protected ref array<TextWidget> m_CardFallbacks;
	protected ref array<TextWidget> m_CardNames;
	protected ref array<ButtonWidget> m_CardButtons;
	protected ref array<ItemPreviewWidget> m_CardPreviews;
	protected ref array<EntityAI> m_CardEntities;

	// --- bins, indexed the same way as JobsModTrashBin ---
	protected ref array<Widget> m_BinFrames;
	protected ref array<Widget> m_BinFills;
	protected ref array<TextWidget> m_BinLabels;
	protected ref array<TextWidget> m_BinCounters;
	protected ref array<TextWidget> m_BinStatuses;
	protected ref array<TextWidget> m_BinFallbacks;
	protected ref array<Widget> m_BinPreviewHosts;
	protected ref array<ItemPreviewWidget> m_BinPreviews;
	protected ref array<EntityAI> m_BinEntities;
	protected ref array<float> m_BinHomeX;
	protected ref array<float> m_BinHomeY;

	// --- footer ---
	protected TextWidget m_TaskText;
	protected TextWidget m_SortedText;
	protected TextWidget m_RemainingText;
	protected ButtonWidget m_CancelButton;
	protected TextWidget m_CancelButtonText;

	// --- overlays ---
	protected Widget m_ToastPanel;
	protected TextWidget m_ToastTitle;
	protected TextWidget m_ToastSub;

	protected Widget m_ResultLayer;
	protected TextWidget m_ResultTitle;
	protected TextWidget m_ResultBody;
	protected TextWidget m_ResultScore;
	protected ButtonWidget m_FinishButton;
	protected TextWidget m_FinishButtonText;

	protected Widget m_ConfirmLayer;
	protected TextWidget m_ConfirmTitle;
	protected TextWidget m_ConfirmBody;
	protected ButtonWidget m_ConfirmYes;
	protected TextWidget m_ConfirmYesText;
	protected ButtonWidget m_ConfirmNo;
	protected TextWidget m_ConfirmNoText;

	protected Widget m_DragGhost;
	protected Widget m_DragGhostPreviewHost;
	protected TextWidget m_DragGhostFallback;
	protected ItemPreviewWidget m_DragGhostPreview;
	protected EntityAI m_DragGhostEntity;

	// --- state ---
	protected ref array<bool> m_Sorted;
	protected ref array<int> m_Assignment;   // bin index each item ended up in
	protected ref array<int> m_BinCounts;
	protected int m_DragIndex;
	protected int m_HoverBin;
	protected int m_SortedCount;
	protected int m_Mistakes;
	protected bool m_Done;
	protected bool m_Confirming;
	protected bool m_Submitted;

	protected float m_FlashRemaining;
	protected int m_FlashBin;
	protected bool m_FlashCorrect;
	protected float m_ShakeRemaining;
	protected int m_ShakeBin;
	protected float m_ToastRemaining;
	protected float m_DoneRemaining;

	// =====================================================================
	// Construction
	// =====================================================================
	void TrashSortingMenu()
	{
		m_ProgressPips = new array<Widget>();
		m_CardFrames = new array<Widget>();
		m_CardFills = new array<Widget>();
		m_CardPreviewHosts = new array<Widget>();
		m_CardFallbacks = new array<TextWidget>();
		m_CardNames = new array<TextWidget>();
		m_CardButtons = new array<ButtonWidget>();
		m_CardPreviews = new array<ItemPreviewWidget>();
		m_CardEntities = new array<EntityAI>();

		m_BinFrames = new array<Widget>();
		m_BinFills = new array<Widget>();
		m_BinLabels = new array<TextWidget>();
		m_BinCounters = new array<TextWidget>();
		m_BinStatuses = new array<TextWidget>();
		m_BinFallbacks = new array<TextWidget>();
		m_BinPreviewHosts = new array<Widget>();
		m_BinPreviews = new array<ItemPreviewWidget>();
		m_BinEntities = new array<EntityAI>();
		m_BinHomeX = new array<float>();
		m_BinHomeY = new array<float>();

		m_Sorted = new array<bool>();
		m_Assignment = new array<int>();
		m_BinCounts = new array<int>();

		ResetState();
	}

	void ~TrashSortingMenu()
	{
		DestroyPreviews();
	}

	protected void ResetState()
	{
		m_Sorted.Clear();
		m_Assignment.Clear();

		for (int i = 0; i < ITEM_COUNT; i++)
		{
			m_Sorted.Insert(false);
			m_Assignment.Insert(-1);
		}

		m_BinCounts.Clear();
		for (int b = 0; b < JobsModTrashBin.COUNT; b++)
		{
			m_BinCounts.Insert(0);
		}

		m_DragIndex = -1;
		m_HoverBin = -1;
		m_SortedCount = 0;
		m_Mistakes = 0;
		m_Done = false;
		m_Confirming = false;
		m_Submitted = false;
		m_FlashRemaining = 0.0;
		m_FlashBin = -1;
		m_ShakeRemaining = 0.0;
		m_ShakeBin = -1;
		m_ToastRemaining = 0.0;
		m_DoneRemaining = 0.0;
	}

	// The context hands the granted session over before the menu is entered.
	void SetSession(JobsModSortingSession session)
	{
		m_Session = session;
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
		{
			JobsLog.Error("CLIENT/UI: разметка меню неполная, меню не будет работать.");
			return layoutRoot;
		}

		ApplyStaticText();
		return layoutRoot;
	}

	// Every lookup is checked. A layout edit that renames or drops a widget then
	// fails here with the name in the log, instead of throwing a null pointer
	// somewhere in the middle of a drag.
	protected bool BindWidgets()
	{
		bool ok = true;

		ok = ok && BindText("TitleText", m_TitleText);
		ok = ok && BindText("ZoneText", m_ZoneText);
		ok = ok && BindText("ProgressText", m_ProgressText);
		ok = ok && BindButton("CloseButton", m_CloseButton);
		ok = ok && BindText("CloseButtonText", m_CloseButtonText);

		ok = ok && BindText("InstructionText", m_InstructionText);
		ok = ok && BindText("HintText", m_HintText);
		ok = ok && BindText("WorkAreaTitle", m_WorkAreaTitle);
		ok = ok && BindText("BinAreaTitle", m_BinAreaTitle);

		ok = ok && BindText("TaskText", m_TaskText);
		ok = ok && BindText("SortedText", m_SortedText);
		ok = ok && BindText("RemainingText", m_RemainingText);
		ok = ok && BindButton("CancelButton", m_CancelButton);
		ok = ok && BindText("CancelButtonText", m_CancelButtonText);

		ok = ok && BindWidget("ToastPanel", m_ToastPanel);
		ok = ok && BindText("ToastTitle", m_ToastTitle);
		ok = ok && BindText("ToastSub", m_ToastSub);

		ok = ok && BindWidget("ResultLayer", m_ResultLayer);
		ok = ok && BindText("ResultTitle", m_ResultTitle);
		ok = ok && BindText("ResultBody", m_ResultBody);
		ok = ok && BindText("ResultScore", m_ResultScore);
		ok = ok && BindButton("FinishButton", m_FinishButton);
		ok = ok && BindText("FinishButtonText", m_FinishButtonText);

		ok = ok && BindWidget("ConfirmLayer", m_ConfirmLayer);
		ok = ok && BindText("ConfirmTitle", m_ConfirmTitle);
		ok = ok && BindText("ConfirmBody", m_ConfirmBody);
		ok = ok && BindButton("ConfirmYes", m_ConfirmYes);
		ok = ok && BindText("ConfirmYesText", m_ConfirmYesText);
		ok = ok && BindButton("ConfirmNo", m_ConfirmNo);
		ok = ok && BindText("ConfirmNoText", m_ConfirmNoText);

		ok = ok && BindWidget("DragGhost", m_DragGhost);
		ok = ok && BindWidget("DragGhostPreviewHost", m_DragGhostPreviewHost);
		ok = ok && BindText("DragGhostFallback", m_DragGhostFallback);

		int i;
		for (i = 0; i < ITEM_COUNT; i++)
		{
			Widget pip;
			ok = ok && BindWidget("ProgressPip" + i.ToString() + "Background", pip);
			m_ProgressPips.Insert(pip);

			Widget frame;
			Widget fill;
			Widget host;
			TextWidget fallback;
			TextWidget name;
			ButtonWidget button;

			ok = ok && BindWidget("TrashCard" + i.ToString() + "Background", frame);
			ok = ok && BindWidget("TrashCard" + i.ToString() + "BgBackground", fill);
			ok = ok && BindWidget("TrashPreviewHost" + i.ToString(), host);
			ok = ok && BindText("TrashFallback" + i.ToString(), fallback);
			ok = ok && BindText("TrashName" + i.ToString(), name);
			ok = ok && BindButton("TrashItem" + i.ToString(), button);

			m_CardFrames.Insert(frame);
			m_CardFills.Insert(fill);
			m_CardPreviewHosts.Insert(host);
			m_CardFallbacks.Insert(fallback);
			m_CardNames.Insert(name);
			m_CardButtons.Insert(button);
			m_CardPreviews.Insert(null);
			m_CardEntities.Insert(null);
		}

		for (i = 0; i < JobsModTrashBin.COUNT; i++)
		{
			string suffix = GetBinWidgetSuffix(i);

			Widget binFrame;
			Widget binFill;
			TextWidget binLabel;
			TextWidget binCounter;
			TextWidget binStatus;
			TextWidget binFallback;
			Widget binHost;

			ok = ok && BindWidget("Bin" + suffix + "Background", binFrame);
			ok = ok && BindWidget("Bin" + suffix + "BgBackground", binFill);
			ok = ok && BindText("Bin" + suffix + "Label", binLabel);
			ok = ok && BindText("Bin" + suffix + "Counter", binCounter);
			ok = ok && BindText("Bin" + suffix + "Status", binStatus);
			ok = ok && BindText("Bin" + suffix + "Fallback", binFallback);
			ok = ok && BindWidget("Bin" + suffix + "PreviewHost", binHost);

			m_BinFrames.Insert(binFrame);
			m_BinFills.Insert(binFill);
			m_BinLabels.Insert(binLabel);
			m_BinCounters.Insert(binCounter);
			m_BinStatuses.Insert(binStatus);
			m_BinFallbacks.Insert(binFallback);
			m_BinPreviewHosts.Insert(binHost);
			m_BinPreviews.Insert(null);
			m_BinEntities.Insert(null);
			m_BinHomeX.Insert(0.0);
			m_BinHomeY.Insert(0.0);
		}

		return ok;
	}

	protected bool BindWidget(string name, out Widget target)
	{
		target = layoutRoot.FindAnyWidget(name);
		if (!target)
		{
			JobsLog.Error("CLIENT/UI: виджет '" + name + "' не найден в разметке.");
			return false;
		}

		return true;
	}

	protected bool BindText(string name, out TextWidget target)
	{
		Widget found;
		if (!BindWidget(name, found))
			return false;

		target = TextWidget.Cast(found);
		if (!target)
		{
			JobsLog.Error("CLIENT/UI: виджет '" + name + "' не является TextWidget.");
			return false;
		}

		return true;
	}

	protected bool BindButton(string name, out ButtonWidget target)
	{
		Widget found;
		if (!BindWidget(name, found))
			return false;

		target = ButtonWidget.Cast(found);
		if (!target)
		{
			JobsLog.Error("CLIENT/UI: виджет '" + name + "' не является ButtonWidget.");
			return false;
		}

		return true;
	}

	protected string GetBinWidgetSuffix(int binIndex)
	{
		switch (binIndex)
		{
			case 0:
				return "Wood";
			case 1:
				return "Plastic";
			case 2:
				return "Metal";
		}

		return "";
	}

	// The bin models are vanilla barrels: the mod ships no art, and a barrel is
	// the closest thing the base game has to a sorting container.
	protected string GetBinPreviewClass(int binIndex)
	{
		switch (binIndex)
		{
			case 0:
				return "Barrel_Green";
			case 1:
				return "Barrel_Blue";
			case 2:
				return "Barrel_Red";
		}

		return "";
	}

	// =====================================================================
	// Static copy, straight from the design
	// =====================================================================
	protected void ApplyStaticText()
	{
		m_TitleText.SetText("СОРТИРОВКА МУСОРА");
		m_CloseButtonText.SetText("× ЗАКРЫТЬ");
		m_InstructionText.SetText("Распределите найденный мусор по соответствующим контейнерам.");
		m_HintText.SetText("Неправильно отсортированный предмет будет возвращён обратно.");
		m_WorkAreaTitle.SetText("НАЙДЕННЫЙ МУСОР");
		m_BinAreaTitle.SetText("КОНТЕЙНЕРЫ");
		m_TaskText.SetText("Задание: Уборка территории");
		m_CancelButtonText.SetText("ОТМЕНИТЬ СОРТИРОВКУ");

		m_ResultTitle.SetText("СОРТИРОВКА ЗАВЕРШЕНА");
		m_ResultBody.SetText("Все предметы распределены правильно. Рабочая зона очищена.");
		m_ResultScore.SetText("9 / 9");
		m_FinishButtonText.SetText("ЗАВЕРШИТЬ РАБОТУ");

		m_ConfirmTitle.SetText("ПРЕРВАТЬ СОРТИРОВКУ?");
		m_ConfirmYesText.SetText("ПРЕРВАТЬ");
		m_ConfirmNoText.SetText("ПРОДОЛЖИТЬ");

		for (int i = 0; i < JobsModTrashBin.COUNT; i++)
		{
			m_BinLabels.Get(i).SetText(JobsModTrashBin.GetLabel(JobsModTrashBin.GetIdByIndex(i)));
		}
	}

	// =====================================================================
	// Show / hide
	// =====================================================================
	override void OnShow()
	{
		super.OnShow();

		GetGame().GetInput().ChangeGameFocus(1);
		GetGame().GetUIManager().ShowUICursor(true);

		ResetState();
		BuildPreviews();
		RefreshAll();

		m_ToastPanel.Show(false);
		m_ResultLayer.Show(false);
		m_ConfirmLayer.Show(false);
		m_DragGhost.Show(false);

		if (m_Session)
			JobsLog.Debug("CLIENT/UI: меню сортировки открыто; nonce=" + m_Session.GetNonce().ToString() + ".");
	}

	override void OnHide()
	{
		super.OnHide();

		GetGame().GetInput().ChangeGameFocus(-1);
		GetGame().GetUIManager().ShowUICursor(false);

		// Whatever closed the menu — the finish button, the abort dialog, Escape,
		// or the player dying — the server must not be left holding an open
		// session. Submitting already told it; anything else is an abort.
		if (!m_Submitted && m_Session)
			JobsModClientContext.SendSortingAbort(m_Session.GetNonce());

		DestroyPreviews();
		JobsModClientContext.OnSortingMenuClosed();
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
	// 3D previews
	//
	// The mod carries no textures, so each card and bin shows a real game model
	// rendered by an ItemPreviewWidget. Any class that fails to spawn falls back
	// to a text label, so a wrong class name costs a picture, not the minigame.
	// =====================================================================
	protected void BuildPreviews()
	{
		if (!m_Session)
			return;

		int i;
		for (i = 0; i < ITEM_COUNT && i < m_Session.GetItemCount(); i++)
		{
			JobsModTrashItem definition = JobsModTrashCatalog.Find(m_Session.GetItemId(i));
			if (!definition)
				continue;

			m_CardNames.Get(i).SetText(definition.GetDisplayName());

			EntityAI entity = CreatePreviewEntity(definition.GetPreviewClassName());
			ItemPreviewWidget preview = null;

			if (entity)
				preview = CreatePreviewWidget(m_CardPreviewHosts.Get(i), entity, 20);

			if (preview)
			{
				m_CardEntities.Set(i, entity);
				m_CardPreviews.Set(i, preview);
				m_CardFallbacks.Get(i).Show(false);
			}
			else
			{
				if (entity)
					entity.Delete();

				m_CardFallbacks.Get(i).SetText(definition.GetDisplayName());
				m_CardFallbacks.Get(i).Show(true);
				JobsLog.Warning("CLIENT/UI: превью '" + definition.GetPreviewClassName() + "' недоступно, показан текст.");
			}
		}

		for (i = 0; i < JobsModTrashBin.COUNT; i++)
		{
			EntityAI binEntity = CreatePreviewEntity(GetBinPreviewClass(i));
			ItemPreviewWidget binPreview = null;

			if (binEntity)
				binPreview = CreatePreviewWidget(m_BinPreviewHosts.Get(i), binEntity, 20);

			if (binPreview)
			{
				m_BinEntities.Set(i, binEntity);
				m_BinPreviews.Set(i, binPreview);
				m_BinFallbacks.Get(i).Show(false);
			}
			else
			{
				if (binEntity)
					binEntity.Delete();

				m_BinFallbacks.Get(i).SetText(JobsModTrashBin.GetLabel(JobsModTrashBin.GetIdByIndex(i)));
				m_BinFallbacks.Get(i).Show(true);
			}
		}
	}

	protected EntityAI CreatePreviewEntity(string className)
	{
		if (className == "")
			return null;

		// Guard first: CreateObjectEx on an unknown class is not something the
		// menu should discover by crashing mid-open.
		if (!GetGame().ConfigIsExisting("CfgVehicles " + className))
		{
			JobsLog.Warning("CLIENT/UI: класс '" + className + "' отсутствует в CfgVehicles.");
			return null;
		}

		Object created = GetGame().CreateObjectEx(className, vector.Zero, ECE_LOCAL | ECE_NOLIFETIME);
		EntityAI entity = EntityAI.Cast(created);

		if (!entity)
		{
			if (created)
				created.Delete();

			return null;
		}

		return entity;
	}

	protected ItemPreviewWidget CreatePreviewWidget(Widget host, EntityAI entity, int sort)
	{
		if (!host || !entity)
			return null;

		ItemPreviewWidget preview = ItemPreviewWidget.Cast(
			GetGame().GetWorkspace().CreateWidget(
				ItemPreviewWidgetTypeID,
				0,
				0,
				1,
				1,
				WidgetFlags.VISIBLE | WidgetFlags.IGNOREPOINTER,
				ARGB(255, 255, 255, 255),
				sort,
				host));

		if (!preview)
			return null;

		preview.SetItem(entity);
		preview.SetView(entity.GetViewIndex());
		preview.SetModelPosition(Vector(0, 0, 0.5));
		preview.SetModelOrientation(Vector(0, 0, 0));
		preview.Show(true);
		return preview;
	}

	protected void DestroyPreviews()
	{
		int i;

		if (m_CardPreviews)
		{
			for (i = 0; i < m_CardPreviews.Count(); i++)
			{
				ReleasePreview(m_CardPreviews.Get(i));
				m_CardPreviews.Set(i, null);
			}
		}

		if (m_CardEntities)
		{
			for (i = 0; i < m_CardEntities.Count(); i++)
			{
				if (m_CardEntities.Get(i))
					m_CardEntities.Get(i).Delete();

				m_CardEntities.Set(i, null);
			}
		}

		if (m_BinPreviews)
		{
			for (i = 0; i < m_BinPreviews.Count(); i++)
			{
				ReleasePreview(m_BinPreviews.Get(i));
				m_BinPreviews.Set(i, null);
			}
		}

		if (m_BinEntities)
		{
			for (i = 0; i < m_BinEntities.Count(); i++)
			{
				if (m_BinEntities.Get(i))
					m_BinEntities.Get(i).Delete();

				m_BinEntities.Set(i, null);
			}
		}

		ReleaseDragGhostPreview();
	}

	protected void ReleasePreview(ItemPreviewWidget preview)
	{
		if (!preview)
			return;

		preview.SetItem(null);
		preview.Unlink();
	}

	protected void ReleaseDragGhostPreview()
	{
		if (m_DragGhostPreview)
		{
			ReleasePreview(m_DragGhostPreview);
			m_DragGhostPreview = null;
		}

		if (m_DragGhostEntity)
		{
			m_DragGhostEntity.Delete();
			m_DragGhostEntity = null;
		}
	}

	// =====================================================================
	// Rendering
	// =====================================================================
	protected void RefreshAll()
	{
		RefreshHeader();
		RefreshCards();
		RefreshBins();
		RefreshFooter();
	}

	protected void RefreshHeader()
	{
		string zone = "";
		if (m_Session)
			zone = m_Session.GetZoneName();

		if (zone == "")
			zone = "не указана";

		m_ZoneText.SetText("Рабочая зона: " + zone);
		m_ProgressText.SetText("ПРОГРЕСС: " + m_SortedCount.ToString() + " / " + ITEM_COUNT.ToString());

		for (int i = 0; i < m_ProgressPips.Count(); i++)
		{
			if (i < m_SortedCount)
				m_ProgressPips.Get(i).SetColor(JobsModPalette.Accent());
			else
				m_ProgressPips.Get(i).SetColor(JobsModPalette.BorderDim());
		}
	}

	protected void RefreshCards()
	{
		for (int i = 0; i < m_CardFrames.Count(); i++)
		{
			Widget frame = m_CardFrames.Get(i);

			if (m_Sorted.Get(i))
			{
				frame.SetColor(JobsModPalette.BorderDim());
				frame.SetAlpha(JobsModPalette.ALPHA_DISABLED);
			}
			else if (i == m_DragIndex)
			{
				frame.SetColor(JobsModPalette.BorderBase());
				frame.SetAlpha(JobsModPalette.ALPHA_DRAGGING);
			}
			else
			{
				frame.SetColor(JobsModPalette.BorderBase());
				frame.SetAlpha(1.0);
			}

			m_CardFills.Get(i).SetColor(JobsModPalette.Card());
		}
	}

	protected void RefreshBins()
	{
		for (int i = 0; i < m_BinFrames.Count(); i++)
		{
			int count = m_BinCounts.Get(i);
			bool full = count >= PER_BIN;
			bool hovered = (m_HoverBin == i && m_DragIndex >= 0);
			bool flashing = (m_FlashBin == i && m_FlashRemaining > 0.0);

			int border = JobsModPalette.BorderBase();
			int fill = JobsModPalette.Bin();
			string status = "ОЖИДАНИЕ";
			int statusColor = JobsModPalette.TextMuted();

			if (full)
			{
				border = JobsModPalette.BorderFilled();
				status = "ЗАПОЛНЕНО";
				statusColor = JobsModPalette.Accent();
			}

			if (hovered)
			{
				border = JobsModPalette.Accent();
				fill = JobsModPalette.AccentSoft();
				status = "ОТПУСТИТЕ ПРЕДМЕТ";
				statusColor = JobsModPalette.Accent();
			}

			// A flash outranks every other state for its half second: it is the
			// only feedback telling the player whether the drop was right.
			if (flashing)
			{
				if (m_FlashCorrect)
				{
					border = JobsModPalette.Accent();
					fill = JobsModPalette.AccentFlash();
					statusColor = JobsModPalette.Accent();
				}
				else
				{
					border = JobsModPalette.Danger();
					fill = JobsModPalette.DangerFlash();
					statusColor = JobsModPalette.Danger();
				}
			}

			m_BinFrames.Get(i).SetColor(border);
			m_BinFills.Get(i).SetColor(fill);

			m_BinCounters.Get(i).SetText(count.ToString() + " / " + PER_BIN.ToString());
			if (count > 0)
				m_BinCounters.Get(i).SetColor(JobsModPalette.TextPrimary());
			else
				m_BinCounters.Get(i).SetColor(JobsModPalette.TextMuted());

			m_BinStatuses.Get(i).SetText(status);
			m_BinStatuses.Get(i).SetColor(statusColor);
		}
	}

	protected void RefreshFooter()
	{
		int remaining = ITEM_COUNT - m_SortedCount;
		m_SortedText.SetText("Отсортировано: " + m_SortedCount.ToString() + " из " + ITEM_COUNT.ToString());
		m_RemainingText.SetText("ОСТАЛОСЬ: " + remaining.ToString());
	}

	// =====================================================================
	// Input
	// =====================================================================
	override bool OnMouseButtonDown(Widget w, int x, int y, int button)
	{
		if (button != MouseState.LEFT)
			return super.OnMouseButtonDown(w, x, y, button);

		if (!IsInteractive())
			return super.OnMouseButtonDown(w, x, y, button);

		int index = FindCardIndex(w);
		if (index < 0 || m_Sorted.Get(index))
			return super.OnMouseButtonDown(w, x, y, button);

		BeginDrag(index);
		return true;
	}

	override bool OnMouseButtonUp(Widget w, int x, int y, int button)
	{
		if (m_DragIndex < 0)
			return super.OnMouseButtonUp(w, x, y, button);

		EndDrag();
		return true;
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (w == m_FinishButton)
		{
			SubmitResult();
			return true;
		}

		if (w == m_ConfirmYes)
		{
			m_Confirming = false;
			m_ConfirmLayer.Show(false);
			Close();
			return true;
		}

		if (w == m_ConfirmNo)
		{
			m_Confirming = false;
			m_ConfirmLayer.Show(false);
			return true;
		}

		if (w == m_CloseButton || w == m_CancelButton)
		{
			RequestClose();
			return true;
		}

		return super.OnClick(w, x, y, button);
	}

	// True only when the player is allowed to move cards: not while a modal is
	// up, not after the round is finished, not while waiting on the server.
	protected bool IsInteractive()
	{
		return !m_Done && !m_Confirming && !m_Submitted;
	}

	protected int FindCardIndex(Widget w)
	{
		if (!w)
			return -1;

		for (int i = 0; i < m_CardButtons.Count(); i++)
		{
			if (m_CardButtons.Get(i) == w)
				return i;
		}

		return -1;
	}

	// =====================================================================
	// Drag and drop
	// =====================================================================
	protected void BeginDrag(int index)
	{
		m_DragIndex = index;
		m_HoverBin = -1;

		ReleaseDragGhostPreview();

		JobsModTrashItem definition = JobsModTrashCatalog.Find(m_Session.GetItemId(index));
		if (definition)
		{
			m_DragGhostEntity = CreatePreviewEntity(definition.GetPreviewClassName());
			if (m_DragGhostEntity)
				m_DragGhostPreview = CreatePreviewWidget(m_DragGhostPreviewHost, m_DragGhostEntity, 60);

			if (m_DragGhostPreview)
			{
				m_DragGhostFallback.Show(false);
			}
			else
			{
				if (m_DragGhostEntity)
				{
					m_DragGhostEntity.Delete();
					m_DragGhostEntity = null;
				}

				m_DragGhostFallback.SetText(definition.GetDisplayName());
				m_DragGhostFallback.Show(true);
			}
		}

		MoveGhostToCursor();
		m_DragGhost.Show(true);
		RefreshCards();
		RefreshBins();
	}

	protected void EndDrag()
	{
		int index = m_DragIndex;
		int targetBin = m_HoverBin;

		m_DragIndex = -1;
		m_HoverBin = -1;
		m_DragGhost.Show(false);
		ReleaseDragGhostPreview();

		// Released over empty space: the design calls this a cancel, not a
		// mistake, so nothing is counted and no feedback flashes.
		if (targetBin < 0)
		{
			RefreshCards();
			RefreshBins();
			return;
		}

		ApplyDrop(index, targetBin);
	}

	protected void ApplyDrop(int index, int binIndex)
	{
		string itemId = m_Session.GetItemId(index);
		string correctBin = JobsModTrashCatalog.GetBinIdForItem(itemId);
		string droppedBin = JobsModTrashBin.GetIdByIndex(binIndex);

		if (correctBin == droppedBin)
		{
			m_Sorted.Set(index, true);
			m_Assignment.Set(index, binIndex);
			m_BinCounts.Set(binIndex, m_BinCounts.Get(binIndex) + 1);
			m_SortedCount++;

			StartFlash(binIndex, true);

			JobsModTrashItem definition = JobsModTrashCatalog.Find(itemId);
			string subject = "";
			if (definition)
				subject = definition.GetDisplayName();

			ShowToast("ПРЕДМЕТ ОТСОРТИРОВАН", subject, JobsModPalette.Accent());

			if (m_SortedCount >= ITEM_COUNT)
				m_DoneRemaining = DONE_DELAY;
		}
		else
		{
			// Wrong bin: the item goes back and the progress is untouched. The
			// design is explicit that the correct bin must not be revealed.
			m_Mistakes++;
			StartFlash(binIndex, false);
			m_ShakeBin = binIndex;
			m_ShakeRemaining = SHAKE_DURATION;
			ShowToast("НЕВЕРНЫЙ КОНТЕЙНЕР", "Попробуйте определить материал предмета ещё раз.", JobsModPalette.Danger());
		}

		RefreshAll();
	}

	protected void StartFlash(int binIndex, bool correct)
	{
		m_FlashBin = binIndex;
		m_FlashCorrect = correct;
		m_FlashRemaining = FLASH_DURATION;
	}

	protected void ShowToast(string title, string sub, int accent)
	{
		m_ToastTitle.SetText(title);
		m_ToastTitle.SetColor(accent);
		m_ToastSub.SetText(sub);
		m_ToastPanel.Show(true);
		m_ToastRemaining = TOAST_DURATION;
	}

	protected void MoveGhostToCursor()
	{
		int mouseX;
		int mouseY;
		GetMousePos(mouseX, mouseY);
		m_DragGhost.SetScreenPos(mouseX - GHOST_OFFSET_X, mouseY - GHOST_OFFSET_Y);
	}

	// Which bin the cursor is over, or -1. Screen rectangles are read fresh each
	// frame so the answer stays right if the window is moved or rescaled.
	protected int FindBinUnderCursor()
	{
		int mouseX;
		int mouseY;
		GetMousePos(mouseX, mouseY);

		for (int i = 0; i < m_BinFrames.Count(); i++)
		{
			float bx;
			float by;
			float bw;
			float bh;
			m_BinFrames.Get(i).GetScreenPos(bx, by);
			m_BinFrames.Get(i).GetScreenSize(bw, bh);

			if (mouseX >= bx && mouseX <= bx + bw && mouseY >= by && mouseY <= by + bh)
				return i;
		}

		return -1;
	}

	// =====================================================================
	// Closing and submitting
	// =====================================================================
	protected void RequestClose()
	{
		// Nothing sorted yet, or the round is already over: no progress can be
		// lost, so do not make the player confirm anything.
		if (m_Done || m_SortedCount == 0)
		{
			Close();
			return;
		}

		m_Confirming = true;
		m_ConfirmBody.SetText("Отсортировано " + m_SortedCount.ToString() + " из " + ITEM_COUNT.ToString() + ". Прогресс не будет сохранён.");
		m_ConfirmLayer.Show(true);
	}

	protected void SubmitResult()
	{
		if (m_Submitted || !m_Session)
			return;

		m_Submitted = true;
		m_FinishButton.Enable(false);

		array<string> sequence = new array<string>();
		for (int i = 0; i < ITEM_COUNT; i++)
		{
			sequence.Insert(JobsModTrashBin.GetIdByIndex(m_Assignment.Get(i)));
		}

		JobsModClientContext.SendSortingSubmit(m_Session.GetNonce(), sequence, m_Mistakes);
		JobsLog.Info("CLIENT/JANITOR: результат отправлен; ошибок за смену: " + m_Mistakes.ToString() + ".");

		Close();
	}

	protected void ShowResult()
	{
		m_Done = true;
		m_DragIndex = -1;
		m_HoverBin = -1;
		m_ToastPanel.Show(false);
		m_ToastRemaining = 0.0;
		m_ResultLayer.Show(true);
		RefreshAll();
	}

	// =====================================================================
	// Frame update: cursor tracking and every timed visual state
	// =====================================================================
	override void Update(float timeslice)
	{
		super.Update(timeslice);

		if (m_DragIndex >= 0)
		{
			MoveGhostToCursor();

			int bin = FindBinUnderCursor();
			if (bin != m_HoverBin)
			{
				m_HoverBin = bin;
				RefreshBins();
			}
		}

		if (m_FlashRemaining > 0.0)
		{
			m_FlashRemaining = m_FlashRemaining - timeslice;
			if (m_FlashRemaining <= 0.0)
			{
				m_FlashRemaining = 0.0;
				m_FlashBin = -1;
				RefreshBins();
			}
		}

		if (m_ShakeRemaining > 0.0)
		{
			m_ShakeRemaining = m_ShakeRemaining - timeslice;
			ApplyShake();
		}

		if (m_ToastRemaining > 0.0)
		{
			m_ToastRemaining = m_ToastRemaining - timeslice;
			if (m_ToastRemaining <= 0.0)
			{
				m_ToastRemaining = 0.0;
				m_ToastPanel.Show(false);
			}
		}

		if (m_DoneRemaining > 0.0)
		{
			m_DoneRemaining = m_DoneRemaining - timeslice;
			if (m_DoneRemaining <= 0.0)
			{
				m_DoneRemaining = 0.0;
				ShowResult();
			}
		}
	}

	// Horizontal nudge on a wrong drop. The rest position is captured the first
	// time a bin shakes, so repeated shakes cannot make it drift off its anchor.
	protected void ApplyShake()
	{
		if (m_ShakeBin < 0 || m_ShakeBin >= m_BinFrames.Count())
			return;

		Widget frame = m_BinFrames.Get(m_ShakeBin);

		if (m_BinHomeX.Get(m_ShakeBin) == 0.0 && m_BinHomeY.Get(m_ShakeBin) == 0.0)
		{
			float homeX;
			float homeY;
			frame.GetScreenPos(homeX, homeY);
			m_BinHomeX.Set(m_ShakeBin, homeX);
			m_BinHomeY.Set(m_ShakeBin, homeY);
		}

		float offset = 0.0;
		if (m_ShakeRemaining > 0.32)
			offset = -8.0;
		else if (m_ShakeRemaining > 0.26)
			offset = 8.0;
		else if (m_ShakeRemaining > 0.20)
			offset = -6.0;
		else if (m_ShakeRemaining > 0.14)
			offset = 6.0;
		else if (m_ShakeRemaining > 0.08)
			offset = -3.0;
		else if (m_ShakeRemaining > 0.02)
			offset = 3.0;

		frame.SetScreenPos(m_BinHomeX.Get(m_ShakeBin) + offset, m_BinHomeY.Get(m_ShakeBin));

		if (m_ShakeRemaining <= 0.0)
		{
			m_ShakeRemaining = 0.0;
			frame.SetScreenPos(m_BinHomeX.Get(m_ShakeBin), m_BinHomeY.Get(m_ShakeBin));
			m_ShakeBin = -1;
		}
	}
}
