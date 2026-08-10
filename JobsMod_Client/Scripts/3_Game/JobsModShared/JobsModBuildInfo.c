// JobsModBuildInfo.c
//
// Single source of truth for the build identity. The `version` field of both
// config.cpp files must be kept equal to VERSION — they are separate files the
// engine reads before any script runs, so they cannot read this constant.

class JobsModBuildInfo
{
	static const string VERSION = "2.3.0";
	// Bumped on every push that has to be verified on a live server. VERSION is
	// tied to config.cpp and stays put; this one is free-form on purpose, so the
	// startup line answers "is the running build actually the one I just
	// deployed?" without guessing from line numbers in a compile error.
	static const string BUILD = "2.3.0-courier-b13";
}
