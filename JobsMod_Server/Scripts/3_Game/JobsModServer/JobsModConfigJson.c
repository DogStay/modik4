// JobsModConfigJson.c
//
// Plain data classes mirroring the on-disk server config. They carry no logic
// beyond reading their own coordinate strings: JsonFileLoader fills the fields
// by name, so a field rename here silently changes the config format and must be
// treated as a breaking change.
//
// The config is three things, and the split follows how often each is edited:
//
//   $profile:JobsMod/settings.json     the world: switches, zones, trash points
//                                      and freight routes, all in one file
//   $profile:JobsMod/Jobs/<id>.json    one file per job
//   $profile:JobsMod/NPC/<id>.json     one file per employer
//
// Jobs and NPCs are the two things an admin adds and removes as separate
// decisions, so they stay one file each: dropping a file in adds an NPC,
// deleting it removes them, and the file name is the id everything else refers
// to. Everything a job or an NPC merely points at — a zone, a trash point, a
// freight route — is a line in the settings file, because those are edited
// together and a folder of three-line files is more to manage than it is worth.
//
// Positions are written as one string rather than as x/y/z fields, in whatever
// shape #position printed them. See JobsModCoords for what is accepted.

// One region. It carries no geometry at all: it exists to tie trash points,
// jobs and freight routes together and to give the player a name to read on the
// HUD.
class JobsModZoneJson
{
	string id;
	string name;
}

// An exact spot for one trash pile. Exact rather than random: a pile rolled
// inside a radius eventually lands inside a wall or under a floor, and nobody
// can work it. The admin walks to the spot, reads the coordinates and pastes
// them once.
class JobsModPilePointJson
{
	string id;
	string zone_id;
	string name;

	// "6600 300 2500". A height of 0 means "put it on the terrain".
	string position;

	vector GetPosition()
	{
		vector parsed;
		JobsModCoords.Parse(position, parsed);
		return parsed;
	}
}

// A freight route: a circle where the boxes appear and a circle they have to end
// up in.
class JobsModLoaderAreaJson
{
	string id;
	string name;
	string zone_id;

	string source;
	float source_radius;

	string destination;
	float destination_radius;

	vector GetSource()
	{
		vector parsed;
		JobsModCoords.Parse(source, parsed);
		return parsed;
	}

	vector GetDestination()
	{
		vector parsed;
		JobsModCoords.Parse(destination, parsed);
		return parsed;
	}
}

// A post a guard has to hold. A circle, like a freight yard: the player counts
// as on duty while inside it and the clock stops the moment they step out.
class JobsModGuardPostJson
{
	string id;
	string name;
	string zone_id;

	string position;
	float radius;

	vector GetPosition()
	{
		vector parsed;
		JobsModCoords.Parse(position, parsed);
		return parsed;
	}
}

// settings.json in full: the switches plus everything jobs and NPCs point at.
class JobsModSettingsJson
{
	// Seconds before a sorted pile comes back at its own point.
	int pile_respawn_seconds;

	// Seconds a player may hold an unfinished job before the server drops it
	// and cleans up after it. Without a bound, freight spawned for a player who
	// wandered off would stay in the world for the rest of the uptime.
	int assignment_timeout_seconds;

	bool debug_logging;

	ref array<ref JobsModZoneJson> zones;
	ref array<ref JobsModPilePointJson> pile_points;
	ref array<ref JobsModLoaderAreaJson> loader_areas;
	ref array<ref JobsModGuardPostJson> guard_posts;
}

// One job an NPC can hand out.
class JobsModJobJson
{
	string id;
	string name;
	string description;

	// "sorting", "loading" or "messenger" — see JobsModJobType.
	string type;

	string zone_id;

	int reward;

	// Seconds this player must wait before taking this same job again.
	int cooldown_seconds;

	// Sorting jobs only: how many piles to work through.
	int piles_required;

	// Loading jobs only: which route to use, how many boxes, and what class the
	// box is. The class is written out in full rather than picked from a preset
	// table, so any item in the game can be used as freight without a code
	// change.
	string loader_area_id;
	int cargos_required;
	string cargo_class;

	// Messenger jobs only: who the parcel is for, and what the parcel is.
	//
	// target_npc_id is the second person in the job. They do not have to offer
	// any work of their own — being named here is what puts them in the config
	// at all — and they are the one that takes the parcel and pays, not the
	// employer who handed it out.
	//
	// package_class is written out in full like cargo_class, but with a caveat:
	// only JobsMod_Parcel and classes inheriting from it refuse to be dropped.
	// Any other item works and is simply droppable.
	string target_npc_id;
	string package_class;

	// Guard jobs only: which post to hold, for how long, and what the employer
	// hands over for the shift.
	//
	// The kit belongs to the contract, not to the player: it is tracked with
	// the assignment and taken back when the job ends by any route, so a
	// player cannot take the same job repeatedly to collect batons.
	string guard_post_id;
	int guard_seconds;
	ref array<string> equipment;

	// Collect jobs only: what counts, how many, and what to call it on screen.
	//
	// A list rather than one class so that "any meat" is one contract instead of
	// one per animal. Counted as whole items — a food item's quantity in DayZ is
	// its weight in grams, and counting that would read as nine thousand steaks.
	ref array<string> collect_classes;
	int collect_required;
	string collect_label;
}

// An employer standing in the world.
class JobsModNpcJson
{
	string id;
	string name;
	string description;

	// "6620 300 2520". A height of 0 means "stand on the terrain".
	string position;

	// Compass heading in degrees the NPC faces.
	float rotation;

	// Any vanilla survivor class, e.g. "SurvivorM_Mirek" or "SurvivorF_Huska".
	string player_class;

	// Blocks all damage. Off would mean players can shoot the employer and the
	// job disappears until the next restart, so the default file ships it on.
	bool invulnerable;

	// Item classes to dress it in, applied top to bottom. Plain class names:
	// whatever the admin writes is what gets created.
	ref array<string> clothing;

	// Ids of the jobs this NPC hands out. May be empty, but only for an NPC that
	// some messenger job names as its recipient.
	ref array<string> jobs;

	vector GetPosition()
	{
		vector parsed;
		JobsModCoords.Parse(position, parsed);
		return parsed;
	}
}
