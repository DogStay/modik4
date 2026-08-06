// JobsModJsonFileIO.c
//
// A narrow adapter over JsonFileLoader, kept in 3_Game on purpose.
//
// Instantiating the generic loader directly from a 4_World class has been seen
// to fail compilation ("Bad type 'JsonFileLoader'") once enough mods are
// loaded. Routing every call through this adapter keeps the generic in its
// native Game module, where it compiles reliably.

class JobsModJsonFileIO
{
	static bool LoadConfig(string filename, out JobsModConfigJson data, out string errorMessage)
	{
		return JsonFileLoader<JobsModConfigJson>.LoadFile(filename, data, errorMessage);
	}

	static bool SaveConfig(string filename, JobsModConfigJson data, out string errorMessage)
	{
		return JsonFileLoader<JobsModConfigJson>.SaveFile(filename, data, errorMessage);
	}
}
