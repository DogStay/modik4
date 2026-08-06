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
			JobsLog.Error("CLIENT/RPC: не удалось прочитать NOTIFY_SORTING_REJECTED.");
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
		JobsLog.Info("CLIENT/JANITOR: сервер принял смену.");
		NotificationSystem.AddNotificationExtended(6.0, "СМЕНА ЗАВЕРШЕНА", data.param1);
	}

	// =====================================================================
	// Menu lifecycle
	// =====================================================================
	protected static void OpenSortingMenu()
	{
		if (s_MenuOpen)
		{
			JobsLog.Warning("CLIENT/UI: меню сортировки уже открыто, повторное открытие пропущено.");
			return;
		}

		UIManager manager = GetGame().GetUIManager();
		if (!manager)
		{
			JobsLog.Error("CLIENT/UI: UIManager недоступен.");
			return;
		}

		TrashSortingMenu menu = TrashSortingMenu.Cast(manager.EnterScriptedMenu(JobsModMenuIds.TRASH_SORTING, null));
		if (!menu)
		{
			JobsLog.Error("CLIENT/UI: меню сортировки не создано.");
			return;
		}

		s_MenuOpen = true;
	}

	// Called by the menu itself from OnHide, whatever closed it.
	static void OnSortingMenuClosed()
	{
		s_MenuOpen = false;
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
	static void SendSortingStart()
	{
		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		if (!player)
			return;

		GetGame().RPCSingleParam(
			player,
			JobsModRPC.REQUEST_SORTING_START,
			new Param1<int>(JobsModRPC.PROTOCOL_VERSION),
			true,
			null);

		JobsLog.Debug("CLIENT/RPC: запрошено начало сортировки.");
	}

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
}
