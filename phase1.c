#include <stdlib.h>
#include <string.h>
#include "phase1.h"

typedef struct PCB{  // this needs contexts
    char *name;
    int pid;
    // 0 for runnable, >10 for blocked
    int status;
    int priority;
    struct PCB* firstChild;
    struct PCB* parent;
    struct PCB* nextChild;
    int startTime;
    // might be necessary
    int totalTime;
    // 1== is zapped
    int zapped;
    // This is set when the process calls quit() to terminate itself:
    int exitStatus;
    int isDead;
    USLOSS_Context *context;
} PCB;

typedef struct ListNode{
    PCB* node;
    struct ListNode* next;
}ListNode;

PCB *processTable[MAXPROC];
int PID=1;
int processTable_size=0;
PCB* current;


// todo: add function pointer
// Priority Array - what the dispatcher uses.
// The first slot of the array (#0) is not used.
ListNode * priorityArray[8];

// Function Delarations
void dispatcher();
void clockHandler();
void dispatchHelper_buildArray();
PCB* dispatchHelper_findNextProcess();
PCB* getProcess(int); // TODO: unmade

USLOSS_IntVec[USLOSS_CLOCK_INT]= &clockHandler;

/*
 * According to the spec, every function in Phase1 needs to disable interrupts at start,
 * and "Restore" interrupts before returning,
 * Additionally, Phase1 is only supposed to run in kernel mode, so each function
 * has to check if kernel mode is enabled, otherwise return an error immediately.
 * These checks can both be done by accessing the PSR, with: 
 * unsigned int USLOSS_PsrGet();   and 
 * void USLOSS_PsrSet(unsigned int);
 */


/*
 * Makes a new PCB with default values
*/
PCB* makeDefaultPCB(char* name, int p){
    PCB* item= malloc(sizeof(PCB));
    item->name= name;
    item->pid= PID;
    PID++;
    item->status= 0;
    item->priority= p;
    item->startTime=0;
    item->totalTime;
    item->zapped= 0;
    item->exitStatus= 0;
    item->context= NULL;
    item->firstChild= NULL;
    item->nextChild= NULL;
    item->parent= NULL;
    item->isDead= 0;
    return item;
}

/*
 * This function checks if we are in kernel or user mode.
 * Return:
 * 1 if not in kernel mode
 * 0 if in kernel mode
*/
 int checkKernelMode(){
    // mask psr to get kernel mode status value
    if ((USLOSS_PSR_CURRENT_MODE & USLOSS_PsrGet())== 0){
        return 1;
    }
    return 0;
 }

/**
 * Disables interrupts.
*/
void disableInterrupts() {
    if (checkKernelMode()==1){
        USLOSS_Console("Error: Not in kernel mode.")
        USLOSS_Halt(1);
    }
    else{
        // mask to set the interrupts to 0 (disabled)
        USLOSS_PsrSet(USLOSS_PsrGet() & ~USLOSS_PSR_CURRENT_INT)
    }

}

/**
 * Restores interrupts to previous state
*/
void restoreInterrupts() {
    if (checkKernelMode()== 1){
        USLOSS_Console("Error: Not in kernel mode.")
        USLOSS_Halt(1);
    }
    else{
        // set the interrupts to its previous state
        // gets the previous interrupt value and right shifts
        int new_mask= (USLOSS_PsrGet() & USLOSS_PSR_PREV_INT) >> 2;
        USLOSS_PsrSet(USLOSS_PsrGet() | new_mask);
    }
}

void init() {
    //boostrapping
    phase2_create_service_processes();
    phase3_create_service_processes();
    phase4_create_service_processes();
    phase5_create_service_processes();
    //call fork1() two times, create sentinel and testcase_main
    fork1("sentinel", &sentinel, "", USLOSS_MIN_STACK, 7);
    fork1("testcase_main", &testcase_main, "", USLOSS_MIN_STACK, 5);
    while (1){
        int temp=1;
        int check= join(&temp);
        if (check==-2){
            USLOSS_Console("Error: No children left.");
            USLOSS_Halt(1);
        }
    }
}

int testcase_main(char* nothing){
    // call other function (provided by testcase?)
    return 0;
}

int sentinel(char* nothing){
    if (checkKernelMode()== 1){
        USLOSS_Console("Error: Not in kernel mode.")
        USLOSS_Halt(1);
    }
    while(1){
        if (phase2_check_io() == 0){
            USLOSS_Console("Error: Deadlock detected");
            USLOSS_Halt(2);
        }
        USLOSS_WaitInt();
    }
}

/*
 * From the spec:
 * "This will be called exactly once, when the simulation starts up. Initialize
 * your data structures, including setting up the process table entry for the
 * starting process, init.
 * Although you will create a process table entry for init, you must not run
 * it (yet)."
 */
void phase1_init(){
    PCB* init= makeDefaultPCB("init", 6);
    processTable[1]= init;
    processTable_size++;
}


void startProcesses(){
    dispatcher();
}

int fork1(char *name, int (*startFunc)(char*), char *arg, int stackSize, int priority){
    if (stackSize<USLOSS_MIN_STACK){
        return -2;
    }
    if (processTable_size== MAXPROC || priority<1 || (priority>5 && strcmp(name, "sentinel")!=0) || startFunc==NULL || name==NULL || strlen(name)>MAXNAME){
        return -1;
    }
    if (checkKernelMode()==1){
        USLOSS_Console("Error: Not in kernel mode.")
        USLOSS_Halt(1);
    }
    PCB* child= malloc(sizeof(PCB));
    child->pid= PID;
    PID++;
    child->name= name;
    child->priority= priority;
    child-> zapped=0;
    child-> status=0;
    child-> parent= current;
    // todo: add pointer to PCB
    int slot= PID%MAXPROC;
    // todo: need to check if slot is in use
    processTable[slot]= child;
    mmu_init_proc(child->pid);
    // todo: make function to add a child to a current process
    // add_child(child);
    // todo: wrapper function
    dispatcher();

    // TODO: this is a temporary return value
    return 0;
}

int join(int *status){
    if (current->firstChild == NULL) {
        return -2;
    }
    if (checkKernelMode()==1){
        USLOSS_Console("Error: Not in kernel mode.")
        USLOSS_Halt(1);
    }
    // TEMP
    return 0;
    // TEMP

    // search list of children starting at current->firstChild
    PCB* child = current->firstChild;
    while (child != NULL) {
        if (child->status == 2) { // if child is dead (status 2 equals dead)
            // TODO: AND if child has not been joined before ???
            break;
        }
        child = child->nextChild;
    }
    // if all children are blocked:
    if (child == NULL) {
        // block current process
        blockMe(11);
        // search again, find the child
        PCB* child = current->firstChild;
        while (child != NULL) {
            if (child->status == 2) { // if child is dead (status 2 equals dead)
                // TODO: AND if child has not been joined before ???
                break;
            }
            child = child->nextChild;
        }
        if (child == NULL) {
            USLOSS_Console("ERROR: join() fail");
        }
    }
    // you have found the dead child
    // set parameter *status to exit status of the child
    // status = &child->returnValue;
    // return found child's PID value
}

void dispatcher() {
    // TODO: block interrupts
    // TODO: look for a process with STATUS == 0

    // this function builds a new priority array each time it is called
    dispatchHelper_buildArray();
    // search for first runnable process according to priority
    PCB * nextProcess = dispatchHelper_findNextProcess();

    if (nextProcess != current) {
        // TODO: Context Switch
        // USLOSS_ContextSwitch(USLOSS_Context *old_context, USLOSS_Context *new_context);
        // TODO: What about time slicing?
    }

    // print Process Table info for debugging (why?)
    dumpProcesses();

    // TODO: "Restore" interrupts
}

/* 
 * Builds the priorityArray based on the current Process Table,
 * considering each process's priority value
 * NOTE: The first slot of the array (#0) is not used.
 */
void dispatchHelper_buildArray() {
    // NOTE: This runs in O(n^2) and could be updated to run in O(n), but n<50 so whatever
    // clear the priorityArray
    ListNode * priorityArray[8];
    // iterate through the Process Table and add each PCB* to priorityArray
    for (int i = 0; i < MAXPROC; i++) {
        if (processTable[i] == NULL) {
            continue;
        }
        // make a new node to hold the PCB* from processTable
        ListNode * newNode = malloc(sizeof(ListNode));
        // determine where to put this PCB* based on its own priority
        int priorityNum = processTable[i]->priority;
        ListNode * tempNode = priorityArray[priorityNum];
        if (tempNode == NULL) {
            priorityArray[priorityNum] = newNode;
        } else {
            while (tempNode->next != NULL) {
                tempNode = tempNode->next;
            }
            tempNode->next = newNode;
        }
        newNode->node = processTable[i];
        newNode->next = NULL;
    }
}

/*
 * Uses the Priority Array to determine the next process to call
 * NOTE: The first slot of the array (#0) is not used.
 */
PCB* dispatchHelper_findNextProcess() {
    for (int priority = 1; priority <= 7; priority++) {
        ListNode * head = priorityArray[priority];
        while (head != NULL) {
            if (head->node->status == 0) {
                return head->node;
            }
            head = head->next;
        }
    }
    return NULL; // TODO: check
}

void quit(int status){
    if (checkKernelMode()==1){
        USLOSS_Console("Error: Not in kernel mode.")
        USLOSS_Halt(1);
    }
    // write status out to PCB
    // if parent is blocked, wake them

    // tell dispatcher that there is no currently running process
    dispatcher();
}

int zap(int pid){
    if (checkKernelMode()==1){
        USLOSS_Console("Error: Not in kernel mode.")
        USLOSS_Halt(1);
    }
    PCB* item= getProcess(pid);
    if (pid==1 || pid== current->pid || item== NULL){
        USLOSS_Halt(1);
    }
    item->zapped= 1;
    // call dispatcher to update priority list
    dispatcher();    
    // check if calling process (current process was zapped)
    if (current->zapped== 1){
        return -1;
    }
    // todo: check if zapped process has called quit()
    return 0;
}

int isZapped(){
    if (checkKernelMode()==1){
        USLOSS_Console("Error: Not in kernel mode.")
        USLOSS_Halt(1);
    }
    return current->zapped;
}

int getpid(void){
    if (checkKernelMode()==1){
        USLOSS_Console("Error: Not in kernel mode.")
        USLOSS_Halt(1);
    }
    return current->pid;
}

/*
 * Prints out info on the Process Table, for debugging
 */
void dumpProcesses(){
    if (checkKernelMode()==1){
        USLOSS_Console("Error: Not in kernel mode.")
        USLOSS_Halt(1);
    }
   for (int i=0; i<MAXPROC; i++){
        PCB* cur= processTable[i];
        if (cur != NULL){
            USLOSS_Console("Process Name: %s\n", cur->name);
            USLOSS_Console("PID %d\n", cur->pid);
            // is this even possible? probably not lol
            USLOSS_Console("Parent PID: %d\n", cur->parent->pid);
            USLOSS_Console("Priority: %d\n", cur->priority);
            USLOSS_Console("Status: %d\n", cur->status);
            // todo: add number of children
            USLOSS_Console("CPU time consumed: %d", cur->totalTime);
        }
   }
}

int blockMe(int newStatus){
    if (checkKernelMode()==1){
        USLOSS_Console("Error: Not in kernel mode.")
        USLOSS_Halt(1);
    }
    if (newStatus<10){
        USLOSS_Console("Error: status message incorrect.");
        USLOSS_Halt(1);
    }
    current->status= newStatus;
    dispatcher();
    if (isZapped()){
        return -1;
    }
    return 0;
}

/*
 * Unblocks a process
 */
int unblockProc(int pid){
    if (checkKernelMode()==1){
        USLOSS_Console("Error: Not in kernel mode.")
        USLOSS_Halt(1);
    }
    // todo: make function to get an element based on pid
    PCB* item= getProcess(pid);
    // checking status
    if (item== NULL || item->status<2|| item->status<=10 ){
        return -2;
    }
    //unblock the process
    item->status= 1;
    // call dispatcher in case priority has changed.
    dispatcher();
    return 0;
}

/*
 * Returns the time at which current process began its current time slice.
*/
int readCurStartTime(){
    if (checkKernelMode()==1){
        USLOSS_Console("Error: Not in kernel mode.")
        USLOSS_Halt(1);
    }
    return current->startTime;
}

/*
 * Returns current wall-clock time.
*/
int currentTime(){
    if (checkKernelMode()==1){
        USLOSS_Console("Error: Not in kernel mode.")
        USLOSS_Halt(1);
    }
    // todo: what argumenst are needed in USLOSS function?
    int result = USLOSS_DeviceInput(USLOSS_CLOCK_DEV, );
    return result;
}

/*
 * Returns total CPU time of the process since it was created.
 */
int readtime(){
    if (checkKernelMode()==1){
        USLOSS_Console("Error: Not in kernel mode.")
        USLOSS_Halt(1);
    }
    return current->totalTime+currentTime();
}

static void clockHandler(int dev, void *arg){
    timeSlice();
    phase2_clockHandler();
}
/*
* Returns if current process exceeds its time slice. Call dispatcher if true.
*/
void timeSlice(){
    if (checkKernelMode()==1){
        USLOSS_Console("Error: Not in kernel mode.")
        USLOSS_Halt(1);
    }
    int t= currentTime();
    int st= readCurStartTime();
    if (t- st >=80){
        current->totalTime+=t-st;
        dispatcher();
    }
}
