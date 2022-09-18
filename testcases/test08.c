/* This test is to check whether fork tests for the mode being == kernel */

#include <stdio.h>
#include <usloss.h>
#include <phase1.h>

int XXp1(char *);

int testcase_main()
{
    int status, pid1, kidpid;
    int result;

    USLOSS_Console("testcase_main(): started\n");
    USLOSS_Console("EXPECTATION: The simulation should be terminated as soon as fork() is called, since we are not in kernel mode.\n");

    result = USLOSS_PsrSet( USLOSS_PsrGet() & ~USLOSS_PSR_CURRENT_MODE );
    if ( result != USLOSS_DEV_OK ) {
        USLOSS_Console("testcase_main: USLOSS_PsrSet returned %d\n", result);
        USLOSS_Console("Halting...\n");
        USLOSS_Halt(1);
    }
    pid1 = fork1("XXp1", XXp1, "XXp1", USLOSS_MIN_STACK, 3);
    USLOSS_Console("testcase_main(): after fork of child %d\n", pid1);
    USLOSS_Console("testcase_main(): performing join\n");
    kidpid = join(&status);
    USLOSS_Console("testcase_main(): exit status for child %d is %d\n", kidpid, status); 

    return 0;
}

int XXp1(char *arg)
{
    int i;

    USLOSS_Console("XXp1(): started -- should not see this message!!!!!!!!\n");
    USLOSS_Console("XXp1(): arg = `%s'\n", arg);
    for(i = 0; i < 100; i++)
        ;
    quit(3);
    return 0;    // so that gcc won't complain
}
