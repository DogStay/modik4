// JobsModSortingSession.c
//
// The client-side copy of one granted sorting session.
//
// The server owns the real session; this is only what the menu needs in order
// to draw itself and to label the submission it sends back. The nonce is the
// single value that ties a submission to the grant — it is never generated
// here, only echoed, so a client cannot invent a session it was not given.

class JobsModSortingSession
{
	protected int m_Nonce;
	protected string m_ZoneName;
	protected ref array<string> m_Order;

	void JobsModSortingSession(int nonce, string zoneName, array<string> order)
	{
		m_Nonce = nonce;
		m_ZoneName = zoneName;
		m_Order = new array<string>();

		for (int i = 0; i < order.Count(); i++)
		{
			m_Order.Insert(order.Get(i));
		}
	}

	int GetNonce() { return m_Nonce; }
	string GetZoneName() { return m_ZoneName; }
	array<string> GetOrder() { return m_Order; }

	int GetItemCount() { return m_Order.Count(); }

	string GetItemId(int index)
	{
		if (index < 0 || index >= m_Order.Count())
			return "";

		return m_Order.Get(index);
	}
}
