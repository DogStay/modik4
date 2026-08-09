// JobsModCoords.c
//
// Reads a coordinate out of whatever an admin pasted into the config.
//
// Coordinates come from one place in practice: the admin stands where the thing
// should be, types #position, and copies what the game printed. So the parser is
// built around that rather than around a format of our own — the useful question
// is not "is this valid JSON" but "did the numbers survive the trip from the
// chat window into the file".
//
// All of these mean the same point:
//
//     "6600 300 2500"
//     "6600, 300, 2500"
//     "<6600.5, 302.3, 2500.1>"
//     "[6600; 300; 2500]"
//     "6600 2500"                 -- two numbers: x and z, height off the ground
//
// Separators are thrown away and the numbers are read in order, which is what
// makes every line above work without the admin having to know which one we
// wanted. Two numbers is treated as x and z on purpose: it is what somebody
// reading coordinates off a map has, and demanding a height they do not have
// would only get a zero typed in the middle anyway.
//
// A height of zero is not an error and not a position — the rest of the mod
// reads it as "put this on the terrain". That rule lives at the point of use,
// because only the caller knows whether snapping to the ground makes sense.

class JobsModCoords
{
	// Everything that can legally appear inside a number. Anything else in a
	// token means the token is not one.
	protected static const string NUMERIC_CHARS = "0123456789+-.eE";
	protected static const string DIGITS = "0123456789";

	// Returns false when there was nothing readable, so a typo is reported with
	// the id of the entry that carries it instead of quietly becoming 0 0 0 —
	// which is a real place on every map, out in the sea off the south-west
	// corner, and a very confusing place to find your NPC standing.
	static bool Parse(string text, out vector result)
	{
		result = vector.Zero;

		if (text == "")
			return false;

		array<float> numbers = new array<float>();
		ReadNumbers(text, numbers);

		if (numbers.Count() == 2)
		{
			result = Vector(numbers.Get(0), 0, numbers.Get(1));
			return true;
		}

		// More than three is not rejected: #position output has been seen with
		// trailing values, and the first three are the ones that mean anything.
		if (numbers.Count() >= 3)
		{
			result = Vector(numbers.Get(0), numbers.Get(1), numbers.Get(2));
			return true;
		}

		return false;
	}

	protected static void ReadNumbers(string text, out array<float> numbers)
	{
		string cleaned = text;

		// Every bracket and punctuation mark becomes a space, so the split below
		// only ever has one separator to think about.
		cleaned.Replace("<", " ");
		cleaned.Replace(">", " ");
		cleaned.Replace("[", " ");
		cleaned.Replace("]", " ");
		cleaned.Replace("(", " ");
		cleaned.Replace(")", " ");
		cleaned.Replace(",", " ");
		cleaned.Replace(";", " ");
		cleaned.Replace("\t", " ");
		cleaned.Replace("\r", " ");
		cleaned.Replace("\n", " ");

		array<string> tokens = new array<string>();
		cleaned.Split(" ", tokens);

		for (int i = 0; i < tokens.Count(); i++)
		{
			string token = tokens.Get(i);

			// ToFloat answers 0 for anything it cannot read, so a token has to be
			// checked before it is converted. Without that, "x=6600" would parse
			// as the number zero and look like a deliberate coordinate.
			if (!IsNumber(token))
				continue;

			numbers.Insert(token.ToFloat());
		}
	}

	protected static bool IsNumber(string token)
	{
		if (token == "")
			return false;

		bool sawDigit = false;

		for (int i = 0; i < token.Length(); i++)
		{
			string character = token.Get(i);

			if (NUMERIC_CHARS.IndexOf(character) < 0)
				return false;

			if (DIGITS.IndexOf(character) >= 0)
				sawDigit = true;
		}

		// A lone "-" or "." passes the character test and is still not a number.
		return sawDigit;
	}
}
