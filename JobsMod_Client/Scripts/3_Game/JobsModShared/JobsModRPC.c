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

	static const int PROTOCOL_VERSION = 1;

	// --- Client -> Server ---
	// Player interacted with a trash pile and wants to start sorting.
	static const int REQUEST_SORTING_START = BASE + 1;
	// Player finished the minigame; payload carries the produced bin sequence.
	static const int REQUEST_SORTING_SUBMIT = BASE + 2;
	// Player closed the menu without finishing.
	static const int REQUEST_SORTING_ABORT = BASE + 3;

	// --- Server -> Client ---
	// Session granted; payload carries nonce, zone name and the shuffled order.
	static const int NOTIFY_SORTING_SESSION = BASE + 50;
	// Session refused; payload carries a JobsModRejectReason value.
	static const int NOTIFY_SORTING_REJECTED = BASE + 51;
	// Submitted result accepted; the job is done and the reward was paid.
	static const int NOTIFY_SORTING_ACCEPTED = BASE + 52;

	// Separator between item ids inside the packed order/sequence strings.
	// A comma is safe here because no catalog id may contain one — the catalog
	// enforces that in JobsModTrashCatalog.Validate().
	static const string FIELD_SEPARATOR = ",";

	// A player must stand this close (metres) to the trash pile both when the
	// session starts and when the result is submitted.
	static const float INTERACTION_DISTANCE = 4.0;

	// A granted session expires after this many seconds. It bounds how long a
	// stale nonce stays usable if the client never answers.
	static const int SESSION_TIMEOUT_SECONDS = 600;
}

// Why a request was refused. The client maps these to readable text; the raw
// value is what travels over the wire so the strings can change freely.
class JobsModRejectReason
{
	static const int UNKNOWN = 0;
	static const int PROTOCOL_MISMATCH = 1;
	static const int PLAYER_NOT_READY = 2;
	static const int TOO_FAR = 3;
	static const int NOT_OWNER = 4;
	static const int NO_ACTIVE_SESSION = 5;
	static const int SESSION_EXPIRED = 6;
	static const int RESULT_INCORRECT = 7;
	static const int ALREADY_BUSY = 8;

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
			case NOT_OWNER:
				return "Эта рабочая точка закреплена за другим сотрудником.";
			case NO_ACTIVE_SESSION:
				return "Смена не найдена. Начните работу заново.";
			case SESSION_EXPIRED:
				return "Время смены истекло. Начните работу заново.";
			case RESULT_INCORRECT:
				return "Сортировка не принята: проверьте контейнеры.";
			case ALREADY_BUSY:
				return "Вы уже сортируете мусор на другой точке.";
		}

		return "Запрос отклонён.";
	}
}
