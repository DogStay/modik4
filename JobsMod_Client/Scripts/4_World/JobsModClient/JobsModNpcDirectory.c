// JobsModNpcDirectory.c
//
// Where the employers stand, as far as the client knows.
//
// The client cannot tell an employer from any other survivor: which classes are
// NPCs, and which of them the server actually spawned, lives in a server-only
// config that clients never download. Without this list the talk action would
// either have to be offered on every human in the game or on none of them.
//
// This is presentation only. It decides whether the action appears in the
// player's action menu, nothing else — when the action fires, the server
// resolves the object itself and refuses anything that is not one of its own
// NPCs. A tampered directory therefore buys nothing but a menu entry that gets
// rejected.

class JobsModNpcDirectory
{
	// NPCs do not move, so matching by position is exact in practice. The
	// tolerance only has to cover the difference between where the server placed
	// the NPC and where the client has it standing after its own ground snap.
	protected static const float MATCH_RADIUS = 2.5;

	protected static ref array<vector> s_Positions;
	protected static ref array<string> s_Names;

	static void Clear()
	{
		s_Positions = new array<vector>();
		s_Names = new array<string>();
	}

	static void Add(string name, vector position)
	{
		if (!s_Positions)
			Clear();

		s_Names.Insert(name);
		s_Positions.Insert(position);
	}

	static int GetCount()
	{
		if (!s_Positions)
			return 0;

		return s_Positions.Count();
	}

	// True when something standing here is one of the mod's employers. Compared
	// in 2D: the client's ground height under a survivor is not guaranteed to
	// match the server's to the centimetre, and a height difference is never
	// what tells two NPCs apart.
	static bool IsNpcAt(vector position)
	{
		if (!s_Positions)
			return false;

		for (int i = 0; i < s_Positions.Count(); i++)
		{
			vector known = s_Positions.Get(i);
			float dx = position[0] - known[0];
			float dz = position[2] - known[2];

			if ((dx * dx + dz * dz) <= (MATCH_RADIUS * MATCH_RADIUS))
				return true;
		}

		return false;
	}
}
