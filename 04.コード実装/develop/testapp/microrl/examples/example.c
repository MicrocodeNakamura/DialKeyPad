#include <stdio.h>
#include <string.h>
#include "../src/microrl.h"
#include "example_misc.h"
#include "pico/types.h"
#include "pico/time.h"
#include "pico/stdio.h"

// create microrl object and pointer on it
microrl_t rl;
microrl_t * prl = &rl;

int myGetchar(void) ;
//*****************************************************************************
int microrl_main (void/*int argc, char ** argv*/)
{
	static unsigned char initialized = false;
	unsigned char mychar;

	if (!initialized) {
		initialized = true;
		init ();
		// call init with ptr to microrl instance and print callback
		microrl_init (prl, print);
		// set callback for execute
		microrl_set_execute_callback (prl, execute);

#ifdef _USE_COMPLETE
		// set callback for completion
		microrl_set_complete_callback (prl, complet);
#endif
		// set callback for Ctrl+C
		microrl_set_sigint_callback (prl, sigint);
	}
	
	// put received char from stdin to microrl lib
	mychar = myGetchar ();
	if (mychar != (unsigned char)0xff) {
		microrl_insert_char (prl, mychar);
	}

	return 0;
}

int myGetchar(void) {
    char buf[1];
	absolute_time_t timeout = make_timeout_time_ms(5);

	int len = stdio_get_until(buf, 1, timeout);
    if (len < 0) return 0xff;
    assert(len == 1);
    return (uint8_t)buf[0];	
}
