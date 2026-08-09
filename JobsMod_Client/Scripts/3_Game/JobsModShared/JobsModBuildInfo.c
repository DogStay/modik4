// JobsModBuildInfo.c
//
// Single source of truth for the build identity. The `version` field of both
// config.cpp files must be kept equal to VERSION — they are separate files the
// engine reads before any script runs, so they cannot read this constant.

class JobsModBuildInfo
{
	static const string VERSION = "2.3.0";
	static const string BUILD = "2.3.0-courier";
}
