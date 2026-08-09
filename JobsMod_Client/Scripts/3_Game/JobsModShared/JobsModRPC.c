// JobsModRPC.c
//
// The wire contract between client and server. Both sides read these constants
// from this one file, so an id can never drift apart between the two PBOs.
//
// PROTOCOL_VERSION is sent in every client request and checked by the server.
// Bump it whenever the payload shape of any message below changes: an outdated
// client is then rejected with a clear reason instead of silently misreading
// the stream and corrupting a job.

class JobsModRPC
{
	// Base offset chosen high enough to stay clear of vanilla RPC ids.
	static const int BASE = 24500;

	// 1 -> 2: NPC job assignment and the loader job were added.
	// 2 -> 3: the courier job was added; NOTIFY_JOB_STATE now carries the job
	//         type, because the client can no longer tell what to point the
	//         marker at from the freight class alone.
	static const int PROTOCOL_VERSION = 3;

	// Every id below must stay inside [BASE, BASE + ID_RANGE]; both OnRPC
	// handlers use that window to ignore traffic that is not ours.
	static const int ID_RANGE = 100;

	// --- Client -> Server ---
	// Starting a sorting session has no message of its own: ActionSortTrash
	// already runs its OnExecuteServer half on the server, so the request
	// travels through the engine's own action pipeline. Talking to an NPC works
	// the same way through ActionTalkToNpc.
	//
	// Player finished the minigame; payload carries the produced bin sequence.
	static const int REQUEST_SORTING_SUBMIT = BASE + 2;
	// Player closed the minigame menu without finishing.
	static const int REQUEST_SORTING_ABORT = BASE + 3;
	// Player picked a job in the NPC menu.
	static const int REQUEST_JOB_ACCEPT = BASE + 4;
	// Player asked the NPC to sign off a finished job and pay.
	static const int REQUEST_JOB_COMPLETE = BASE + 5;
	// Player gave up on the job they are holding.
	static const int REQUEST_JOB_ABANDON = BASE + 6;

	// --- Server -> Client ---
	// Sorting session granted; carries nonce, zone name and the shuffled order.
	static const int NOTIFY_SORTING_SESSION = BASE + 50;
	// A request was refused; carries a JobsModRejectReason value.
	static const int NOTIFY_REJECTED = BASE + 51;
	// Sorted pile accepted.
	static const int NOTIFY_SORTING_ACCEPTED = BASE + 52;
	// The offer list of the NPC the player is standing at; opens the NPC menu.
	static const int NOTIFY_JOB_MENU = BASE + 53;
	// Current job of this player, or "no job"; drives the HUD.
	static const int NOTIFY_JOB_STATE = BASE + 54;
	// A free-text notification (job taken, paid, abandoned).
	static const int NOTIFY_JOB_MESSAGE = BASE + 55;
	// Where the NPCs stand, so the client can offer the talk action on them.
	static const int NOTIFY_NPC_DIRECTORY = BASE + 56;

	// Separator between item ids inside the packed order/sequence strings.
	// A comma is safe here because no catalog id may contain one — the catalog
	// enforces that in JobsModTrashCatalog.Validate().
	static const string FIELD_SEPARATOR = ",";

	// A player must stand this close (metres) to the trash pile both when the
	// sorting session starts and when the result is submitted.
	static const float INTERACTION_DISTANCE = 4.0;

	// How close a player must stand to an NPC to talk to it and to hand a job in.
	static const float NPC_INTERACTION_DISTANCE = 3.0;

	// A granted sorting session expires after this many seconds. It bounds how
	// long a stale nonce stays usable if the client never answers.
	static const int SESSION_TIMEOUT_SECONDS = 600;

	// Upper bound on how many offers one NPC menu may carry. The layout has a
	// fixed number of rows, and the server must never promise more than it can
	// draw.
	static const int MAX_JOBS_PER_NPC = 6;
}

// What kind of work a job definition describes. The string form is what an
// admin writes in the job JSON; the int form is what the code branches on.
class JobsModJobType
{
	static const int UNKNOWN = 0;
	static const int SORTING = 1;
	static const int LOADING = 2;
	// Carry one sealed parcel from the employer to a second NPC, who is the one
	// that takes it and pays. The only job with two people in it.
	static const int MESSENGER = 3;

	static const string TEXT_SORTING = "sorting";
	static const string TEXT_LOADING = "loading";
	static const string TEXT_MESSENGER = "messenger";

	static int FromText(string text)
	{
		if (text == TEXT_SORTING)
			return SORTING;

		if (text == TEXT_LOADING)
			return LOADING;

		if (text == TEXT_MESSENGER)
			return MESSENGER;

		return UNKNOWN;
	}
}

// What a world marker points at. The client uses this to pick which of the
// points the server sent is the one to draw right now — the yard you load at
// and the yard you unload at are both relevant, but never at the same moment.
class JobsModMarkerKind
{
	// A place to work: a trash pile.
	static const int TARGET = 0;
	// Where the freight is picked up.
	static const int SOURCE = 1;
	// Where what you are carrying has to end up: the unloading yard for the
	// loader, the recipient for the courier.
	static const int DESTINATION = 2;
	// The employer, once the job is done and only the pay is left.
	static const int EMPLOYER = 3;
}

// Why one offer in an NPC menu can or cannot be taken. Decided by the server
// and sent as a number, so the client never has to reason about cooldowns or
// about what job the player is holding.
class JobsModOfferState
{
	static const int AVAILABLE = 0;
	static const int ON_COOLDOWN = 1;
	static const int BLOCKED_BY_OTHER = 2;
	static const int HELD = 3;
}

// Lifecycle of one accepted job, as far as the client needs to know.
class JobsModJobStatus
{
	// No job held: the HUD hides itself.
	static const int NONE = 0;
	// Work in progress.
	static const int ACTIVE = 1;
	// Everything done; the player has to walk back to the NPC to be paid.
	static const int READY_TO_HAND_IN = 2;
}

// Why a request was refused. The client maps these to readable text; the raw
// value is what travels over the wire so the strings can change freely.
class JobsModRejectReason
{
	static const int UNKNOWN = 0;
	static const int PROTOCOL_MISMATCH = 1;
	static const int PLAYER_NOT_READY = 2;
	static const int TOO_FAR = 3;
	static const int ON_COOLDOWN = 4;
	static const int NO_ACTIVE_SESSION = 5;
	static const int SESSION_EXPIRED = 6;
	static const int RESULT_INCORRECT = 7;
	static const int ALREADY_BUSY = 8;
	static const int UNKNOWN_PILE = 9;
	static const int UNKNOWN_NPC = 10;
	static const int UNKNOWN_JOB = 11;
	static const int NPC_DOES_NOT_OFFER = 12;
	static const int NO_ACTIVE_JOB = 13;
	static const int JOB_ALREADY_HELD = 14;
	static const int JOB_NOT_FINISHED = 15;
	static const int WRONG_ZONE = 16;
	static const int WRONG_NPC = 17;
	static const int PARCEL_MISSING = 18;
	static const int NO_INVENTORY_SPACE = 19;

	static string GetText(int reason)
	{
		switch (reason)
		{
			case PROTOCOL_MISMATCH:
				return "Версия мода не совпадает с серверной. Обновите мод.";
			case PLAYER_NOT_READY:
				return "Сейчас нельзя приступить к работе.";
			case TOO_FAR:
				return "Отойдя от точки, работу продолжить нельзя.";
			case ON_COOLDOWN:
				return "Вы недавно закончили эту работу. Приходите позже.";
			case NO_ACTIVE_SESSION:
				return "Смена не найдена. Начните работу заново.";
			case SESSION_EXPIRED:
				return "Время смены истекло. Начните работу заново.";
			case RESULT_INCORRECT:
				return "Сортировка не принята: проверьте контейнеры.";
			case ALREADY_BUSY:
				return "Вы уже сортируете мусор на другой точке.";
			case UNKNOWN_PILE:
				return "Эта куча не является рабочей точкой.";
			case UNKNOWN_NPC:
				return "Этот человек не выдаёт работу.";
			case UNKNOWN_JOB:
				return "Такой работы больше нет.";
			case NPC_DOES_NOT_OFFER:
				return "Этот человек такую работу не выдаёт.";
			case NO_ACTIVE_JOB:
				return "Сначала возьмите работу у нанимателя.";
			case JOB_ALREADY_HELD:
				return "Вы уже взяли работу. Сначала закончите её.";
			case JOB_NOT_FINISHED:
				return "Работа ещё не выполнена.";
			case WRONG_ZONE:
				return "Это место не относится к вашей работе.";
			case WRONG_NPC:
				return "Эту работу принимает другой человек.";
			case PARCEL_MISSING:
				return "Пакета при вас нет. Работа не выполнена.";
			case NO_INVENTORY_SPACE:
				return "Освободите место в инвентаре под пакет.";
		}

		return "Запрос отклонён.";
	}
}
