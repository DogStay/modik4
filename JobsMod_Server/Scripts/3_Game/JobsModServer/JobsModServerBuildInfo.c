// JobsModServerBuildInfo.c
//
// The server PBO's own build stamp, kept deliberately separate from
// JobsModBuildInfo — that one ships inside JobsMod_Client.
//
// The two PBOs are one mod but two deployable artifacts, and nothing about
// copying them is atomic: an admin can rebuild one and not the other, or copy
// one while the server is already starting. The result is a mixed build, and a
// mixed build fails in whatever way the two halves happen to disagree — a
// missing constant at compile time if you are lucky, a protocol that reads the
// wrong fields at runtime if you are not. Neither says "you deployed half the
// mod".
//
// So both halves carry a stamp and the runtime compares them at startup. The
// two constants must be bumped together, in the same commit, on every push that
// has to be verified on a live server.

class JobsModServerBuildInfo
{
	static const string BUILD = "2.3.0-courier-b10";
}
