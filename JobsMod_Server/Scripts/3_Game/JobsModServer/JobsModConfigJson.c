// JobsModConfigJson.c
//
// Plain data classes mirroring the on-disk server config. They carry no logic:
// JsonFileLoader fills the fields by name, so a field rename here silently
// changes the config format and must be treated as a breaking change.
//
// Height is deliberately absent. A pile is always placed on the terrain surface
// under its x/z, which keeps a hand-edited config from burying piles under the
// ground or floating them in the air.

class JobsModZoneJson
{
	string name;
	float x;
	float z;
	float radius;
	int piles;
}

class JobsModConfigJson
{
	// Seconds a player must wait between finished shifts. Without it, a player
	// could walk a circuit of piles and mint tokens without limit.
	int player_cooldown_seconds;

	// Seconds before a sorted pile comes back, at a new spot inside its zone.
	int pile_respawn_seconds;

	bool debug_logging;

	ref array<ref JobsModZoneJson> zones;
}
