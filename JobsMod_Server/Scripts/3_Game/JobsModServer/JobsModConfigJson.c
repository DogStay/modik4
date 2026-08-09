// JobsModConfigJson.c
//
// Plain data classes mirroring the on-disk server config. They carry no logic:
// JsonFileLoader fills the fields by name, so a field rename here silently
// changes the config format and must be treated as a breaking change.
//
// The config is a tree of small files rather than one big document:
//
//   $profile:JobsMod/settings.json          global switches
//   $profile:JobsMod/Zones/zones.json       the list of regions
//   $profile:JobsMod/PilePoints/<id>.json   one file per trash pile
//   $profile:JobsMod/LoaderAreas/<id>.json  one file per freight route
//   $profile:JobsMod/Jobs/<id>.json         one file per job
//   $profile:JobsMod/NPC/<id>.json          one file per employer
//
// Everything in the per-entity folders is found by scanning for *.json, so an
// admin adds an NPC by dropping in a file and removes one by deleting it. The
// "id" field inside the file is what the rest of the config refers to; the file
// name is expected to match it and a mismatch is reported at load.

// A point in the world. Height is loaded but treated as advisory: a value of 0
// or below means "put it on the terrain", which is what a config copied from a
// map without heights needs.
class JobsModVectorJson
{
	float x;
	float y;
	float z;
}

// A circle on the map, used for the freight pick-up and drop-off areas.
class JobsModAreaJson
{
	float x;
	float z;
	float radius;
}

// Global switches. Everything that is per-job lives in the job file instead.
class JobsModSettingsJson
{
	// Seconds before a sorted pile comes back at its own point.
	int pile_respawn_seconds;

	// Seconds a player may hold an unfinished job before the server drops it
	// and cleans up after it. Without a bound, freight spawned for a player who
	// wandered off would stay in the world for the rest of the uptime.
	int assignment_timeout_seconds;

	bool debug_logging;
}

// A region. It carries no geometry at all: it exists to tie piles, jobs and
// freight routes together and to give the player a name to read on the HUD.
class JobsModZoneJson
{
	string id;
	string name;
}

// zones.json holds the whole list, because zones are one line each and a folder
// of one-line files would be more to manage than it is worth.
class JobsModZoneListJson
{
	ref array<ref JobsModZoneJson> zones;
}

// An exact spot for one trash pile. Exact rather than random: a pile rolled
// inside a radius eventually lands inside a wall or under a floor, and nobody
// can work it. The admin walks to the spot, reads the coordinates and writes
// them down once.
class JobsModPilePointJson
{
	string id;
	string zone_id;
	string name;
	float x;
	float y;
	float z;
}

// A freight route: where the boxes appear and where they have to end up.
class JobsModLoaderAreaJson
{
	string id;
	string name;
	string zone_id;
	ref JobsModAreaJson source;
	ref JobsModAreaJson destination;
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
}

// An employer standing in the world.
class JobsModNpcJson
{
	string id;
	string name;
	string description;

	ref JobsModVectorJson position;

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

	// Ids of the jobs this NPC hands out.
	ref array<string> jobs;
}
