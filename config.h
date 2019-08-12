// Using arrays for intervals and signals allows us to dispatch
// events/modules in O(1) (constant) time.
const char* format_separator = " | ";
#define FORMAT "%s"

// Event members:
//	   int order;                     // The relative order of the module.
//	   int on_startup;                // Run on startup.
//
//	   const char const *command;     // Command.
//	   const char const *placeholder; // Text to use if no output.
//
//	   struct string      // Last output from command.
//		laststatus;

Event
on_interval [][MAX_PER_INTERVAL + 1] = {
	ON_INTERVAL(1,
		EVENT(
			.command    = "date '+%H:%M:%S'",
			.on_startup = 1,
			.order      = 4
		)
	),

	ON_INTERVAL(10,
		EVENT(
			.command    = "acpi -b | awk '{ print $3, substr($4,1,index($4,\"%\")) }'",
			.on_startup = 1,
			.order      = 3
		)
	),

	ON_INTERVAL(60,
		EVENT(
			.command    = "uptime -p | awk '{ print \"up\",$2 \":\" $4 }'",
			.on_startup = 1,
			.order      = 2
		)
	),
};

Event
on_signal [][MAX_PER_SIGNAL + 1] = {
	ON_SIGNAL(1,
			EVENT(
				.command     = "echo got signal 1",
				.placeholder = "waiting for signal 1",
				.order       = 1
			)
	),

	{EMPTYEVENT},
};

Event
on_startup [] = {
	EVENT(
		.placeholder = "atomstatus vs." VERSION,
		.on_startup  = 0,
		.order       = 0
	),
};

