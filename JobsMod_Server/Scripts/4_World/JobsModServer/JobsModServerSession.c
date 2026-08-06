// JobsModServerSession.c
//
// One sorting session as the server knows it. This is the authoritative record;
// the client's copy is a rendering aid.
//
// It stores the three things a submission is judged against: the nonce that
// identifies the grant, where the pile stood when it was granted, and when the
// clock started.

class JobsModServerSession
{
	protected int m_Nonce;
	protected string m_PlayerId;
	protected string m_ZoneName;
	protected vector m_PilePosition;
	protected Object m_Pile;
	protected int m_StartedAtMs;
	protected ref array<string> m_Order;

	void JobsModServerSession(int nonce, string playerId, string zoneName, Object pile, array<string> order)
	{
		m_Nonce = nonce;
		m_PlayerId = playerId;
		m_ZoneName = zoneName;
		m_Pile = pile;
		m_PilePosition = pile.GetPosition();
		m_StartedAtMs = GetGame().GetTime();

		m_Order = new array<string>();
		for (int i = 0; i < order.Count(); i++)
		{
			m_Order.Insert(order.Get(i));
		}
	}

	int GetNonce() { return m_Nonce; }
	string GetPlayerId() { return m_PlayerId; }
	string GetZoneName() { return m_ZoneName; }
	vector GetPilePosition() { return m_PilePosition; }

	// May be null by submit time: another system can always delete a world
	// object, so the position captured at grant time is what distance is
	// checked against, not this reference.
	Object GetPile() { return m_Pile; }
	array<string> GetOrder() { return m_Order; }

	int GetElapsedSeconds()
	{
		return (GetGame().GetTime() - m_StartedAtMs) / 1000;
	}

	bool IsExpired()
	{
		return GetElapsedSeconds() > JobsModRPC.SESSION_TIMEOUT_SECONDS;
	}

	// Grades a submitted bin sequence against the order this session handed out.
	// Returns false on the first mismatch: a partially correct submission is not
	// a finished job, and the caller has no use for how far it got.
	bool Matches(array<string> submittedBins)
	{
		if (submittedBins.Count() != m_Order.Count())
			return false;

		for (int i = 0; i < m_Order.Count(); i++)
		{
			if (JobsModTrashCatalog.GetBinIdForItem(m_Order.Get(i)) != submittedBins.Get(i))
				return false;
		}

		return true;
	}
}
