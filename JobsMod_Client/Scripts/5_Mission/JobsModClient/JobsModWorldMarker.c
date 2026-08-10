// JobsModWorldMarker.c
//
// The marker over the place the player has to get to, and how far it still is.
//
// It shows one point at a time, on purpose. A job can have a dozen relevant
// points — every pile in a zone, both ends of a freight route — and drawing all
// of them turns the screen into a field of pips that says nothing. One point,
// the one that matters right now, answers the only question the player has.
//
// Which point that is comes from JobsModJobView.PickMarker: the server sends
// every candidate with what it means, and the choice between them is made from
// what the player is doing — carrying a box or not, finished or not.
//
// When the target is off screen or behind the player the marker is pinned to
// the edge it lies towards rather than hidden. A marker that disappears the
// moment you turn around is worse than no marker: it stops answering exactly
// when the player looks away to find it.

class JobsModWorldMarker
{
	protected static const string LAYOUT_PATH = "JobsMod_Client/GUI/Layouts/JobsModWorldMarker.layout";

	// Where the pip floats above the point itself.
	protected static const float MARKER_HEIGHT = 1.6;

	// The group is this big in screen fractions; half of each is the offset
	// needed to centre it on the projected point.
	protected static const float GROUP_HALF_WIDTH = 0.05;
	protected static const float GROUP_HALF_HEIGHT = 0.0375;

	// Kept off the very border so the pip is never half outside the screen.
	protected static const float EDGE_MARGIN_X = 0.06;
	protected static const float EDGE_MARGIN_Y = 0.1;

	protected Widget m_Root;
	protected Widget m_Group;
	protected Widget m_PipOuter;
	protected Widget m_Stem;
	protected TextWidget m_Label;
	protected TextWidget m_Distance;

	void JobsModWorldMarker()
	{
		m_Root = GetGame().GetWorkspace().CreateWidgets(LAYOUT_PATH);
		if (!m_Root)
		{
			JobsLog.Error("CLIENT/UI: не удалось загрузить " + LAYOUT_PATH + ".");
			return;
		}

		m_Group = m_Root.FindAnyWidget("MarkerGroup");
		m_PipOuter = m_Root.FindAnyWidget("MarkerPipOuterBackground");
		m_Stem = m_Root.FindAnyWidget("MarkerStemBackground");
		m_Label = TextWidget.Cast(m_Root.FindAnyWidget("MarkerLabel"));
		m_Distance = TextWidget.Cast(m_Root.FindAnyWidget("MarkerDistance"));

		if (!m_Group || !m_PipOuter || !m_Stem || !m_Label || !m_Distance)
		{
			JobsLog.Error("CLIENT/UI: разметка метки неполная, 3D-метка отключена.");
			m_Root = null;
			return;
		}

		m_Root.Show(false);
	}

	void ~JobsModWorldMarker()
	{
		if (m_Root)
			m_Root.Unlink();
	}

	void Update(bool hudVisible)
	{
		if (!m_Root)
			return;

		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		JobsModJobView view = JobsModClientContext.GetJobView();

		if (!hudVisible || !player || !view.HasJob())
		{
			m_Root.Show(false);
			return;
		}

		JobsModMarker marker = view.PickMarker(player.GetPosition(), IsCarryingCargo(player, view));
		if (!marker)
		{
			m_Root.Show(false);
			return;
		}

		vector target = marker.m_Position;

		// Loading yards are configured as a circle on the map and carry no
		// height at all, so the ground under them is looked up here. Piles come
		// with the height they were actually placed at and are left alone.
		if (target[1] <= 0)
			target[1] = GetGame().SurfaceY(target[0], target[2]);

		target[1] = target[1] + MARKER_HEIGHT;

		float x;
		float y;
		bool onScreen = Project(target, x, y);

		m_Group.SetPos(x - GROUP_HALF_WIDTH, y - GROUP_HALF_HEIGHT);

		// The stem reads as "the point is down there" and only makes sense when
		// the marker is actually over the point.
		m_Stem.Show(onScreen);

		m_Label.SetText(marker.m_Label);
		m_Distance.SetText(FormatDistance(vector.Distance(player.GetPosition(), marker.m_Position)));

		int colour = JobsModPalette.Accent();
		if (marker.m_Kind == JobsModMarkerKind.EMPLOYER)
			colour = JobsModPalette.BorderFilled();

		m_PipOuter.SetColor(colour);
		m_Stem.SetColor(colour);
		m_Distance.SetColor(colour);

		m_Root.Show(true);
	}

	// Projects a world point to relative screen coordinates, and pins it to the
	// edge when it does not land on screen. Returns whether it landed on screen
	// by itself.
	protected bool Project(vector worldPosition, out float x, out float y)
	{
		vector screen = GetGame().GetScreenPosRelative(worldPosition);

		x = screen[0];
		y = screen[1];

		// A negative depth means the point is behind the camera, where the
		// projection mirrors it: the raw coordinates would send the player the
		// wrong way. Flipping the horizontal side and dropping it to the bottom
		// of the screen turns it into a "turn around, it is that way" arrow.
		bool behind = screen[2] <= 0;
		if (behind)
		{
			x = 1.0 - x;
			y = 1.0 - EDGE_MARGIN_Y;
		}

		bool inside = !behind && x >= EDGE_MARGIN_X && x <= 1.0 - EDGE_MARGIN_X && y >= EDGE_MARGIN_Y && y <= 1.0 - EDGE_MARGIN_Y;

		x = Math.Clamp(x, EDGE_MARGIN_X, 1.0 - EDGE_MARGIN_X);
		y = Math.Clamp(y, EDGE_MARGIN_Y, 1.0 - EDGE_MARGIN_Y);

		return inside;
	}

	// True while the player is holding this job's freight, which is what turns
	// the marker from the loading yard to the unloading yard.
	protected bool IsCarryingCargo(PlayerBase player, JobsModJobView view)
	{
		if (view.m_CargoClass == "")
			return false;

		EntityAI inHands = player.GetItemInHands();
		if (!inHands)
			return false;

		return inHands.GetType() == view.m_CargoClass;
	}

	// Kept to integer arithmetic on purpose: a float straight into ToString()
	// prints six decimal places, which is not a distance anyone can read.
	protected string FormatDistance(float metres)
	{
		if (metres >= 1000)
		{
			int tenths = Math.Round(metres / 100);
			int whole = tenths / 10;
			int fraction = tenths % 10;
			return whole.ToString() + "." + fraction.ToString() + " км";
		}

		int rounded = Math.Round(metres);
		return rounded.ToString() + " м";
	}
}
