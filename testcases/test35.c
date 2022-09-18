#include <stdio.h>
#include <usloss.h>
#include <phase1.h>

/* The purpose of this test is to demonstrate that
 * if we zap a process that has already quit but has
 * not joined, we return immediatly from zap with
 * status -1 (if we have already been zapped before
 * calling zap).
 */

int XXp1(char *), XXp2(char *), XXp3(char *);

int toZapPid;

int testcase_main()
{
    int status, pid1, kidpid;

    USLOSS_Console("testcase_main(): started\n");
// TODO    USLOSS_Console("EXPECTATION: TBD\n");
    USLOSS_Console("QUICK SUMMARY: Attempting to zap() a process that has already died, but the parent has not yet join()ed.\n");

    pid1 = fork1("XXp1", XXp1, "XXp1", USLOSS_MIN_STACK, 4);
    USLOSS_Console("testcase_main(): after fork of child %d\n", pid1);

    USLOSS_Console("testcase_main(): performing first join\n");
    kidpid = join(&status);
    USLOSS_Console("testcase_main(): exit status for child %d is %d\n", kidpid, status);

    return 0;
}

int XXp1(char *arg)
{
    int status, pid1, kidpid, result;

    USLOSS_Console("XXp1(): started\n");
    USLOSS_Console("XXp1(): arg = `%s'\n", arg);

    toZapPid = fork1("XXp2", XXp2, "XXp2", USLOSS_MIN_STACK, 3);
    USLOSS_Console("XXp1(): after fork of child %d\n", toZapPid);

    pid1 = fork1("XXp3", XXp3, "XXp3", USLOSS_MIN_STACK, 5);
    USLOSS_Console("XXp1(): after fork of child %d\n", pid1);

    USLOSS_Console("XXp1(): calling zap(%d)\n", pid1);
    result = zap(pid1);
    USLOSS_Console("XXp1(): zap(%d) returned: %d\n", pid1, result);

    USLOSS_Console("XXp1(): performing first join\n");
    kidpid = join(&status);
    USLOSS_Console("XXp1(): exit status for child %d is %d\n", kidpid, status);

    USLOSS_Console("XXp1(): performing second join\n");
    kidpid = join(&status);
    USLOSS_Console("XXp1(): exit status for child %d is %d\n", kidpid, status);

    quit(1);
    return 0;    // so that gcc won't complain
}

int XXp2(char *arg)
{
    USLOSS_Console("XXp2(): started\n");
    USLOSS_Console("XXp2(): arg = `%s'\n", arg);

    USLOSS_Console("XXp2(): exiting by calling quit(2)\n");
    quit(2);
    return 0;    // so that gcc won't complain
}

int XXp3(char *arg)
{
    int result;

    USLOSS_Console("XXp3(): started\n");
    USLOSS_Console("XXp3(): arg = `%s'\n", arg);

    USLOSS_Console("XXp3(): calling zap(%d)\n", toZapPid);
    result = zap(toZapPid);
    USLOSS_Console("XXp3(): zap(%d) returned: %d\n", toZapPid, result);

    quit(3);
    return 0;    // so that gcc won't complain
}

