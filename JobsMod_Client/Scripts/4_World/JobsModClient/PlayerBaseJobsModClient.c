// PlayerBaseJobsModClient.c
//
// Client-side receiver for the mod's server-to-client messages, and the place
// the mod's user actions are registered.
//
// This file lives in the client PBO, which the server also loads, so the
// handler explicitly refuses to run its client half on a dedicated server.

modded class PlayerBase
{
	// True on the mod's employers, and replicated with the entity.
	//
	// A client cannot tell an employer from any other survivor: which classes
	// are NPCs lives in a server-only config that clients never download. That
	// leaves two ways to tell them, and only one of them holds up. Announcing it
	// on the side — a directory of coordinates sent once after connect — has to
	// arrive, has to arrive after the player entity exists, has to survive a
	// reconnect, and has to still match the NPC's position afterwards; when any
	// of that fails the action simply never appears, with nothing logged. A
	// synchronised flag travels with the entity, so it is right as soon as the
	// client can see the NPC at all, and there is no window where it is not.
	protected bool m_JobsModIsNpc;

	void PlayerBase()
	{
		RegisterNetSyncVariableBool("m_JobsModIsNpc");
	}

	// Server side only. Clients receive the value through synchronisation and
	// never set it; a client that lies to itself here gains a menu entry the
	// server rejects, which is what it was already free to do.
	void JobsModSetNpc(bool value)
	{
		m_JobsModIsNpc = value;
		SetSynchDirty();
	}

	bool JobsModIsNpc()
	{
		return m_JobsModIsNpc;
	}

	// Actions aimed at objects in the world. The trash pile is one, so sorting
	// belongs here — and only sorting.
	override void SetActions(out TInputActionMap InputActionMap)
	{
		super.SetActions(InputActionMap);
		AddAction(ActionSortTrash, InputActionMap);

		// Printed once per player init, on whichever side is running this code.
		// It is the only direct evidence that this PBO is loaded at all: without
		// it, every conclusion about why an action does not appear is a guess
		// about whether the class even exists on that machine.
		JobsLog.Info("ACTIONS: действия по объектам зарегистрированы; build=" + JobsModBuildInfo.BUILD + ".");
	}

	// Actions aimed at another PlayerBase come from this map and not from
	// SetActions — DayZ gathers them separately once the target is a survivor.
	// The employers are ordinary survivor entities, so the talk action is only
	// ever offered if it is registered here. Vanilla's own target actions
	// (check pulse, bandage, CPR) live in this map for the same reason, which is
	// why check pulse appeared on an NPC while this one never did.
	override void SetActionsRemoteTarget(out TInputActionMap InputActionMap)
	{
		super.SetActionsRemoteTarget(InputActionMap);
		AddAction(ActionTalkToNpc, InputActionMap);

		JobsLog.Info("ACTIONS: действия по выжившим зарегистрированы; build=" + JobsModBuildInfo.BUILD + ".");
	}

	override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		super.OnRPC(sender, rpc_type, ctx);

		// Ignore everything outside the mod's own id block, so vanilla traffic
		// is never touched and an unrelated id can never reach the parser below.
		if (rpc_type < JobsModRPC.BASE || rpc_type > JobsModRPC.BASE + JobsModRPC.ID_RANGE)
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

		if (rpc_type == JobsModRPC.NOTIFY_REJECTED)
		{
			JobsModClientContext.HandleRejected(ctx);
			return;
		}

		if (rpc_type == JobsModRPC.NOTIFY_SORTING_ACCEPTED)
		{
			JobsModClientContext.HandleAccepted(ctx);
			return;
		}

		if (rpc_type == JobsModRPC.NOTIFY_JOB_MENU)
		{
			JobsModClientContext.HandleJobMenu(ctx);
			return;
		}

		if (rpc_type == JobsModRPC.NOTIFY_JOB_STATE)
		{
			JobsModClientContext.HandleJobState(ctx);
			return;
		}

		if (rpc_type == JobsModRPC.NOTIFY_JOB_MESSAGE)
		{
			JobsModClientContext.HandleJobMessage(ctx);
			return;
		}
	}
}
