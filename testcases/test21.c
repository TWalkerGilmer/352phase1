/* this test case checks for a process zapping itself 
 * testcase_main forks XXp1 and is blocked on the join of XXp1 and XXp1 zaps itself
 */

#include <stdio.h>
#include <usloss.h>
#include <phase1.h>

int XXp1(char *);
int pid1;

int testcase_main()
{
    int status, kidpid;

    USLOSS_Console("testcase_main(): started\n");
// TODO    USLOSS_Console("EXPECTATION: TBD\n");
    USLOSS_Console("QUICK SUMMARY: A process tries to zap() itself\n");

    pid1 = fork1("XXp1", XXp1, "XXp1", USLOSS_MIN_STACK, 3);
    USLOSS_Console("testcase_main(): after fork of child %d\n", pid1);

    USLOSS_Console("testcase_main(): performing join\n");
    kidpid = join(&status);
    USLOSS_Console("testcase_main(): exit status for child %d is %d\n", kidpid, status); 

    return 0;
}

int XXp1(char *arg)
{
    int status;

    USLOSS_Console("XXp1(): started\n");
    USLOSS_Console("XXp1(): arg = `%s'\n", arg);

    status = zap(pid1);
    USLOSS_Console("XXp1(): after zap'ing itself , status = %d\n", status);

    quit(3);
    return 0;    // so that gcc won't complain
}

