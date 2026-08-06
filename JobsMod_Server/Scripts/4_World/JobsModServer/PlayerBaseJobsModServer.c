// PlayerBaseJobsModServer.c
//
// Server-side entry point for the mod's client requests.
//
// This must be modded in 4_World: PlayerBase does not exist while 5_Mission
// compiles. The services it routes to are reached through JobsModServerRuntime,
// which the mission bootstrap starts.

modded class PlayerBase
{
	override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		super.OnRPC(sender, rpc_type, ctx);

		if (rpc_type < JobsModRPC.BASE || rpc_type > JobsModRPC.BASE + 100)
			return;

		if (!GetGame().IsServer())
			return;

		// sender is attached by the engine, not by the client. Every check below
		// is made against it, never against anything read from the payload.
		if (!sender)
		{
			JobsLog.Warning("SERVER/RPC: запрос без identity отброшен; rpc=" + rpc_type.ToString() + ".");
			return;
		}

		SortingSessionService service = JobsModServerRuntime.GetSessionService();
		if (!service)
		{
			JobsLog.Error("SERVER/RPC: службы не запущены; rpc=" + rpc_type.ToString() + ".");
			return;
		}

		if (rpc_type == JobsModRPC.REQUEST_SORTING_SUBMIT)
		{
			service.HandleSubmit(this, sender, ctx);
			return;
		}

		if (rpc_type == JobsModRPC.REQUEST_SORTING_ABORT)
		{
			service.HandleAbort(this, sender, ctx);
			return;
		}
	}
}
