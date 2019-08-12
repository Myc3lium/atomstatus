# AtomStatus
## About
A very simple plain-text status-line generator for dwm/other window managers.
Intended to follow a suckless-style strategy for configuration and extending.

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
placeholder text, the last status, etc.

```c
int order;                     // The relative order of the module.
int on_startup;                // Run on startup.

const char const *command;     // Command.
const char const *placeholder; // Text to use if no output.

struct string      // Last output from command.
	laststatus;
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

Printed segments are separated using the variable `format_separator` in `config.h` as a separator.
Alternately, segments can be printf-d by defining the preprocessor macro `FORMAT` as a format string.

## Installation
1. Configure status commands.
2. Run `./build`. Move the resulting executable `atomstatus` to a suitable directory.

## TODO
1. Add non blocking read from `popen` processes.
