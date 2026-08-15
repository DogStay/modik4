// CacheModClientContext.c
//
// The client's whole state, which is deliberately almost nothing: a search
// duration, the last notification, and — for an administrator — the point list
// behind the debug spheres.
//
// Nothing here decides anything. A cache the client does not know about is a
// cache it cannot betray to the player, so the client is told about caches only
// when an authorised administrator asks, and even then only what a sphere needs
// to be drawn.

class CacheModClientContext
{
	// The admin point list as last sent by the server, already unpacked. Empty
	// for an ordinary player, and empty for an admin who has not switched the
	// display on.
	protected static ref array<ref CacheModAdminPoint> s_AdminPoints;

	// Raised whenever a fresh point list arrives, so the sphere renderer in the
	// admin PBO can redraw without polling. The renderer lives in a different
	// PBO that this one may not name — an invoker is the only way across.
	protected static ref ScriptInvoker s_OnAdminPointsChanged;

	static ScriptInvoker GetOnAdminPointsChanged()
	{
		if (!s_OnAdminPointsChanged)
			s_OnAdminPointsChanged = new ScriptInvoker();

		return s_OnAdminPointsChanged;
	}

	static array<ref CacheModAdminPoint> GetAdminPoints()
	{
		if (!s_AdminPoints)
			s_AdminPoints = new array<ref CacheModAdminPoint>();

		return s_AdminPoints;
	}

	// Asks the server for the current picture. The server answers only if the
	// sender holds the Cache Manager permission; there is no client-side check
	// here on purpose, because a client-side one would be worth nothing.
	static void RequestAdminPoints()
	{
		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		if (!player)
			return;

		GetGame().RPCSingleParam(
			player,
			CacheModRPC.REQUEST_ADMIN_POINTS,
			new Param1<int>(0),
			true,
			null);
	}

	// =====================================================================
	// Incoming
	// =====================================================================
	static void HandleMessage(ParamsReadContext ctx)
	{
		Param1<string> data = new Param1<string>("");
		if (!ctx.Read(data))
			return;

		string text = data.param1;
		if (text == "")
			return;

		// The vanilla notification strip is enough for the two things the mod
		// ever has to say, and it costs no layout of our own.
		NotificationSystem.AddNotificationExtended(6.0, "ТАЙНИК", text);
	}

	static void HandleSettings(ParamsReadContext ctx)
	{
		Param1<float> data = new Param1<float>(0);
		if (!ctx.Read(data))
			return;

		CacheModSearchTuning.SetDurationSeconds(data.param1);
		CacheLog.Debug(CacheLog.ROOT, "CLIENT: длительность обыска = " + data.param1.ToString() + " c.");
	}

	// The point list arrives as one packed string: records separated by "|",
	// fields inside a record by ";". One string rather than a Param per point
	// because the count is unbounded — a server may hold hundreds of caches —
	// and an unbounded array of Params is not something an RPC can carry.
	static void HandleAdminPoints(ParamsReadContext ctx)
	{
		Param1<string> data = new Param1<string>("");
		if (!ctx.Read(data))
			return;

		ApplyPackedPoints(data.param1);
	}

	// The same list can arrive two ways: through the mod's own RPC, or through
	// VPP's channel when the Cache Manager asked for it. Both land here, so the
	// spheres and the menu can never disagree about what the server said.
	static void ApplyPackedPoints(string packed)
	{
		s_AdminPoints = new array<ref CacheModAdminPoint>();

		array<string> records = new array<string>();
		packed.Split(CacheModRPC.RECORD_SEPARATOR, records);

		for (int i = 0; i < records.Count(); i++)
		{
			string record = records.Get(i);
			if (record == "")
				continue;

			CacheModAdminPoint point = CacheModAdminPoint.Parse(record);
			if (point)
				s_AdminPoints.Insert(point);
		}

		CacheLog.Debug(CacheLog.ADMIN, "CLIENT: получено точек: " + s_AdminPoints.Count().ToString() + ".");
		GetOnAdminPointsChanged().Invoke();
	}
}

// One cache as an administrator sees it: enough to draw a sphere and to label a
// row in the manager, and nothing more.
class CacheModAdminPoint
{
	string m_Id;
	vector m_Position;
	float m_Radius;
	int m_State;
	int m_CacheType;
	float m_SpawnChance;
	int m_RespawnSecondsLeft;

	// "id;x;y;z;radius;state;type;spawnChance;respawnLeft"
	static CacheModAdminPoint Parse(string record)
	{
		array<string> fields = new array<string>();
		record.Split(CacheModRPC.FIELD_SEPARATOR, fields);

		if (fields.Count() < 9)
			return null;

		CacheModAdminPoint point = new CacheModAdminPoint();
		point.m_Id = fields.Get(0);

		float x = fields.Get(1).ToFloat();
		float y = fields.Get(2).ToFloat();
		float z = fields.Get(3).ToFloat();
		point.m_Position = Vector(x, y, z);

		point.m_Radius = fields.Get(4).ToFloat();
		point.m_State = fields.Get(5).ToInt();
		point.m_CacheType = fields.Get(6).ToInt();
		point.m_SpawnChance = fields.Get(7).ToFloat();
		point.m_RespawnSecondsLeft = fields.Get(8).ToInt();

		return point;
	}

	string GetStateText()
	{
		if (m_State == CacheModState.ACTIVE)
			return "ACTIVE";

		if (m_State == CacheModState.COOLDOWN)
			return "COOLDOWN";

		return "INACTIVE";
	}
}
