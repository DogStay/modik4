// PlayerBaseCacheModClient.c
//
// Registers the mod's actions and receives the server's messages.
//
// This file is in the client PBO, which the server also loads, so the client
// half explicitly refuses to run on a dedicated server.

modded class PlayerBase
{
	override void SetActions(out TInputActionMap InputActionMap)
	{
		super.SetActions(InputActionMap);

		// The only action an ordinary player ever gets from this mod. It is
		// registered on the player rather than on an item because the target —
		// the interaction proxy — is a world object, not something in hand.
		AddAction(ActionSearchCache, InputActionMap);
	}

	override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		super.OnRPC(sender, rpc_type, ctx);

		if (GetGame().IsDedicatedServer())
			return;

		if (rpc_type < CacheModRPC.BASE)
			return;

		if (rpc_type > CacheModRPC.BASE + CacheModRPC.ID_RANGE)
			return;

		if (rpc_type == CacheModRPC.NOTIFY_MESSAGE)
		{
			CacheModClientContext.HandleMessage(ctx);
			return;
		}

		if (rpc_type == CacheModRPC.NOTIFY_SETTINGS)
		{
			CacheModClientContext.HandleSettings(ctx);
			return;
		}

		if (rpc_type == CacheModRPC.NOTIFY_ADMIN_POINTS)
		{
			CacheModClientContext.HandleAdminPoints(ctx);
			return;
		}
	}
}
