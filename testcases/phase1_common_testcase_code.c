
#include <usloss.h>
#include <phase1.h>



/* dummy MMU structure, used for debugging whether the student called thee
 * right functions.
 */
int dummy_mmu_pids[MAXPROC];

int mmu_init_count = 0;
int mmu_quit_count = 0;
int mmu_switch_count = 0;



void startup(int argc, char **argv)
{
    for (int i=0; i<MAXPROC; i++)
        dummy_mmu_pids[i] = -1;

    /* all student implementations should have PID 1 (init) */
    dummy_mmu_pids[1] = 1;

    phase1_init();
    startProcesses();
}



void mmu_init_proc(int pid)
{
    mmu_init_count++;

    int slot = pid % MAXPROC;

    if (dummy_mmu_pids[slot] != -1)
    {
        USLOSS_Console("TESTCASE ERROR: mmu_init_proc(%d) called, when the slot was already allocated for process %d\n", pid, dummy_mmu_pids[slot]);
        USLOSS_Halt(1);
    }

    dummy_mmu_pids[slot] = pid;
}

void mmu_quit(int pid)
{
    mmu_quit_count++;

    int slot = pid % MAXPROC;

    if (dummy_mmu_pids[slot] != pid)
    {
        USLOSS_Console("TESTCASE ERROR: mmu_quit(%d) called, but the slot didn't contain the expected pid.  slot: %d\n", pid, dummy_mmu_pids[slot]);
        USLOSS_Halt(1);
    }

    dummy_mmu_pids[slot] = -1;
}

void mmu_switch(int new_pid)
{
    mmu_switch_count++;

    int slot = new_pid % MAXPROC;

    if (dummy_mmu_pids[slot] != new_pid)
    {
        USLOSS_Console("TESTCASE ERROR: mmu_switch(%d) called, but the slot didn't contain the expected pid.  slot: %d\n", new_pid, dummy_mmu_pids[slot]);
        USLOSS_Halt(1);
    }
}



void phase2_start_service_processes()
{
    USLOSS_Console("%s() called -- currently a NOP\n", __func__);
}

void phase3_start_service_processes()
{
    USLOSS_Console("%s() called -- currently a NOP\n", __func__);
}

void phase4_start_service_processes()
{
    USLOSS_Console("%s() called -- currently a NOP\n", __func__);
}

void phase5_start_service_processes()
{
    USLOSS_Console("%s() called -- currently a NOP\n", __func__);
}



static int check_io_CALL_COUNT = 0;
static int clockHandler_CALL_COUNT = 0;

int phase2_check_io()
{
    check_io_CALL_COUNT++;
    return 0;
}

void phase2_clockHandler()
{
    clockHandler_CALL_COUNT++;
};

void finish(int argc, char **argv)
{
    USLOSS_Console("TESTCASE ENDED: Call counts:   check_io() %d   clockHandler() %d\n", check_io_CALL_COUNT, clockHandler_CALL_COUNT);

//    USLOSS_Console("                               mmu counts: %d/%d/%d\n", mmu_init_count,mmu_quit_count,mmu_switch_count);
}



void test_setup  (int argc, char **argv) {}
void test_cleanup(int argc, char **argv) {}

