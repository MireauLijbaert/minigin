#include "string"
#include <cstdint>
namespace dae
{
	struct EventArg {

		union // Use a union to store different types of argument values but use the same memory location 
		{
			int intValue;
			float floatValue;
		};
	};

	struct Event {
		const std::string id;

		static const uint8_t MAX_ARGS = 8;
		uint8_t nbArgs;
		EventArg args[MAX_ARGS];

		explicit Event(const char* _id) : id{ _id }, nbArgs{ 0 } {}
	};
}
