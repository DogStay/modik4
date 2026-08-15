// CacheModClock.c
//
// A UTC timestamp in seconds that survives a restart.
//
// Respawn timers are the reason this exists. Measuring them against the
// engine's own tick counter would reset every restart, which would either
// resurrect every emptied cache at once or freeze them all — depending on which
// way the comparison happens to fall. A wall-clock second, written to JSON, is
// the only form of "later" that still means something after the process dies.
//
// The engine hands out the date in pieces; turning those into a single number
// is the days-from-civil conversion, which is exact for every date the game
// will ever see and needs no table.

class CacheModClock
{
	static int GetUtcSeconds()
	{
		int year;
		int month;
		int day;
		int hour;
		int minute;
		int second;

		GetYearMonthDayUTC(year, month, day);
		GetHourMinuteSecondUTC(hour, minute, second);

		int days = DaysFromCivil(year, month, day);
		int total = days * 86400;
		total = total + hour * 3600;
		total = total + minute * 60;
		total = total + second;

		return total;
	}

	// Days since 1970-01-01. Howard Hinnant's civil-date algorithm, written out
	// in full rather than folded into one expression: Enforce is happier with
	// small statements, and so is anyone reading this in a year.
	protected static int DaysFromCivil(int year, int month, int day)
	{
		int y = year;
		if (month <= 2)
			y = y - 1;

		int era = y / 400;
		if (y < 0)
			era = (y - 399) / 400;

		int yoe = y - era * 400;

		int monthShift = month + 9;
		if (month > 2)
			monthShift = month - 3;

		int doy = (153 * monthShift + 2) / 5 + day - 1;
		int doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;

		return era * 146097 + doe - 719468;
	}
}
