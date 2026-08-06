// PlayerBaseJobsModClient.c
//
// Client-side receiver for the mod's server-to-client messages, and the place
// the sorting action is registered.
//
// This file lives in the client PBO, which the server also loads, so the
// handler explicitly refuses to run its client half on a dedicated server.

modded class PlayerBase
{
	override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		super.OnRPC(sender, rpc_type, ctx);

		// Ignore everything outside the mod's own id block, so vanilla traffic
		// is never touched and an unrelated id can never reach the parser below.
		if (rpc_type < JobsModRPC.BASE || rpc_type > JobsModRPC.BASE + 100)
			return;

		// On a listen server both halves run in one process; on a dedicated
		// server there is no client half at all.
		if (GetGame().IsMultiplayer() && !GetGame().IsClient())
			return;

		if (rpc_type == JobsModRPC.NOTIFY_SORTING_SESSION)
		{
			JobsModClientContext.HandleSessionGranted(ctx);
			return;
		}

		if (rpc_type == JobsModRPC.NOTIFY_SORTING_REJECTED)
		{
			JobsModClientContext.HandleRejected(ctx);
			return;
		}

		if (rpc_type == JobsModRPC.NOTIFY_SORTING_ACCEPTED)
		{
			JobsModClientContext.HandleAccepted(ctx);
			return;
		}
	}
}
