// JobsModJsonFileIO.c
//
// A narrow adapter over JsonFileLoader, kept in 3_Game on purpose.
//
// Instantiating the generic loader directly from a 4_World class has been seen
// to fail compilation ("Bad type 'JsonFileLoader'") once enough mods are
// loaded. Routing every call through this adapter keeps the generic in its
// native Game module, where it compiles reliably.
//
// There is one pair of calls per config type rather than a generic helper,
// because the generic parameter has to be a concrete type at the call site for
// exactly the reason above.

class JobsModJsonFileIO
{
	static bool LoadSettings(string filename, out JobsModSettingsJson data, out string errorMessage)
	{
		return JsonFileLoader<JobsModSettingsJson>.LoadFile(filename, data, errorMessage);
	}

	static bool SaveSettings(string filename, JobsModSettingsJson data, out string errorMessage)
	{
		return JsonFileLoader<JobsModSettingsJson>.SaveFile(filename, data, errorMessage);
	}

	static bool LoadZoneList(string filename, out JobsModZoneListJson data, out string errorMessage)
	{
		return JsonFileLoader<JobsModZoneListJson>.LoadFile(filename, data, errorMessage);
	}

	static bool SaveZoneList(string filename, JobsModZoneListJson data, out string errorMessage)
	{
		return JsonFileLoader<JobsModZoneListJson>.SaveFile(filename, data, errorMessage);
	}

	static bool LoadPilePoint(string filename, out JobsModPilePointJson data, out string errorMessage)
	{
		return JsonFileLoader<JobsModPilePointJson>.LoadFile(filename, data, errorMessage);
	}

	static bool SavePilePoint(string filename, JobsModPilePointJson data, out string errorMessage)
	{
		return JsonFileLoader<JobsModPilePointJson>.SaveFile(filename, data, errorMessage);
	}

	static bool LoadLoaderArea(string filename, out JobsModLoaderAreaJson data, out string errorMessage)
	{
		return JsonFileLoader<JobsModLoaderAreaJson>.LoadFile(filename, data, errorMessage);
	}

	static bool SaveLoaderArea(string filename, JobsModLoaderAreaJson data, out string errorMessage)
	{
		return JsonFileLoader<JobsModLoaderAreaJson>.SaveFile(filename, data, errorMessage);
	}

	static bool LoadJob(string filename, out JobsModJobJson data, out string errorMessage)
	{
		return JsonFileLoader<JobsModJobJson>.LoadFile(filename, data, errorMessage);
	}

	static bool SaveJob(string filename, JobsModJobJson data, out string errorMessage)
	{
		return JsonFileLoader<JobsModJobJson>.SaveFile(filename, data, errorMessage);
	}

	static bool LoadNpc(string filename, out JobsModNpcJson data, out string errorMessage)
	{
		return JsonFileLoader<JobsModNpcJson>.LoadFile(filename, data, errorMessage);
	}

	static bool SaveNpc(string filename, JobsModNpcJson data, out string errorMessage)
	{
		return JsonFileLoader<JobsModNpcJson>.SaveFile(filename, data, errorMessage);
	}

	// Lists the *.json files of one config folder. The names come back bare
	// ("npc_sorter_01.json"), without the folder, which is what the caller wants
	// both for opening the file and for checking it against the id inside.
	static void ListJsonFiles(string directory, out array<string> fileNames)
	{
		fileNames = new array<string>();

		string name;
		FileAttr attributes;

		FindFileHandle handle = FindFile(directory + "/*.json", name, attributes, FindFileFlags.ALL);
		if (!handle)
			return;

		// FindFile already yields the first match; FindNextFile continues from
		// there. A leading empty name means the folder held nothing.
		if (name != "")
			fileNames.Insert(name);

		while (FindNextFile(handle, name, attributes))
		{
			if (name != "")
				fileNames.Insert(name);
		}

		CloseFindFile(handle);
	}
}
