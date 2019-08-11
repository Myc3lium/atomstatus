#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <signal.h>
#include <time.h>


#define Unused                     __attribute__((unused))

#define VERSION "0.1.0"
#define const_string const char const

#define eprintf(FORMAT, ...)\
	fprintf(stderr, "[cstatus] " FORMAT "\n", __VA_ARGS__)

#define EMPTYEVENT { .command = NULL }
#define ISEMPTYEVENT(ev)\
	(ev. command == NULL)

#define MAX_PER_INTERVAL 8
#define MAX_INTERVAL (sizeof(on_interval) / sizeof(on_interval[0]))
#define ON_INTERVAL(interval, ...)\
    [interval - 1] = { __VA_ARGS__, EMPTYEVENT}

#define MAX_PER_SIGNAL 8
#define MAX_SIGNAL (sizeof(on_signal) / sizeof(on_signal [0]))
#define ON_SIGNAL(signal, ...)\
    [signal - 1] = { __VA_ARGS__, EMPTYEVENT}

#define MAX_STARTUP (sizeof(on_startup) / sizeof(on_startup[0]))
#define MAX_ORDERED (sizeof(ordered_events) / sizeof(ordered_events[0]))

#define ISNULLSTRING(st) (!(st. length) && !(st. allocated) && !(st. internal))
#define NULL_STRING { .length = 0, .allocated = 0, .internal = NULL }
#define EVENT(...)\
    { __VA_ARGS__, .laststatus = NULL_STRING }


struct string {
	unsigned length;
	unsigned allocated;
	char     *internal;
};

typedef
struct {
	int order;      // The relative order of the module.
	int on_startup; // Run on startup.

	const_string *command;     // Command.
	const_string *placeholder; // Text to use if no output.

    struct string      // Last output from command.
		laststatus;

} Event;


int   compare_elements       (const void*, const void*);
void  sort_events            (void);
void  initial_run            (void);
int   run_modules            (Event*);
int   run_module             (Event*);
void  handle_user_signal     (int);
void  handle_sigint_cleanup  (int);
int   sfgetline              (FILE*, struct string *);
void  sfree                  (struct string*);
void  print_all              (void);


#include "config.h"


Event *ordered_events
    [(MAX_INTERVAL * MAX_PER_INTERVAL) +
	 (MAX_SIGNAL * MAX_PER_SIGNAL) +
	 (MAX_STARTUP) + 1] =

                { NULL };

int
compare_elements (const void* a, const void* b){
	Event *ev_a = *((Event**)a),
	      *ev_b = *((Event**)b);

     if (ev_a -> order == ev_b -> order)
		 return 0;

     else if (ev_a -> order < ev_b -> order)
		 return -1;

     else
		 return 1;
}

void
sort_events (void){
	// Copy pointers for each module array.

    size_t all = 0;
    for (size_t i = 0; i < MAX_INTERVAL; i++)
		for (size_t x = 0; !ISEMPTYEVENT((on_interval [i][x])); x++, all++)
			ordered_events [all] = &(on_interval [i][x]);

    for (size_t i = 0; i < MAX_SIGNAL; i++)
		for (size_t x = 0; !ISEMPTYEVENT((on_signal [i][x])); x++, all++)
			ordered_events [all] = &(on_signal [i][x]);

    for (size_t i = 0; i < MAX_STARTUP; i++, all++)
		ordered_events [all] = &(on_startup [i]);

	// Sort by .order priority.
    qsort(ordered_events,
			all,
			sizeof(Event*),
			compare_elements);

	ordered_events [all] = NULL; // Place terminating NULL pointer for seeking end.
}

void
print_all (void){
	// Print all modules in order, separated by format_separator.

	Event **modules = ordered_events;
	size_t index = 0;

	for (; *modules; modules++, index++){
		if (!ISNULLSTRING((*modules)->laststatus)){
		    if (index > 0)
				printf("%s", format_separator);

			printf(FORMAT, (*modules) -> laststatus. internal);
		}

		else if ((*modules) -> placeholder) {
			if (index > 0)
				printf("%s", format_separator);

			printf(FORMAT, (*modules) -> placeholder);
		}
	}

	printf("\n"); // flush stdout.
	// Could use fflush(stdout), but we need a newline anyway.
}

int
sfgetline (FILE* source, struct string *output){
	int carry;

	if (!output -> internal){
		output -> allocated = 64;

		if (!(output -> internal = malloc (64))){
			eprintf("Failed allocating %d initial bytes for string.", output -> length);
			return 1;
		}
	}

	output -> length = 0; // Set length;
	output -> internal [0] = '\0';

	while ((carry = fgetc (source)) != EOF){ // EOF encountered, leave the string with intact from last state.
		if (carry == '\n' || carry == '\r' || carry == '\0') // Stop on newline.
			return 0;

		if (output -> length + 1 == output -> allocated) // width of characters == allocated width
			if (! realloc (output -> internal, (output ->allocated += output -> allocated / 2))){
				eprintf("Failed allocating %d bytes reading output of command.", output -> length);
				return 1;
			}

		output -> internal [output -> length ++] = carry;
		output -> internal [output -> length]    = '\0';
	}

	return 0;
}

void
sfree(struct string *st){
	if (st && st -> allocated && st -> internal)
		free (st -> internal);
}

int
run_module (Event *module){
	// Take a pointer to a single module and run it.
	if (!(module -> command)) // abort on empty command.
		return 1;

	FILE *process = popen (module -> command, "r");
	int gotline   = sfgetline (process, &(module -> laststatus)),
	    exitstat  = 0;

	if ((exitstat = pclose (process)))
        eprintf("Command '%s' exited with nonzero status %d", module -> command, exitstat);

	return (gotline || exitstat);
}

int
run_modules (Event *modules){
	// Take a pointer to a series of modules and run them.
	int exitstat = 0;
	for (; !(ISEMPTYEVENT((*modules))); modules++)
		exitstat = (exitstat || run_module (modules));

	return exitstat;
}

void
initial_run (void){
	// Run startup modules and other modules which have .on_startup = 1
    for (size_t i = 0; i < MAX_STARTUP; i++)
		run_module (&(on_startup [i]));

	for (size_t i = 0; i < MAX_INTERVAL; i++){
		for (size_t x = 0; !ISEMPTYEVENT((on_interval [i][x])); x++)
			if (on_interval [i][x]. on_startup)
				run_module (&(on_interval [i][x]));
	}

	for (size_t i = 0; i < MAX_SIGNAL; i++){
		for (size_t x = 0; !ISEMPTYEVENT((on_signal [i][x])); x++)
			if (on_signal [i][x]. on_startup)
				run_module (&(on_signal [i][x]));
	}
}

void
handle_user_signal (int signo){
	// Y U NO DISPATCH COMMANDS?
	// Ok, this seems to work now. sending signals > MAX_SIGNAL
	// crashes the program though. Tried registering handlers that do
	// nothing for RTsignals above MAX_SIGNAL, but that stopped the signals
	// from being received at all.
	if ((signo - SIGRTMIN - 1) >= (int)MAX_SIGNAL)
		return;

	run_modules (on_signal [(signo - SIGRTMIN - 1)]);
}

void
handle_sigint_cleanup (Unused int signo){
	eprintf("Caught signal SIGINT, exiting...%s", "");

	// Cleanup tabled statuses.
	for (size_t i = 0; i < MAX_INTERVAL; i++)
		for (Event *module = on_interval[i]; !ISEMPTYEVENT((*module)); module++)
			sfree (&(module -> laststatus));

	for (size_t i = 0; i < MAX_SIGNAL; i++)
		for (Event *module = on_signal[i]; !ISEMPTYEVENT((*module)); module++)
			sfree (&(module -> laststatus));

	for (size_t i = 0; i < MAX_STARTUP; i++)
		sfree (&(on_startup [i]. laststatus));

	exit (EXIT_SUCCESS); // Abort program.
}

int
main (void){
	// Set cleanup handle for SIGNINT.
	if (signal(SIGINT, handle_sigint_cleanup) == SIG_ERR)
		eprintf("Failed to register handler for SIGINT.%s", "");

   	// Set handlers for user defined signals.
   	for (int signo = 0; signo < (int)MAX_SIGNAL; signo ++)
   		if (signal(SIGRTMIN+ signo, handle_user_signal) == SIG_ERR)
   			eprintf("Failed to register shared handler for signal (%d)", (SIGRTMIN+ signo));

	// Run modules to get starting values.
    initial_run ();

	// Sort pointers to each event for print ordering.
	sort_events ();

    // Timed loop for dispatching events on interval.
    for (time_t counter = time (NULL);; ){
    	counter = time (NULL);

    	for (size_t interval = 0; interval < (MAX_INTERVAL-1); interval ++)
    		if (counter % (interval + 1) == 0)
    			run_modules (on_interval [interval]);

    	print_all ();
    	sleep (1);
    }
}
