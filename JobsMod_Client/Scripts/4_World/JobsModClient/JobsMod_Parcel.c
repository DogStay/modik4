// JobsMod_Parcel.c
//
// The courier's parcel: an inventory item that goes where the player goes and
// nowhere else.
//
// The job is "carry this across the map", so the parcel has to be something the
// player cannot put down, cannot hand to a friend, and cannot stash in a car
// and drive after. Everything below exists to make those three impossible.
//
// Two layers, and it is worth being clear about which one is which:
//
//   * The overrides here are the inventory rules. They run on both client and
//     server, so the drag is refused in the UI and refused again by the server
//     that validates it. This is what the player actually experiences.
//
//   * JobsModMessengerService on the server sweeps every online player and
//     deletes any parcel that is not the one their current job handed them.
//     That is the backstop for whatever gets past the rules — a mod that moves
//     items directly, an admin tool, a parcel that survived into a saved
//     character after a crash.
//
// Neither layer is redundant. The rules alone would leave a parcel behind on
// every disconnect; the sweep alone would let a player drop the parcel and
// carry on for up to fifteen seconds as if nothing happened.
//
// This class ships in the client PBO because every player has to have it —
// an item class the client does not carry cannot be shown in their inventory.
// It therefore knows nothing about jobs: it cannot name a server type, and it
// must behave the same whether the server mod is loaded or not.

class JobsMod_Parcel extends ItemBase
{
	// No actions at all: not "take", not "drop", not the parent's own use
	// action. super is deliberately not called — calling it is what would add
	// them back, and there is nothing here a player is meant to do besides
	// walk.
	override void SetActions()
	{
	}

	// Into a container only while that container hangs off a player. This is
	// what lets the server create the parcel in the player's inventory, and in
	// a worn backpack, while refusing a tent, a car boot or a barrel.
	override bool CanPutInCargo(EntityAI parent)
	{
		if (!super.CanPutInCargo(parent))
			return false;

		return parent != null && parent.GetHierarchyRootPlayer() != null;
	}

	// Once it is somewhere, it stays there. This is the rule that blocks
	// dropping it, throwing it away, and moving it between containers, and it
	// is why "cannot be dropped" does not need an action to be blocked.
	//
	// It does not block the server deleting the parcel: ObjectDelete does not
	// go through the inventory at all.
	override bool CanRemoveFromCargo(EntityAI parent)
	{
		return false;
	}

	// Hands are how an item gets thrown, planted, and used. The parcel never
	// goes there, so none of that has a path to begin with.
	override bool CanPutIntoHands(EntityAI parent)
	{
		return false;
	}

	override bool CanRemoveFromHands(EntityAI parent)
	{
		return false;
	}

	override bool CanDetachAttachment(EntityAI parent)
	{
		return false;
	}

	// A parcel that somehow ends up on the ground is rubbish, not loot: the
	// job it belonged to is already lost. Refusing to pick it up keeps it from
	// turning into a second parcel in someone else's inventory before the
	// server's sweep gets to it.
	override bool IsTakeable()
	{
		return false;
	}

	override bool CanBeSplit()
	{
		return false;
	}
}
