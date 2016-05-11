#ifndef MYSCHED_H
#define MYSCHED_H

#include <malloc.h>
#include <stdio.h>
#include "mythread.h"
#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/syscall.h>

static int internal_scheduler(void);

static void sig_handler(int sig); //, siginfo_t *a, void *b

void mythread_init_sched(void);

void mythread_exit_sched(void);

void mythread_leave_kernel(void);

#endif /* MYSCHED_H */