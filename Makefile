
OPTS=-Wall -Wextra -pedantic -Wno-comment -Wformat-nonliteral -Wformat-security -Wuninitialized -Winit-self -Warray-bounds=2 -Wenum-compare -Werror=implicit-function-declaration 
CC=gcc

atomstatus: atomstatus.c config.h
	rm -f $@
	$(CC) $(OPTS) $< -o $@

clean:
	rm -f atomstatus

.PHONY = clean
