// CacheModAdminPlugin.c
//
// The server backend of the Cache Manager, and the bridge between VPP's
// permission system and the mod's own authority check.
//
// It never edits JSON behind the manager's back. Every operation goes through
// CacheModServerRuntime, which owns the runtime state — a cache edited straight
// in the file while the server runs would be overwritten by the next save, and
// a cache deleted that way would leave its proxy standing in the world.
//
// Every entry point re-checks the permission it needs, with the identity the
// engine attached to the connection. Checking once when the menu opens would
// mean a client that got the menu open keeps the rights it had then.

class CacheModAdminPlugin extends PluginBase
{
	protected static const string PERMISSION_MENU = "MenuCacheManager";
	protected static const string PERMISSION_VIEW = "MenuCacheManager:View";
	protected static const string PERMISSION_EDIT = "MenuCacheManager:Edit";
	protected static const string PERMISSION_CREATE = "MenuCacheManager:Create";
	protected static const string PERMISSION_DELETE = "MenuCacheManager:Delete";
	protected static const string PERMISSION_RESET = "MenuCacheManager:Reset";
	protected static const string PERMISSION_RELOAD = "MenuCacheManager:Reload";

	// The CF RPC channel the VPP menus travel on.
	protected static const int CACHEMOD_CF_RPC_ID = 10042;

	void CacheModAdminPlugin()
	{
		PermissionManager permissionManager = GetPermissionManager();
		if (permissionManager)
		{
			ref array<string> permissions = new array<string>;
			permissions.Insert(PERMISSION_MENU);
			permissions.Insert(PERMISSION_VIEW);
			permissions.Insert(PERMISSION_EDIT);
			permissions.Insert(PERMISSION_CREATE);
			permissions.Insert(PERMISSION_DELETE);
			permissions.Insert(PERMISSION_RESET);
			permissions.Insert(PERMISSION_RELOAD);
			permissionManager.AddPermissionType(permissions);
		}

		GetRPCManager().AddRPC("RPC_CacheModAdmin", "GetPoints", this, SingeplayerExecutionType.Server);
		GetRPCManager().AddRPC("RPC_CacheModAdmin", "SaveCache", this, SingeplayerExecutionType.Server);
		GetRPCManager().AddRPC("RPC_CacheModAdmin", "DeleteCache", this, SingeplayerExecutionType.Server);
		GetRPCManager().AddRPC("RPC_CacheModAdmin", "ReloadStatic", this, SingeplayerExecutionType.Server);
		GetRPCManager().AddRPC("RPC_CacheModAdmin", "TeleportToCache", this, SingeplayerExecutionType.Server);

		// This is what lets an administrator place caches with the book without
		// being written into admins.json: the core mod asks, and this answers
		// out of VPP. Both routes stay open, because a server may run either.
		CacheModAuthority.GetExternalCheck().Insert(OnAuthorityQuery);

		CacheLog.Info(CacheLog.ADMIN, "VPP-плагин Cache Manager зарегистрирован.");
	}

	void ~CacheModAdminPlugin()
	{
		CacheModAuthority.GetExternalCheck().Remove(OnAuthorityQuery);
	}

	// =====================================================================
	// Authority
	// =====================================================================
	// Answers the core mod's question. Holding the menu permission is what
	// makes someone an administrator as far as placement is concerned — there
	// is no separate "may use the book" right to keep in step with this one.
	void OnAuthorityQuery(CacheModAuthorityQuery query)
	{
		if (!query)
			return;

		PermissionManager manager = GetPermissionManager();
		if (!manager)
			return;

		if (manager.VerifyPermission(query.m_SteamId, PERMISSION_MENU, "", false))
			query.m_Allowed = true;
	}

	// =====================================================================
	// Reading
	// =====================================================================
	void GetPoints(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server)
			return;

		if (!HasPermission(sender, PERMISSION_VIEW))
			return;

		if (!CacheModServerRuntime.IsStarted())
			return;

		string packed = CacheModServerRuntime.GetManager().BuildAdminPointList();
		SendToClient(sender, "OnPoints", new Param1<string>(packed));
	}

	// =====================================================================
	// Writing
	// =====================================================================
	// One cache, all its editable fields at once. Radius and tool take effect
	// immediately — the proxy is rebuilt — while a changed spawn chance is
	// saved and applies from the next restart, because this run's active set
	// was decided when the server started.
	void SaveCache(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server)
			return;

		Param4<string, float, int, string> data = new Param4<string, float, int, string>("", 0, 0, "");
		if (!ctx.Read(data))
			return;

		if (!HasPermission(sender, PERMISSION_EDIT))
		{
			SendMessage(sender, "Нет прав на изменение тайников.");
			return;
		}

		if (!CacheModServerRuntime.IsStarted())
			return;

		// The spawn chance rides in the tool field's tail rather than in a
		// fifth Param slot: Param takes at most four, and packing one string is
		// less fragile than splitting the operation into two messages that
		// could arrive apart.
		string cacheId = data.param1;
		float radius = data.param2;
		int cacheType = data.param3;

		array<string> parts = new array<string>();
		data.param4.Split(CacheModRPC.FIELD_SEPARATOR, parts);

		string requiredTool = "";
		float spawnChance = 0.0;

		if (parts.Count() > 0)
			requiredTool = parts.Get(0);

		if (parts.Count() > 1)
			spawnChance = parts.Get(1).ToFloat();

		CacheModManager manager = CacheModServerRuntime.GetManager();
		if (!manager.UpdateCache(cacheId, radius, cacheType, requiredTool, spawnChance))
		{
			SendMessage(sender, "Тайник " + cacheId + " не найден.");
			return;
		}

		CacheLog.Info(CacheLog.ADMIN, sender.GetPlainId() + " изменил тайник " + cacheId + ".");
		SendMessage(sender, "Тайник " + cacheId + " сохранён.");
		SendPointsTo(sender);
	}

	void DeleteCache(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server)
			return;

		Param1<string> data = new Param1<string>("");
		if (!ctx.Read(data))
			return;

		if (!HasPermission(sender, PERMISSION_DELETE))
		{
			SendMessage(sender, "Нет прав на удаление тайников.");
			return;
		}

		if (!CacheModServerRuntime.IsStarted())
			return;

		if (!CacheModServerRuntime.GetManager().DeleteCache(data.param1))
		{
			SendMessage(sender, "Тайник " + data.param1 + " не найден.");
			return;
		}

		CacheLog.Info(CacheLog.ADMIN, sender.GetPlainId() + " удалил тайник " + data.param1 + ".");
		SendMessage(sender, "Тайник " + data.param1 + " удалён.");
		SendPointsTo(sender);
	}

	// Re-reads zones, tiers and settings from disk. Caches are deliberately not
	// re-read: their timers and this run's active set live in memory only.
	void ReloadStatic(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server)
			return;

		if (!HasPermission(sender, PERMISSION_RELOAD))
		{
			SendMessage(sender, "Нет прав на перезагрузку конфигурации.");
			return;
		}

		if (!CacheModServerRuntime.ReloadStaticData())
		{
			SendMessage(sender, "Перезагрузка не выполнена.");
			return;
		}

		CacheLog.Info(CacheLog.ADMIN, sender.GetPlainId() + " перечитал зоны, тиры и настройки.");
		SendMessage(sender, "Зоны, тиры и настройки перечитаны.");
	}

	// Walking to a point to check it is otherwise the slowest part of running
	// this mod. Placed behind the view permission because it moves the sender
	// and nobody else.
	void TeleportToCache(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server)
			return;

		Param1<string> data = new Param1<string>("");
		if (!ctx.Read(data))
			return;

		if (!HasPermission(sender, PERMISSION_VIEW))
			return;

		if (!CacheModServerRuntime.IsStarted())
			return;

		CacheModCacheJson cache = CacheModServerRuntime.GetConfig().FindCache(data.param1);
		if (!cache)
		{
			SendMessage(sender, "Тайник " + data.param1 + " не найден.");
			return;
		}

		PlayerBase player = PlayerBase.Cast(sender.GetPlayer());
		if (!player)
			return;

		vector position = cache.GetPosition();
		player.SetPosition(position);

		CacheLog.Info(CacheLog.ADMIN, sender.GetPlainId() + " телепортирован к " + data.param1 + ".");
	}

	// =====================================================================
	// Plumbing
	// =====================================================================
	protected bool HasPermission(PlayerIdentity sender, string permission)
	{
		if (!sender)
			return false;

		PermissionManager manager = GetPermissionManager();
		if (!manager)
			return false;

		return manager.VerifyPermission(sender.GetPlainId(), permission, "", false);
	}

	protected void SendPointsTo(PlayerIdentity sender)
	{
		if (!CacheModServerRuntime.IsStarted())
			return;

		string packed = CacheModServerRuntime.GetManager().BuildAdminPointList();
		SendToClient(sender, "OnPoints", new Param1<string>(packed));
	}

	protected void SendMessage(PlayerIdentity sender, string text)
	{
		SendToClient(sender, "OnAdminMessage", new Param1<string>(text));
	}

	protected void SendToClient(PlayerIdentity recipient, string functionName, Param payload)
	{
		if (!recipient)
			return;

		array<ref Param> rpcData = new array<ref Param>;
		rpcData.Insert(new Param2<string, string>("RPC_CacheModAdminClient", functionName));

		if (payload)
			rpcData.Insert(payload);

		GetGame().RPC(NULL, CACHEMOD_CF_RPC_ID, rpcData, true, recipient);
	}
}
