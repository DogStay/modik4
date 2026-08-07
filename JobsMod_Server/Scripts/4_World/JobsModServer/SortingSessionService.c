// SortingSessionService.c
//
// Owns every open sorting session and is the only place a submission can be
// accepted.
//
// =========================================================================
// TRUST MODEL
// =========================================================================
// Untrusted: everything inside ParamsReadContext. A client may send any nonce,
// any sequence, any protocol version, at any rate.
//
// Trusted: PlayerIdentity, which the engine attaches rather than the client.
// The player a request acts on is therefore never read from the payload — that
// is what stops one player from submitting in another's name.
//
// A client cannot finish a job by asking. There is no "job complete" message:
// completion is something the server concludes after checking a submission
// against a session it granted itself. Removing the whole class of "client
// claims success" exploits is worth more than validating such a claim.
//
// What each check is actually for:
//   nonce      - proves the session was granted, not invented
//   identity   - proves it is the player who was granted it
//   expiry     - bounds how long a stale grant stays usable
//   min time   - a human cannot sort nine items instantly; catches automation
//   distance   - the player must still be at the pile, not across the map
//   sequence   - catches a malformed or tampered client
//
// The minigame pays nothing by itself. A sorted pile is one step of the job an
// NPC handed out, and the money is paid once, by that NPC, when the whole job is
// signed off — so there is no second economy running alongside the jobs.
// =========================================================================

class SortingSessionService
{
	// Sorting nine items faster than this is not physically plausible, so a
	// submission below it did not come from someone playing the minigame.
	protected static const int MIN_PLAY_SECONDS = 5;

	protected ref map<string, ref JobsModServerSession> m_Sessions;
	protected ref JobsModConfig m_Config;
	protected TrashZoneService m_Zones;
	protected JobsModJobService m_Jobs;

	void SortingSessionService(JobsModConfig config, TrashZoneService zones, JobsModJobService jobs)
	{
		m_Config = config;
		m_Zones = zones;
		m_Jobs = jobs;
		m_Sessions = new map<string, ref JobsModServerSession>();
	}

	// =====================================================================
	// Granting
	// =====================================================================
	void HandleSortRequested(PlayerBase player, Object pile)
	{
		if (!GetGame().IsServer() || !player || !pile)
			return;

		PlayerIdentity identity = player.GetIdentity();
		if (!identity)
			return;

		string playerId = identity.GetId();

		if (!IsPlayerReady(player))
		{
			Reject(player, identity, JobsModRejectReason.PLAYER_NOT_READY);
			return;
		}

		// Only piles this server placed count. Anything else with the same class
		// name -- spawned by an admin tool or another mod -- is not a work point
		// and must not be a source of pay.
		if (!m_Zones.IsManagedPile(pile))
		{
			Reject(player, identity, JobsModRejectReason.UNKNOWN_PILE);
			return;
		}

		// Sorting is not a thing you can do on your own: it is one step of a job
		// an NPC handed out, and the pile has to belong to that job's zone. That
		// keeps the NPC as the single way into the work, and it is what stops a
		// player from farming piles all over the map on one assignment.
		JobsModAssignment assignment = m_Jobs.GetAssignment(playerId);
		if (!assignment || assignment.GetType() != JobsModJobType.SORTING || !assignment.IsActive())
		{
			Reject(player, identity, JobsModRejectReason.NO_ACTIVE_JOB);
			return;
		}

		if (m_Zones.GetZoneId(pile) != assignment.GetZoneId())
		{
			Reject(player, identity, JobsModRejectReason.WRONG_ZONE);
			return;
		}

		if (vector.Distance(player.GetPosition(), pile.GetPosition()) > JobsModRPC.INTERACTION_DISTANCE)
		{
			Reject(player, identity, JobsModRejectReason.TOO_FAR);
			return;
		}

		// An open session for this player is either stale or a second pile being
		// opened while the first is unfinished. Expired ones are silently
		// replaced; a live one blocks, so a player cannot farm two piles at once.
		JobsModServerSession existing;
		if (m_Sessions.Find(playerId, existing) && existing)
		{
			if (!existing.IsExpired())
			{
				Reject(player, identity, JobsModRejectReason.ALREADY_BUSY);
				return;
			}

			m_Sessions.Remove(playerId);
		}

		array<string> order = BuildShuffledOrder();
		string zoneName = m_Zones.GetZoneName(pile);

		JobsModServerSession session = new JobsModServerSession(
			GenerateNonce(), playerId, zoneName, pile, order);

		m_Sessions.Set(playerId, session);

		GetGame().RPCSingleParam(
			player,
			JobsModRPC.NOTIFY_SORTING_SESSION,
			new Param3<int, string, string>(session.GetNonce(), zoneName, JobsModTrashCatalog.PackOrder(order)),
			true,
			identity);

		JobsLog.Info("SERVER/JANITOR: смена выдана; игрок='" + identity.GetName()
			+ "', зона='" + zoneName + "', nonce=" + session.GetNonce().ToString() + ".");
	}

	// =====================================================================
	// Submission
	// =====================================================================
	void HandleSubmit(PlayerBase player, PlayerIdentity identity, ParamsReadContext ctx)
	{
		if (!GetGame().IsServer() || !player || !identity)
			return;

		Param4<int, int, string, int> request = new Param4<int, int, string, int>(0, 0, "", 0);
		if (!ctx.Read(request))
		{
			JobsLog.Warning("SERVER/RPC: не удалось прочитать REQUEST_SORTING_SUBMIT; игрок='" + identity.GetName() + "'.");
			return;
		}

		if (request.param1 != JobsModRPC.PROTOCOL_VERSION)
		{
			Reject(player, identity, JobsModRejectReason.PROTOCOL_MISMATCH);
			return;
		}

		string playerId = identity.GetId();
		JobsModServerSession session;

		if (!m_Sessions.Find(playerId, session) || !session)
		{
			Reject(player, identity, JobsModRejectReason.NO_ACTIVE_SESSION);
			return;
		}

		// A mismatched nonce means the submission belongs to a session this
		// player no longer holds — a replay of an old grant, or a fabricated one.
		if (session.GetNonce() != request.param2)
		{
			JobsLog.Warning("SERVER/JANITOR: неверный nonce от '" + identity.GetName() + "'.");
			Reject(player, identity, JobsModRejectReason.NO_ACTIVE_SESSION);
			return;
		}

		if (session.IsExpired())
		{
			m_Sessions.Remove(playerId);
			Reject(player, identity, JobsModRejectReason.SESSION_EXPIRED);
			return;
		}

		if (session.GetElapsedSeconds() < MIN_PLAY_SECONDS)
		{
			JobsLog.Warning("SERVER/JANITOR: подозрительно быстрая сдача от '" + identity.GetName()
				+ "' (" + session.GetElapsedSeconds().ToString() + " с), отклонено.");
			Reject(player, identity, JobsModRejectReason.RESULT_INCORRECT);
			return;
		}

		if (vector.Distance(player.GetPosition(), session.GetPilePosition()) > JobsModRPC.INTERACTION_DISTANCE)
		{
			Reject(player, identity, JobsModRejectReason.TOO_FAR);
			return;
		}

		array<string> submitted = UnpackBinSequence(request.param3);
		if (!submitted || !session.Matches(submitted))
		{
			Reject(player, identity, JobsModRejectReason.RESULT_INCORRECT);
			return;
		}

		// The assignment is re-read here rather than trusted from the grant: the
		// player may have abandoned the job, or it may have timed out, in the
		// minute they spent in the menu.
		JobsModAssignment assignment = m_Jobs.GetAssignment(playerId);
		if (!assignment || assignment.GetType() != JobsModJobType.SORTING || !assignment.IsActive())
		{
			m_Sessions.Remove(playerId);
			Reject(player, identity, JobsModRejectReason.NO_ACTIVE_JOB);
			return;
		}

		m_Sessions.Remove(playerId);

		// The heap has been worked: take it out of the world before crediting it,
		// so a failure downstream can never leave a pile that is still workable.
		m_Zones.ConsumePile(session.GetPile());

		string message = "Куча разобрана.";
		if (request.param4 > 0)
			message = message + " Ошибок: " + request.param4.ToString() + ".";

		GetGame().RPCSingleParam(
			player,
			JobsModRPC.NOTIFY_SORTING_ACCEPTED,
			new Param1<string>(message),
			true,
			identity);

		m_Jobs.ReportSortedPile(player, identity, assignment);

		JobsLog.Info("SERVER/JANITOR: куча принята; игрок='" + identity.GetName()
			+ "', ошибок=" + request.param4.ToString()
			+ ", прогресс " + assignment.GetProgress().ToString() + "/" + assignment.GetRequired().ToString() + ".");
	}

	void HandleAbort(PlayerBase player, PlayerIdentity identity, ParamsReadContext ctx)
	{
		if (!GetGame().IsServer() || !identity)
			return;

		Param2<int, int> request = new Param2<int, int>(0, 0);
		if (!ctx.Read(request) || request.param1 != JobsModRPC.PROTOCOL_VERSION)
			return;

		string playerId = identity.GetId();
		JobsModServerSession session;

		// Only drop the session the client names. Without the nonce check a
		// stale abort could cancel a session the player has since restarted.
		if (m_Sessions.Find(playerId, session) && session && session.GetNonce() == request.param2)
		{
			m_Sessions.Remove(playerId);
			JobsLog.Info("SERVER/JANITOR: смена прервана игроком '" + identity.GetName() + "'.");
		}
	}

	// Frees sessions of players who disconnected or simply walked away, so the
	// map cannot grow without bound over a long server uptime.
	void PruneExpired()
	{
		array<string> stale = new array<string>();

		for (int i = 0; i < m_Sessions.Count(); i++)
		{
			JobsModServerSession session = m_Sessions.GetElement(i);
			if (!session || session.IsExpired())
				stale.Insert(m_Sessions.GetKey(i));
		}

		for (int s = 0; s < stale.Count(); s++)
		{
			m_Sessions.Remove(stale.Get(s));
			JobsLog.Debug("SERVER/JANITOR: просроченная смена удалена.");
		}
	}

	void DropPlayer(string playerId)
	{
		if (m_Sessions.Contains(playerId))
			m_Sessions.Remove(playerId);
	}

	int GetActiveCount()
	{
		return m_Sessions.Count();
	}

	// =====================================================================
	// Helpers
	// =====================================================================
	protected void Reject(PlayerBase player, PlayerIdentity identity, int reason)
	{
		GetGame().RPCSingleParam(
			player,
			JobsModRPC.NOTIFY_REJECTED,
			new Param1<int>(reason),
			true,
			identity);

		JobsLog.Debug("SERVER/JANITOR: отказ игроку '" + identity.GetName() + "'; причина=" + reason.ToString() + ".");
	}

	protected bool IsPlayerReady(PlayerBase player)
	{
		return player.IsAlive() && !player.IsUnconscious() && !player.IsRestrained();
	}

	protected int GenerateNonce()
	{
		return Math.RandomInt(1, 2000000000);
	}

	// Fisher-Yates over the catalog ids. Shuffling server-side is what keeps the
	// order out of the client's control; a client-chosen order would let a
	// player pre-arrange an easy round.
	protected array<string> BuildShuffledOrder()
	{
		array<string> order = new array<string>();
		array<ref JobsModTrashItem> items = JobsModTrashCatalog.GetItems();

		int i;
		for (i = 0; i < items.Count(); i++)
		{
			order.Insert(items.Get(i).GetId());
		}

		for (i = order.Count() - 1; i > 0; i--)
		{
			int j = Math.RandomInt(0, i + 1);
			string swap = order.Get(i);
			order.Set(i, order.Get(j));
			order.Set(j, swap);
		}

		return order;
	}

	protected array<string> UnpackBinSequence(string packed)
	{
		if (packed == "")
			return null;

		array<string> parts = new array<string>();
		packed.Split(JobsModRPC.FIELD_SEPARATOR, parts);

		if (parts.Count() != JobsModTrashCatalog.ITEM_COUNT)
			return null;

		for (int i = 0; i < parts.Count(); i++)
		{
			if (JobsModTrashBin.GetIndexById(parts.Get(i)) < 0)
				return null;
		}

		return parts;
	}
}
