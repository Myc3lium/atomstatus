# AtomStatus
## About
AtomStatus is a very simple plain-text status-line generator for dwm/other window managers.
It tries to follow a suckless-like strategy for configuration and extending.

Example usage for dwm/xsetroot method:
```bash
#!/bin/sh

atomstatus | xargs -I{} xsetroot -name " {}"
```

Configuration can be done with the `config.h` file. Commands are stored by type in arrays, 
and their index in their array is the interval (in seconds) or user signal to be run on. This
allows dispatching commands in constant time. At runtime, a pointer to each command is sorted 
into a separate array to allow them to be printed in order. Commands are stored as `Event` 
struct types. These contain the relative order of the command, whether to run it on startup, 
placeholder text, the last status, etc. If the macro ENABLE_PARALLEL is defined, the user
can define modules which are only run once, but which are updated according to the normal rules.
This allows scripts to be written which keep state between updates, and which run parallel (?)
to the main process.

```c
	int order;      // The relative order of the module.
	int on_startup; // Whether to run on startup.

#ifdef ENABLE_PARALLEL
	const int is_parallel; // Is command meant to be a continuous subprocess.
	union {                // We don't need command once a subprocess is started.
		FILE *subpr;           // Subprocess handle.
	    char *command;         // Command to run.
	};

#else
    const_string *command;     // Command to run.
#endif

	const_string *placeholder; // Text to use if no output is produced.
    struct string laststatus;     // Last output from command.
```

(see `personal-config.h` for more examples)
The below example illustrates updating the time every second:
```c
Event
on_interval [][MAX_PER_INTERVAL + 1] = {
	ON_INTERVAL(1,
		EVENT(
			.command    = "date '+%H:%M:%S'",
			.on_startup = 1,
			.order      = 4
		)
	),
...
```

Running commands on receiving real time (RT[MIN|MAX]) signals.
```c
Event
on_signal [][MAX_PER_SIGNAL + 1] = {
	ON_SIGNAL(1,
			EVENT(
				.command     = "echo got signal 1",
				.placeholder = "waiting for signal 1",
				.order       = 1
			)
	),
...
```

It is also possible to define commands that run only once, on starting up. This can be used
to display the current IP address, version, or similar constant information.
```c
Event
on_startup [] = {
	EVENT(
		.placeholder = "atomstatus vs." VERSION,
		.on_startup  = 0,
		.order       = 0
	),
};
```

Demonstrating the `is_parallel` functionality:
```c
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
```

Printed segments are separated using the variable `format_separator` in `config.h` as a separator.
Alternately, segments can be printf-d by defining the preprocessor macro `FORMAT` as a format string.

## Installation
1. Configure status commands.
2. Run `./build`. Move the resulting executable `atomstatus` to a suitable directory.

## TODO
1. Add non blocking read from `popen` processes.
2. Find source of weird `realloc` failure. This seems to be something to do
    with heap corruption, although I'm not sure exactly what.
