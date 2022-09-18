/*
 * check_zapped_by_manyprocs
 * NOTE: The output for this is non-deterministic.

 * Check if all process which have issued a zap on a process are awakened
 * when the target process finally quits.
 */

#include <stdio.h>
#include <stdlib.h>
#include <usloss.h>
#include <phase1.h>

int XXp1(char *), XXp2(char *), XXp3(char *), XXp4(char *);

#define N 10

int victim;

int testcase_main()
{
    int i, status, pid2;
    char buf[25];

    USLOSS_Console("testcase_main(): started\n");
    USLOSS_Console("EXPECTATION: testcase_main() creates a single 'victim' process XXp3(); it has the same priority as testcase_main().  It then creates N other processes XXp2(), all at higher priority; each one interrupts testcase_main() until it blocks.  Each of these increments a global variable counter, and then blocks attempting to zap() the victim.  Each time that another XXp2() process blocks, testcase_main() and XXp3() race; if XXp3() runs, it does nothing (yet); if testcase_main() runs it creates another child.  At some point, when all of the XXp2() processes have run, and thus the global counter is at the expected value, then (whenever it happens to win the race next) XXp3() will exit its silent while() loop and terminate.  This will wake up all of the zap()ping processes, which will also terminate; then testcase_main() will clean it all up.\n");

    victim = fork1("XXp3", XXp3,"XXp3",USLOSS_MIN_STACK,5);

    for (i=0; i<N; i++)
    {
        sprintf(buf, "%d", i);

        USLOSS_Console("testcase_main(): fork1 creating child i=%d -- a new child will run next\n", i);
        pid2 = fork1("XXp2", XXp2, buf, USLOSS_MIN_STACK, 4);
        USLOSS_Console("testcase_main(): fork1 has completed, pid=%d\n", pid2);
    }

    /* clean up the victim, and also all of the XXp2() processes */
    USLOSS_Console("testcase_main(): Calling join() the first time.  Depending on how the race with XXp3() runs, this may happen before XXp3() ends its loop, and thus it will block; or it may happen after XXp3() terminates, and thus it will terminate immediately - as will *all* of the join()s, since all of the XXp2() processes will also be dead.\n");

    for (i=0; i<1+N; i++)
    {
        int pid = join(&status);
        USLOSS_Console("testcase_main: join() returned pid %d status %d\n", pid,status);
    }

    return 0;
}

volatile int count = 0;

int XXp2(char *arg)
{
    int i = atoi(arg);
    count++;

    USLOSS_Console("XXp2() %d: getpid()=%d\n", i, getpid());
    USLOSS_Console("XXp2() %d: zapping XXp3    current count: %d\n", i, count);
    zap(victim);
    USLOSS_Console("XXp2() %d: after zap.  This process will now terminate.\n", i);

    quit(3);
    return 0;    // so that gcc won't complain
}

int XXp3(char *arg)
{
    USLOSS_Console("XXp3(): started.  Entering the while() loop.\n");

    while(count < N) {
        ;
    }

    USLOSS_Console("XXp3(): count = %d -- the while loop has ended!\n",count);

    USLOSS_Console("XXp3(): terminating -- all of the XXp2() processes will unblock, and will all run in quick succession.  testcase_main() will not run until all of them have died.\n");
    quit(4);
    return 0;    // so that gcc won't complain
}

