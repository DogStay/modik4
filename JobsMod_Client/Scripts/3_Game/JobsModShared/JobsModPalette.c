// JobsModPalette.c
//
// The palette from DESIGN_REFERENCE/TRASH_SORTING_FINAL/README.md, in one place.
//
// The .layout files carry the same colours as float components; this class
// carries the ones the script has to apply at runtime (state changes: hover,
// correct, wrong, filled, disabled). Keeping both from one written-down source
// is what stops the static markup and the live states from drifting apart.
//
// IMPORTANT: Enfusion colours are ARGB — alpha first, not last. ARGB() takes
// (alpha, red, green, blue) as 0..255 ints. A colour copied from a web palette
// as #RRGGBB must have its alpha moved to the front, which is what every
// constant below already does.

class JobsModPalette
{
	// --- Surfaces ---
	static int MainPanel()      { return ARGB(255, 22, 24, 26); }   // #16181A
	static int Header()         { return ARGB(255, 28, 30, 28); }   // #1C1E1C
	static int Instruction()    { return ARGB(255, 19, 21, 19); }   // #131513
	static int WorkArea()       { return ARGB(255, 29, 32, 31); }   // #1D201F
	static int BinArea()        { return ARGB(255, 32, 36, 26); }   // #20241A
	static int Bin()            { return ARGB(255, 26, 28, 25); }   // #1A1C19
	static int Card()           { return ARGB(255, 19, 21, 19); }   // #131513
	static int Footer()         { return ARGB(255, 24, 26, 24); }   // #181A18
	static int Button()         { return ARGB(255, 41, 45, 40); }   // #292D28
	static int ButtonHover()    { return ARGB(255, 53, 59, 50); }   // #353B32
	static int WorldDim()       { return ARGB(219, 6, 7, 5); }      // rgba(6,7,5,0.86)

	// --- Borders ---
	static int BorderDim()      { return ARGB(255, 48, 53, 47); }   // #30352F
	static int BorderBase()     { return ARGB(255, 56, 61, 56); }   // #383D38
	static int BorderBright()   { return ARGB(255, 83, 90, 80); }   // #535A50
	static int BorderFilled()   { return ARGB(255, 117, 134, 83); } // #758653

	// --- Text ---
	static int TextPrimary()    { return ARGB(255, 237, 239, 234); } // #EDEFEA
	static int TextSecondary()  { return ARGB(255, 164, 169, 161); } // #A4A9A1
	static int TextMuted()      { return ARGB(255, 115, 121, 113); } // #737971
	static int TextFaint()      { return ARGB(255, 85, 90, 84); }    // #555A54

	// --- Accents ---
	// Olive marks progress and every correct action; red marks only a wrong
	// sort. The design forbids using either for anything else.
	static int Accent()         { return ARGB(255, 163, 184, 119); } // #A3B877
	static int AccentSoft()     { return ARGB(31, 163, 184, 119); }  // rgba(163,184,119,0.12)
	static int AccentFlash()    { return ARGB(51, 163, 184, 119); }  // rgba(163,184,119,0.20)
	static int Danger()         { return ARGB(255, 169, 93, 93); }   // #A95D5D
	static int DangerFlash()    { return ARGB(46, 169, 93, 93); }    // rgba(169,93,93,0.18)

	// --- Bin marker stripes ---
	static int MarkerWood()     { return ARGB(255, 118, 100, 78); }  // #76644E
	static int MarkerPlastic()  { return ARGB(255, 86, 105, 120); }  // #566978
	static int MarkerMetal()    { return ARGB(255, 102, 107, 104); } // #666B68

	static int GetMarkerForBin(string binId)
	{
		if (binId == JobsModTrashBin.WOOD)
			return MarkerWood();
		if (binId == JobsModTrashBin.PLASTIC)
			return MarkerPlastic();
		if (binId == JobsModTrashBin.METAL)
			return MarkerMetal();

		return BorderBase();
	}

	// Alpha the design assigns to a card that is being dragged / already sorted.
	static const float ALPHA_DRAGGING = 0.35;
	static const float ALPHA_DISABLED = 0.16;
}
