// CacheModSphereRenderer.c
//
// Draws one sphere per cache, for administrators only.
//
// This is a debug view and nothing else. It has no bearing on whether a cache
// can be found: a player searches the interaction proxy, which exists whether
// or not anybody is drawing spheres, and an admin who switches the display off
// can still search the same points as everyone else.
//
// It is also client-local in the strictest sense. The shapes are created on the
// admin's own machine from a list the server sent to that one player after
// checking their permission — there is no entity, no synchronisation and
// nothing another client could see or ask for.
//
// The sphere is drawn at exactly cache.radius, because that number is the whole
// geometry of a cache: the same radius bounds the search on the server. What an
// admin sees is what a player has to stand inside.

class CacheModSphereRenderer
{
	// State colours, as 0xAARRGGBB.
	protected static const int COLOUR_ACTIVE = 0x4000FF00;
	protected static const int COLOUR_INACTIVE = 0x40808080;
	protected static const int COLOUR_COOLDOWN = 0x40FF0000;
	protected static const int COLOUR_TOOL = 0x40FFFF00;

	// Beyond this the spheres are noise: a hundred overlapping wireframes far
	// across the map hide the two the admin is standing next to.
	protected static const float DRAW_DISTANCE = 400.0;

	// How often the point list is asked for again while the display is on. The
	// only thing that changes on its own is a cooldown expiring, so a slow
	// refresh is enough and costs one small message a minute.
	protected static const int REFRESH_INTERVAL_MS = 20000;

	protected static bool s_Enabled;
	protected static ref array<Shape> s_Shapes;
	protected static bool s_Subscribed;

	static bool IsEnabled()
	{
		return s_Enabled;
	}

	static void Toggle()
	{
		SetEnabled(!s_Enabled);
	}

	static void SetEnabled(bool enabled)
	{
		if (s_Enabled == enabled)
			return;

		s_Enabled = enabled;

		if (!s_Enabled)
		{
			Clear();
			CacheLog.Info(CacheLog.ADMIN, "Отображение сфер выключено.");
			return;
		}

		Subscribe();
		CacheModClientContext.RequestAdminPoints();
		CacheLog.Info(CacheLog.ADMIN, "Отображение сфер включено.");
	}

	// Called by the client context whenever a fresh list arrives, and by the
	// refresh timer below.
	static void Redraw()
	{
		Clear();

		if (!s_Enabled)
			return;

		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		if (!player)
			return;

		vector viewer = player.GetPosition();

		array<ref CacheModAdminPoint> points = CacheModClientContext.GetAdminPoints();

		for (int i = 0; i < points.Count(); i++)
		{
			CacheModAdminPoint point = points.Get(i);
			if (!point)
				continue;

			float distance = vector.Distance(viewer, point.m_Position);
			if (distance > DRAW_DISTANCE)
				continue;

			int colour = GetColour(point);

			Shape sphere = Shape.CreateSphere(colour, ShapeFlags.TRANSP | ShapeFlags.NOZWRITE, point.m_Position, point.m_Radius);
			if (sphere)
				GetShapes().Insert(sphere);
		}

		CacheLog.Debug(CacheLog.ADMIN, "Отрисовано сфер: " + GetShapes().Count().ToString() + ".");
	}

	static void Clear()
	{
		array<Shape> shapes = GetShapes();

		for (int i = 0; i < shapes.Count(); i++)
		{
			Shape shape = shapes.Get(i);
			if (shape)
				shape.Destroy();
		}

		shapes.Clear();
	}

	// =====================================================================
	// Internals
	// =====================================================================
	protected static array<Shape> GetShapes()
	{
		if (!s_Shapes)
			s_Shapes = new array<Shape>();

		return s_Shapes;
	}

	// A cache that needs a tool is drawn yellow whatever else is true of it,
	// because that is the thing an admin standing in front of one cannot
	// otherwise see. State comes second.
	protected static int GetColour(CacheModAdminPoint point)
	{
		if (point.m_State == CacheModState.ACTIVE && point.m_CacheType == CacheModType.TOOL_REQUIRED)
			return COLOUR_TOOL;

		if (point.m_State == CacheModState.ACTIVE)
			return COLOUR_ACTIVE;

		if (point.m_State == CacheModState.COOLDOWN)
			return COLOUR_COOLDOWN;

		return COLOUR_INACTIVE;
	}

	protected static void Subscribe()
	{
		if (s_Subscribed)
			return;

		CacheModClientContext.GetOnAdminPointsChanged().Insert(Redraw);
		s_Subscribed = true;

		GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(RefreshTick, REFRESH_INTERVAL_MS, true);
	}

	protected static void RefreshTick()
	{
		if (!s_Enabled)
			return;

		CacheModClientContext.RequestAdminPoints();
	}
}
