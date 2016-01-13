#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>
 
typedef struct my_timer_s my_timer_t;
 
struct my_timer_s {
    my_timer_t *prev, *next;
    int diff_sec;
    int diff_usec;
    void (*func)();
};
 
my_timer_t  *timer_list = NULL;
 
void callback_timeout()
{
    my_timer_t *p, *q;
    struct itimerval itimer;
    sigset_t set, oldset;
 
    p = timer_list;
 
    sigemptyset(&set);
    sigaddset(&set, SIGALRM);
    sigprocmask(SIG_SETMASK, &set, &oldset);
 
    for (q = timer_list->next; q; q = q->next) {
        q->diff_sec -= p->diff_sec;
        q->diff_usec -= p->diff_usec;
    }
 
    if (timer_list->next != NULL) {
        timer_list = timer_list->next;
 
        itimer.it_interval.tv_sec = 0;
        itimer.it_interval.tv_usec = 0;
        itimer.it_value.tv_sec = timer_list->diff_sec;
        itimer.it_value.tv_usec = timer_list->diff_usec;
 
        setitimer(ITIMER_REAL, &itimer, NULL);
    }
 
    sigprocmask(SIG_SETMASK,&oldset,NULL);
 
    p->func();
 
    free(p);
}
 
int register_timer(int sec, int usec, void (*action)())
{
    my_timer_t  *t, *p, *pre;
    struct itimerval itimer;
    struct sigaction sa;
    sigset_t set, oldset;
 
    t = (my_timer_t *) malloc(sizeof(my_timer_t));
    t->next = t->prev = NULL;
    t->diff_sec = sec;
    t->diff_usec = usec;
    t->func = action;
 
    sigemptyset(&set);
    sigaddset(&set, SIGALRM);
    sigprocmask(SIG_SETMASK,&set,&oldset);
 
    if (timer_list == NULL) {
 
        timer_list = t;
 
    } else {
 
        for (pre = NULL, p = timer_list; p; pre = p, p = p->next) {
            if (p->diff_sec > t->diff_sec ) {
                t->next = p;
                p->prev = t;
 
                if (p->prev) {
                    p->prev->next = t;
                    t->prev = p->prev;
                }
 
                break;
            }
        }
 
        if (p == NULL) {
            t->prev = pre;
            pre->next = t;
        }
    }
 
    t = timer_list;
 
    itimer.it_interval.tv_sec = 0;
    itimer.it_interval.tv_usec = 0;
    itimer.it_value.tv_sec = t->diff_sec;
    itimer.it_value.tv_usec = t->diff_usec;
    setitimer(ITIMER_REAL, &itimer, NULL);
 
    sigprocmask(SIG_SETMASK, &oldset, NULL);
 
    return 0;
}
 
void func1()
{
    printf("timer1\n");
}
 
 
void func2()
{
    printf("timer2\n");
}
 
 
void func3()
{
    printf("timer3\n");
}
 
 
void timer_handler (int signo)
{
    switch(signo) {
    case SIGALRM:
        callback_timeout();
        break;
    }
}
 
int main ()
{
    struct sigaction sa;
    struct itimerval itimer;
 
    /*
    memset (&sa, 0, sizeof (sa));
    sa.sa_handler = &timer_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGALRM, &sa, NULL);
    */
    signal(SIGALRM, timer_handler);
 
    register_timer(1, 0, &func1);
    register_timer(4, 0, &func2);
    register_timer(5, 0, &func3);
 
    while (1);
 
    return 0;
}
