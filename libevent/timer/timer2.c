#include <stdio.h>
#include <event.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdlib.h>

#define rel_time_t unsigned int

struct event_base *main_base;
struct event clockevent,clockevent2;
volatile unsigned int current_time;

void clock_handler(const int fd, const short which, void *arg) ;

static void set_current_time(void)
{
	struct timeval timer;
	gettimeofday(&timer, NULL);
	current_time = (rel_time_t) (timer.tv_sec);
}

void clock_handler2(const int fd, const short which, void *arg)
{
	printf("this is clock_handler2 function\n") ;
}


void clock_handler(const int fd, const short which, void *arg)
{
	struct timeval t = {.tv_sec = 1, .tv_usec = 0 };
	struct timeval t2 = {.tv_sec = 1, .tv_usec = 0 };
	static bool initialized = false;

	if(initialized){
		evtimer_del(&clockevent);
		evtimer_del(&clockevent2);
	}else{
		initialized = true;
	}
	evtimer_set(&clockevent2, clock_handler2, 0);
	event_base_set(main_base, &clockevent2);
	evtimer_add(&clockevent2, &t2);

	evtimer_set(&clockevent, clock_handler, 0);
	event_base_set(main_base, &clockevent);
	evtimer_add(&clockevent, &t);
	
	set_current_time();
	printf("current_time: %d\n", current_time);
}


int main(int argc, char *argv[])
{
	main_base = event_init() ;
	clock_handler(0,0,0) ;

	event_base_loop(main_base, 0);

	return 0 ;
}
