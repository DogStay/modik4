// CacheModJsonFileIO.c
//
// A narrow adapter over JsonFileLoader, kept in 3_Game deliberately.
//
// Instantiating the generic loader directly from a 4_World class has been seen
// to fail compilation ("Bad type 'JsonFileLoader'") once enough mods are
// loaded. Routing every call through this adapter keeps the generic in its
// native Game module, where it compiles reliably.
//
// There is one pair of calls per file type rather than one generic helper, for
// exactly that reason: the generic parameter has to be a concrete type at the
// call site.

class CacheModJsonFileIO
{
	static bool LoadSettings(string filename, out CacheModSettingsJson data, out string errorMessage)
	{
		return JsonFileLoader<CacheModSettingsJson>.LoadFile(filename, data, errorMessage);
	}

	static bool SaveSettings(string filename, CacheModSettingsJson data, out string errorMessage)
	{
		return JsonFileLoader<CacheModSettingsJson>.SaveFile(filename, data, errorMessage);
	}

	static bool LoadCaches(string filename, out CacheModCachesFileJson data, out string errorMessage)
	{
		return JsonFileLoader<CacheModCachesFileJson>.LoadFile(filename, data, errorMessage);
	}

	static bool SaveCaches(string filename, CacheModCachesFileJson data, out string errorMessage)
	{
		return JsonFileLoader<CacheModCachesFileJson>.SaveFile(filename, data, errorMessage);
	}

	static bool LoadZones(string filename, out CacheModZonesFileJson data, out string errorMessage)
	{
		return JsonFileLoader<CacheModZonesFileJson>.LoadFile(filename, data, errorMessage);
	}

	static bool SaveZones(string filename, CacheModZonesFileJson data, out string errorMessage)
	{
		return JsonFileLoader<CacheModZonesFileJson>.SaveFile(filename, data, errorMessage);
	}

	static bool LoadAdmins(string filename, out CacheModAdminsJson data, out string errorMessage)
	{
		return JsonFileLoader<CacheModAdminsJson>.LoadFile(filename, data, errorMessage);
	}

	static bool SaveAdmins(string filename, CacheModAdminsJson data, out string errorMessage)
	{
		return JsonFileLoader<CacheModAdminsJson>.SaveFile(filename, data, errorMessage);
	}

	static bool LoadTiers(string filename, out CacheModTiersFileJson data, out string errorMessage)
	{
		return JsonFileLoader<CacheModTiersFileJson>.LoadFile(filename, data, errorMessage);
	}

	static bool SaveTiers(string filename, CacheModTiersFileJson data, out string errorMessage)
	{
		return JsonFileLoader<CacheModTiersFileJson>.SaveFile(filename, data, errorMessage);
	}
}
