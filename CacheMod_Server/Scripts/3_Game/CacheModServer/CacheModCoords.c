// CacheModCoords.c
//
// Reads a coordinate out of whatever an admin pasted into the config, and
// writes one back in the shape the game itself prints.
//
// Caches are normally placed in-game and written by the server, so most of the
// coordinates in caches.json were produced by Format(). Loot zones are the
// other way round: an admin stands at the centre of the airfield, types
// #position, and pastes what the game printed. The parser is built around that
// second case, because it is the one where the text can be anything.
//
// All of these mean the same point:
//
//     "12000 10 8000"
//     "12000, 10, 8000"
//     "<12000.5, 10.3, 8000.1>"
//     "[12000; 10; 8000]"
//     "12000 8000"              -- two numbers: x and z, height left at zero
//
// Separators are discarded and the numbers read in order, so an admin never has
// to know which of those shapes was wanted.

class CacheModCoords
{
	protected static const string NUMERIC_CHARS = "0123456789+-.eE";
	protected static const string DIGITS = "0123456789";

	// False when nothing readable was there, so a typo is reported against the
	// id that carries it rather than quietly becoming 0 0 0 — a real place on
	// every map, in the sea off the south-west corner, and a baffling one to
	// find a loot zone sitting in.
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

		if (numbers.Count() >= 3)
		{
			result = Vector(numbers.Get(0), numbers.Get(1), numbers.Get(2));
			return true;
		}

		return false;
	}

	// The shape the server writes. Two decimals is finer than any placement the
	// hologram can produce and keeps the file readable.
	static string Format(vector value)
	{
		string x = value[0].ToString();
		string y = value[1].ToString();
		string z = value[2].ToString();

		return x + " " + y + " " + z;
	}

	protected static void ReadNumbers(string text, out array<float> numbers)
	{
		string cleaned = text;

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

			// ToFloat answers 0 for anything it cannot read, so a token is
			// checked before it is converted: without that, "x=12000" would
			// parse as zero and look like a deliberate coordinate.
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
