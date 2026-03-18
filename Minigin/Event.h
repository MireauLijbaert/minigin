#include "string"

struct EventArg {
};

struct Event {
	const std::string id;

	static const uint8_t MAX_ARGS = 8;
	uint8_t nbArgs;
	EventArg args[MAX_ARGS];

	explicit Event(const char* _id) : id{_id} {}
};