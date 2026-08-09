// JobsModClientContext.c
//
// The one place the client keeps sorting state and the one place it talks to
// the server from. The menu never sends RPCs itself: it lives only while it is
// open, and a submission must survive it closing.
//
// Everything here is static because there is exactly one local player. Holding
// an instance would only add a lifetime to get wrong.

class JobsModClientContext
{
	protected static ref JobsModSortingSession s_PendingSession;
	protected static bool s_MenuOpen;

	// The job this player is holding, as last reported by the server. The HUD
	// reads it every frame; nothing else writes it.
	protected static ref JobsModJobView s_JobView;

	// The offer list of the NPC currently being talked to. Lives only between
	// the server's answer and the menu closing.
	protected static ref JobsModNpcOffer s_PendingOffer;
	protected static bool s_NpcMenuOpen;

	// =====================================================================
	// Incoming: session granted by the server
	// =====================================================================
	static void HandleSessionGranted(ParamsReadContext ctx)
	{
		Param3<int, string, string> data = new Param3<int, string, string>(0, "", "");
		if (!ctx.Read(data))
		{
			JobsLog.Error("CLIENT/RPC: не удалось прочитать NOTIFY_SORTING_SESSION.");
			return;
		}

		array<string> order;
		if (!JobsModTrashCatalog.UnpackOrder(data.param3, order))
		{
			JobsLog.Error("CLIENT/RPC: сервер прислал некорректный порядок предметов.");
			return;
		}

		s_PendingSession = new JobsModSortingSession(data.param1, data.param2, order);
		JobsLog.Info("CLIENT/JANITOR: смена выдана; зона '" + data.param2 + "', nonce=" + data.param1.ToString() + ".");

		OpenSortingMenu();
	}

	static void HandleRejected(ParamsReadContext ctx)
	{
		Param1<int> data = new Param1<int>(0);
		if (!ctx.Read(data))
		{
			JobsLog.Error("CLIENT/RPC: не удалось прочитать NOTIFY_REJECTED.");
			return;
		}

		string text = JobsModRejectReason.GetText(data.param1);
		JobsLog.Warning("CLIENT/JANITOR: запрос отклонён; причина=" + data.param1.ToString() + ".");
		NotificationSystem.AddNotificationExtended(6.0, "РАБОТА НЕДОСТУПНА", text);
	}

	static void HandleAccepted(ParamsReadContext ctx)
	{
		Param1<string> data = new Param1<string>("");
		if (!ctx.Read(data))
		{
			JobsLog.Error("CLIENT/RPC: не удалось прочитать NOTIFY_SORTING_ACCEPTED.");
			return;
		}

		s_PendingSession = null;
		JobsLog.Info("CLIENT/JANITOR: сервер принял кучу.");
		NotificationSystem.AddNotificationExtended(6.0, "КУЧА РАЗОБРАНА", data.param1);
	}

	// =====================================================================
	// Incoming: NPC directory
	//
	// Read strictly in the order the server wrote it: a count, then that many
	// entries. A short read leaves the directory as it was rather than half
	// filled, so the talk action never starts pointing at nothing.
	// =====================================================================
	static void HandleNpcDirectory(ParamsReadContext ctx)
	{
		Param1<int> header = new Param1<int>(0);
		if (!ctx.Read(header))
		{
			JobsLog.Error("CLIENT/RPC: не удалось прочитать NOTIFY_NPC_DIRECTORY.");
			return;
		}

		JobsModNpcDirectory.Clear();

		for (int i = 0; i < header.param1; i++)
		{
			Param2<string, vector> entry = new Param2<string, vector>("", vector.Zero);
			if (!ctx.Read(entry))
			{
				JobsLog.Error("CLIENT/RPC: справочник NPC оборван на записи " + i.ToString() + ".");
				return;
			}

			JobsModNpcDirectory.Add(entry.param1, entry.param2);
		}

		// Info rather than Debug: debug output is switched on from the server
		// config, which the client never reads, so a Debug line here is dropped
		// on every client and the directory — the thing that decides whether the
		// talk action is offered at all — becomes impossible to diagnose. It
		// costs one line per connect.
		JobsLog.Info("CLIENT/JOBS: получено NPC: " + JobsModNpcDirectory.GetCount().ToString() + ".");
	}

	// =====================================================================
	// Incoming: the NPC's offer list
	// =====================================================================
	static void HandleJobMenu(ParamsReadContext ctx)
	{
		Param4<string, string, string, int> header = new Param4<string, string, string, int>("", "", "", 0);
		if (!ctx.Read(header))
		{
			JobsLog.Error("CLIENT/RPC: не удалось прочитать заголовок NOTIFY_JOB_MENU.");
			return;
		}

		Param4<int, int, string, int> held = new Param4<int, int, string, int>(0, 0, "", 0);
		if (!ctx.Read(held))
		{
			JobsLog.Error("CLIENT/RPC: не удалось прочитать состояние работы в NOTIFY_JOB_MENU.");
			return;
		}

		JobsModNpcOffer offer = new JobsModNpcOffer();
		offer.m_NpcId = header.param1;
		offer.m_NpcName = header.param2;
		offer.m_NpcDescription = header.param3;
		offer.m_HandInAvailable = held.param1 != 0;
		offer.m_AssignmentId = held.param2;
		offer.m_HeldJobName = held.param3;
		offer.m_HeldStatus = held.param4;

		for (int i = 0; i < header.param4; i++)
		{
			Param4<string, string, string, string> text = new Param4<string, string, string, string>("", "", "", "");
			Param4<int, int, int, int> numbers = new Param4<int, int, int, int>(0, 0, 0, 0);

			if (!ctx.Read(text) || !ctx.Read(numbers))
			{
				JobsLog.Error("CLIENT/RPC: список работ оборван на записи " + i.ToString() + ".");
				return;
			}

			offer.m_Offers.Insert(new JobsModJobOffer(
				text.param1, text.param2, text.param3, text.param4,
				numbers.param1, numbers.param2, numbers.param3, numbers.param4));
		}

		s_PendingOffer = offer;
		OpenNpcMenu();
	}

	// =====================================================================
	// Incoming: the job this player holds
	// =====================================================================
	static void HandleJobState(ParamsReadContext ctx)
	{
		Param4<int, int, int, int> numbers = new Param4<int, int, int, int>(0, 0, 0, 0);
		Param4<string, string, string, string> text = new Param4<string, string, string, string>("", "", "", "");
		Param3<string, int, int> extra = new Param3<string, int, int>("", 0, 0);

		if (!ctx.Read(numbers) || !ctx.Read(text) || !ctx.Read(extra))
		{
			JobsLog.Error("CLIENT/RPC: не удалось прочитать NOTIFY_JOB_STATE.");
			return;
		}

		JobsModJobView view = new JobsModJobView();
		view.m_Status = numbers.param1;
		view.m_Progress = numbers.param2;
		view.m_Required = numbers.param3;
		view.m_AssignmentId = numbers.param4;
		view.m_JobName = text.param1;
		view.m_ZoneName = text.param2;
		view.m_NpcName = text.param3;
		view.m_Hint = text.param4;
		view.m_CargoClass = extra.param1;
		view.m_Type = extra.param2;

		// A truncated marker list leaves the previous state untouched rather
		// than installing a job with nowhere to go.
		for (int i = 0; i < extra.param3; i++)
		{
			Param3<int, string, vector> marker = new Param3<int, string, vector>(0, "", vector.Zero);
			if (!ctx.Read(marker))
			{
				JobsLog.Error("CLIENT/RPC: список меток оборван на записи " + i.ToString() + ".");
				return;
			}

			view.m_Markers.Insert(new JobsModMarker(marker.param1, marker.param2, marker.param3));
		}

		s_JobView = view;

		// The open NPC menu was drawn from a state that has just changed —
		// taking a job, or handing one in. Closing it is both the honest thing
		// to show and what the player expects after pressing the button.
		if (s_NpcMenuOpen)
			CloseNpcMenu();

		JobsLog.Debug("CLIENT/JOBS: состояние работы: статус=" + view.m_Status.ToString() + ", прогресс=" + view.m_Progress.ToString() + "/" + view.m_Required.ToString() + ".");
	}

	static void HandleJobMessage(ParamsReadContext ctx)
	{
		Param2<string, string> data = new Param2<string, string>("", "");
		if (!ctx.Read(data))
		{
			JobsLog.Error("CLIENT/RPC: не удалось прочитать NOTIFY_JOB_MESSAGE.");
			return;
		}

		NotificationSystem.AddNotificationExtended(6.0, data.param1, data.param2);
	}

	// The HUD asks for this every frame, so it must always answer with something
	// drawable rather than null.
	static JobsModJobView GetJobView()
	{
		if (!s_JobView)
			s_JobView = new JobsModJobView();

		return s_JobView;
	}

	static JobsModNpcOffer ConsumePendingOffer()
	{
		return s_PendingOffer;
	}

	// =====================================================================
	// Menu lifecycle
	//
	// The menu classes live in 5_Mission and this file is compiled with 4_World,
	// which is earlier — so naming them here is not possible. The mission
	// subscribes to these invokers during its own init and does the opening;
	// this file only ever says which menu and when.
	// =====================================================================
	protected static ref ScriptInvoker s_OnOpenMenu;
	protected static ref ScriptInvoker s_OnCloseMenu;

	static ScriptInvoker GetOnOpenMenu()
	{
		if (!s_OnOpenMenu)
			s_OnOpenMenu = new ScriptInvoker();

		return s_OnOpenMenu;
	}

	static ScriptInvoker GetOnCloseMenu()
	{
		if (!s_OnCloseMenu)
			s_OnCloseMenu = new ScriptInvoker();

		return s_OnCloseMenu;
	}

	protected static void OpenSortingMenu()
	{
		if (s_MenuOpen)
		{
			JobsLog.Warning("CLIENT/UI: меню сортировки уже открыто, повторное открытие пропущено.");
			return;
		}

		s_MenuOpen = true;
		GetOnOpenMenu().Invoke(JobsModMenuIds.TRASH_SORTING);
	}

	// Called by the menu itself from OnHide, whatever closed it.
	static void OnSortingMenuClosed()
	{
		s_MenuOpen = false;
	}

	protected static void OpenNpcMenu()
	{
		if (s_NpcMenuOpen)
			return;

		s_NpcMenuOpen = true;
		GetOnOpenMenu().Invoke(JobsModMenuIds.NPC_JOBS);
	}

	protected static void CloseNpcMenu()
	{
		GetOnCloseMenu().Invoke(JobsModMenuIds.NPC_JOBS);
	}

	static void OnNpcMenuClosed()
	{
		s_NpcMenuOpen = false;
		s_PendingOffer = null;
	}

	// The mission reports back when a menu could not be created, so a failed
	// open does not leave the flag stuck and block every later attempt.
	static void OnMenuOpenFailed(int menuId)
	{
		if (menuId == JobsModMenuIds.TRASH_SORTING)
			s_MenuOpen = false;

		if (menuId == JobsModMenuIds.NPC_JOBS)
			s_NpcMenuOpen = false;
	}

	// The mission passes the session in while the menu is being constructed,
	// because Init() runs before the caller gets the instance back.
	static JobsModSortingSession ConsumePendingSession()
	{
		return s_PendingSession;
	}

	// =====================================================================
	// Outgoing
	// =====================================================================
	static void SendSortingSubmit(int nonce, array<string> sequence, int mistakes)
	{
		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		if (!player)
			return;

		string packed = "";
		for (int i = 0; i < sequence.Count(); i++)
		{
			if (i > 0)
				packed = packed + JobsModRPC.FIELD_SEPARATOR;

			packed = packed + sequence.Get(i);
		}

		GetGame().RPCSingleParam(
			player,
			JobsModRPC.REQUEST_SORTING_SUBMIT,
			new Param4<int, int, string, int>(JobsModRPC.PROTOCOL_VERSION, nonce, packed, mistakes),
			true,
			null);

		JobsLog.Debug("CLIENT/RPC: результат сортировки отправлен; nonce=" + nonce.ToString() + ".");
	}

	static void SendSortingAbort(int nonce)
	{
		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		if (!player)
			return;

		GetGame().RPCSingleParam(
			player,
			JobsModRPC.REQUEST_SORTING_ABORT,
			new Param2<int, int>(JobsModRPC.PROTOCOL_VERSION, nonce),
			true,
			null);

		s_PendingSession = null;
		JobsLog.Debug("CLIENT/RPC: сортировка прервана; nonce=" + nonce.ToString() + ".");
	}

	static void SendJobAccept(string npcId, string jobId)
	{
		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		if (!player)
			return;

		GetGame().RPCSingleParam(
			player,
			JobsModRPC.REQUEST_JOB_ACCEPT,
			new Param3<int, string, string>(JobsModRPC.PROTOCOL_VERSION, npcId, jobId),
			true,
			null);

		JobsLog.Debug("CLIENT/RPC: запрос на работу '" + jobId + "' у NPC '" + npcId + "'.");
	}

	static void SendJobComplete(string npcId, int assignmentId)
	{
		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		if (!player)
			return;

		GetGame().RPCSingleParam(
			player,
			JobsModRPC.REQUEST_JOB_COMPLETE,
			new Param3<int, string, int>(JobsModRPC.PROTOCOL_VERSION, npcId, assignmentId),
			true,
			null);

		JobsLog.Debug("CLIENT/RPC: сдача работы " + assignmentId.ToString() + " NPC '" + npcId + "'.");
	}

	static void SendJobAbandon(int assignmentId)
	{
		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		if (!player)
			return;

		GetGame().RPCSingleParam(
			player,
			JobsModRPC.REQUEST_JOB_ABANDON,
			new Param2<int, int>(JobsModRPC.PROTOCOL_VERSION, assignmentId),
			true,
			null);

		JobsLog.Debug("CLIENT/RPC: отказ от работы " + assignmentId.ToString() + ".");
	}
}
