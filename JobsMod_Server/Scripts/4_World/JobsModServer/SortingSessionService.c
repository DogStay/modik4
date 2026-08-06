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
// =========================================================================

class SortingSessionService
{
	// Sorting nine items faster than this is not physically plausible, so a
	// submission below it did not come from someone playing the minigame.
	protected static const int MIN_PLAY_SECONDS = 5;

	protected static const int REWARD_QUANTITY = 25;
	protected static const string REWARD_CLASS = "JobsMod_Money";

	protected ref map<string, ref JobsModServerSession> m_Sessions;

	void SortingSessionService()
	{
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
		string zoneName = ResolveZoneName(pile);

		JobsModServerSession session = new JobsModServerSession(
			GenerateNonce(), playerId, zoneName, pile.GetPosition(), order);

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

		m_Sessions.Remove(playerId);
		GrantReward(player, identity, request.param4);
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
	// Reward
	// =====================================================================
	protected void GrantReward(PlayerBase player, PlayerIdentity identity, int mistakes)
	{
		EntityAI reward = player.GetInventory().CreateInInventory(REWARD_CLASS);

		// A full inventory must not swallow the pay: fall back to the ground at
		// the player's feet rather than silently dropping the reward.
		if (!reward)
		{
			Object spawned = GetGame().CreateObjectEx(REWARD_CLASS, player.GetPosition(), ECE_PLACE_ON_SURFACE);
			reward = EntityAI.Cast(spawned);
		}

		if (reward)
		{
			ItemBase rewardItem = ItemBase.Cast(reward);
			if (rewardItem)
				rewardItem.SetQuantity(REWARD_QUANTITY);
		}
		else
		{
			JobsLog.Error("SERVER/JANITOR: награду '" + REWARD_CLASS + "' выдать не удалось.");
		}

		string message = "Смена принята. Начислено: " + REWARD_QUANTITY.ToString() + ".";
		if (mistakes > 0)
			message = message + " Ошибок за смену: " + mistakes.ToString() + ".";

		GetGame().RPCSingleParam(
			player,
			JobsModRPC.NOTIFY_SORTING_ACCEPTED,
			new Param1<string>(message),
			true,
			identity);

		JobsLog.Info("SERVER/JANITOR: смена принята; игрок='" + identity.GetName()
			+ "', ошибок=" + mistakes.ToString() + ".");
	}

	// =====================================================================
	// Helpers
	// =====================================================================
	protected void Reject(PlayerBase player, PlayerIdentity identity, int reason)
	{
		GetGame().RPCSingleParam(
			player,
			JobsModRPC.NOTIFY_SORTING_REJECTED,
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

	protected string ResolveZoneName(Object pile)
	{
		// Zones are not configurable yet, so the label is derived from where the
		// pile actually stands. This keeps the header honest instead of showing
		// a hard-coded name that may not match the location.
		vector position = pile.GetPosition();
		return "X " + Math.Round(position[0]).ToString() + " / Z " + Math.Round(position[2]).ToString();
	}
}
