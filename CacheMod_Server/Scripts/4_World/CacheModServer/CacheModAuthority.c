// CacheModAuthority.c
//
// Who is allowed to place caches and to see the admin point list.
//
// The server half of the mod does not depend on VPP Admin Tools, and must not:
// a server that runs the caches without the admin menu is a supported setup,
// and naming a VPP type here would stop it compiling. So authority is answered
// in two independent ways, and either one is enough:
//
//   * a Steam64 whitelist in $profile:CacheMod/admins.json, which works with no
//     other mod loaded at all;
//   * a checker registered at runtime by CacheMod_Admin_Server, which asks VPP
//     whether the player holds the MenuCacheManager permission.
//
// Both sides are asked with a PlayerIdentity that the engine attached to the
// connection. A Steam64 that arrived inside a message is never consulted —
// that is the one number a client could simply make up.

class CacheModAuthority
{
	static const string FILE_ADMINS = CacheModConfig.ROOT_DIR + "/admins.json";

	protected static ref CacheModAdminsJson s_Admins;

	// Set by the admin server PBO during its own startup. Left null on a server
	// that does not load it, in which case the whitelist is the only answer.
	protected static ref ScriptInvoker s_ExternalCheck;

	static ScriptInvoker GetExternalCheck()
	{
		if (!s_ExternalCheck)
			s_ExternalCheck = new ScriptInvoker();

		return s_ExternalCheck;
	}

	static void Load()
	{
		if (!FileExist(FILE_ADMINS))
		{
			s_Admins = new CacheModAdminsJson();
			s_Admins.steam_ids = new array<string>();

			string writeError;
			CacheModJsonFileIO.SaveAdmins(FILE_ADMINS, s_Admins, writeError);

			CacheLog.Info(CacheLog.ADMIN, "admins.json создан пустым — впишите Steam64 администраторов.");
			return;
		}

		CacheModAdminsJson loaded;
		string error;
		if (!CacheModJsonFileIO.LoadAdmins(FILE_ADMINS, loaded, error))
		{
			CacheLog.Error(CacheLog.ADMIN, "admins.json не прочитан (" + error + "). Белый список пуст.");
			s_Admins = new CacheModAdminsJson();
			s_Admins.steam_ids = new array<string>();
			return;
		}

		s_Admins = loaded;

		if (!s_Admins.steam_ids)
			s_Admins.steam_ids = new array<string>();

		CacheLog.Info(CacheLog.ADMIN, "Белый список: " + s_Admins.steam_ids.Count().ToString() + " Steam64.");
	}

	// The one question the rest of the mod asks. Deny is the default: a server
	// with no whitelist and no VPP lets nobody place caches, which is the safe
	// way round for a tool that writes to the world.
	static bool IsAdmin(PlayerIdentity identity)
	{
		if (!identity)
			return false;

		string steamId = identity.GetPlainId();

		if (IsWhitelisted(steamId))
			return true;

		return AskExternalCheck(steamId);
	}

	protected static bool IsWhitelisted(string steamId)
	{
		if (!s_Admins || !s_Admins.steam_ids)
			return false;

		for (int i = 0; i < s_Admins.steam_ids.Count(); i++)
		{
			if (s_Admins.steam_ids.Get(i) == steamId)
				return true;
		}

		return false;
	}

	// ScriptInvoker has no return value, so the answer travels in a small box
	// the subscriber writes into. One subscriber is expected; several would
	// simply mean "any of them said yes", which is the right reading anyway.
	protected static bool AskExternalCheck(string steamId)
	{
		if (!s_ExternalCheck)
			return false;

		CacheModAuthorityQuery query = new CacheModAuthorityQuery();
		query.m_SteamId = steamId;
		query.m_Allowed = false;

		s_ExternalCheck.Invoke(query);

		return query.m_Allowed;
	}
}

class CacheModAuthorityQuery
{
	string m_SteamId;
	bool m_Allowed;
}
