#include <stdio.h>
#include <event.h>

int main()
{
	struct event_base *base ;
	base = event_init();
	return 0 ;
}
