// PlayerBaseCacheModServer.c
//
// The server's entry point for client messages.
//
// Only two arrive, and both are administrator requests. The search does not
// appear here: it travels through the engine's own action pipeline, where the
// server half of ActionSearchCache runs on the server by construction and the
// client cannot forge a completion it did not play out.
//
// Both handlers do the same three things in the same order, and the order is
// the point: identify the sender from the engine's own PlayerIdentity, check
// the permission, and only then read what the message said.

modded class PlayerBase
{
	override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		super.OnRPC(sender, rpc_type, ctx);

		if (!GetGame().IsServer())
			return;

		if (rpc_type < CacheModRPC.BASE)
			return;

		if (rpc_type > CacheModRPC.BASE + CacheModRPC.ID_RANGE)
			return;

		if (!CacheModServerRuntime.IsStarted())
			return;

		// The identity comes from the connection, not from the payload. A
		// message with no identity behind it is one the engine could not
		// attribute, and nothing in this mod acts on an unattributable request.
		if (!sender)
			return;

		if (rpc_type == CacheModRPC.REQUEST_CREATE_CACHE)
		{
			HandleCacheModCreateRequest(sender, ctx);
			return;
		}

		if (rpc_type == CacheModRPC.REQUEST_ADMIN_POINTS)
		{
			HandleCacheModAdminPointsRequest(sender, ctx);
			return;
		}
	}

	protected void HandleCacheModCreateRequest(PlayerIdentity sender, ParamsReadContext ctx)
	{
		Param3<vector, float, int> data = new Param3<vector, float, int>(vector.Zero, 0, 0);
		if (!ctx.Read(data))
			return;

		if (!CacheModAuthority.IsAdmin(sender))
		{
			CacheLog.Warning(CacheLog.PLACEMENT, "Размещение отклонено: у " + sender.GetPlainId() + " нет прав.");
			CacheModServerRuntime.SendMessage(this, CacheModReject.GetText(CacheModReject.NOT_ADMIN));
			return;
		}

		vector position = data.param1;
		float radius = data.param2;
		int cacheType = data.param3;

		CachePlacementBridge.RequestPlacement(this, position, radius, cacheType);
	}

	protected void HandleCacheModAdminPointsRequest(PlayerIdentity sender, ParamsReadContext ctx)
	{
		Param1<int> data = new Param1<int>(0);
		if (!ctx.Read(data))
			return;

		if (!CacheModAuthority.IsAdmin(sender))
		{
			CacheLog.Warning(CacheLog.ADMIN, "Список точек не выдан: у " + sender.GetPlainId() + " нет прав.");
			return;
		}

		CacheModServerRuntime.SendAdminPoints(this);
	}
}
