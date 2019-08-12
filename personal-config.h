// Using arrays for intervals and signals allows us to dispatch
// events/modules in O(1) (constant) time.
const char* format_separator = "|";
#define FORMAT "%s"

// Event members:
//     int order;      // The relative order of the module.
//     int on_startup; // Whether to run on startup. This option is implied by is_parallel.
//
//     const int is_parallel; // Is command meant to be a continuous subprocess.
//
// #ifdef ENABLE_PARALLEL
//     union { // We don't need a command once a subprocess is started.
//     	   FILE *subpr;       // Subprocess handle.
//         char *command;     // Command.
//     };
// #else
//     const char const *command;
// #endif
//
//     const char const *placeholder; // Text to use if no output.
//
//     struct string      // The last successful output from command.
//         laststatus;

Event
on_interval [][MAX_PER_INTERVAL + 1] = {
	ON_INTERVAL(1,
		EVENT(
			.command    = "date '+[%H:%M:%S]'",
			.on_startup = 1,
			.order      = 5
		)
	),

	ON_INTERVAL(5,
		EVENT(
			.command     = "bash ~/.config/dwm/bandwidth",
			.placeholder = "",
			.on_startup  = 1,
			.order       = 2
		)
	),

	ON_INTERVAL(10,
		EVENT(
			.command    = "acpi -b | awk '{ print $3, substr($4,1,index($4,\"%\")) }'",
			.on_startup = 1,
			.order      = 4
		)
	),

	ON_INTERVAL(60,
		EVENT(
			.command    = "uptime -p | awk '{ print \"up\",$2 \":\" $4 }'",
			.on_startup = 1,
			.order      = 3
		)
	),
};

Event
on_signal [][MAX_PER_SIGNAL + 1] = {
	ON_SIGNAL(1,
			EVENT(
				.command     = "bash \"$HOME/.config/dwm/cmus-blocklet\"",
				.placeholder = "<cmus down>",
				.on_startup  = 1,
				.order       = 0
			)
	),

	ON_SIGNAL(2,
			EVENT(
				.command     = "bash \"$HOME/.config/dwm/volume\"",
				.placeholder = "<volume down>",
				.on_startup  = 1,
				.order       = 1
			)
	),

#ifdef ENABLE_PARALLEL
	ON_SIGNAL(3,
			EVENT(
				.command     = "count=0;while true; do count=$((count + 1)); echo $count; sleep 1; done",
				.placeholder = "blah",
				.is_parallel = 1,
				.order       = 1
			)
	),
#endif

	{EMPTYEVENT},
};

Event
on_startup [] = {
	EMPTYEVENT
	// EVENT(
	// 	.placeholder = "atomstatus vs." VERSION,
	// 	.on_startup  = 0,
	// 	.order       = 0
	// ),
};
