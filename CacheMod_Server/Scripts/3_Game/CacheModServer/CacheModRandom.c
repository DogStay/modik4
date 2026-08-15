// CacheModRandom.c
//
// The two rolls this mod makes, built on the one random call the codebase has
// already proven on this build.
//
// Math.RandomInt(min, maxExclusive) is what JobsMod uses and what compiles
// here. The float and inclusive-integer variants exist in some game versions
// and not in others, and a name that does not resolve costs the whole script
// module rather than one line — so the two shapes this mod actually needs are
// derived from the one call instead of taken on trust.

class CacheModRandom
{
	// A percentage in [0, 100) with two decimal places of resolution. That is
	// finer than any spawn chance or attachment chance an admin will write, and
	// it comes out of an integer draw of 10000 buckets.
	static float GetPercent()
	{
		int drawn = Math.RandomInt(0, 10000);
		return drawn / 100.0;
	}

	// An integer in [minimum, maximum], both ends included — the shape loot
	// counts are written in ("min": 1, "max": 2 means one or two).
	static int GetInt(int minimum, int maximum)
	{
		if (maximum <= minimum)
			return minimum;

		return Math.RandomInt(minimum, maximum + 1);
	}
}
