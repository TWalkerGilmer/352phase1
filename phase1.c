#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "phase1.h"

typedef struct PCB{
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
    // 0 for not dead (default value), or 1 if it has died
    int isDead;
    USLOSS_Context * context;
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
void static clockHandler(int dev, void *args);
void dispatchHelper_buildArray();
PCB* dispatchHelper_findNextProcess();
PCB* getProcess(int);
int sentinel(char* nothing);

// int USLOSS_IntVec[] = malloc(sizeof(void*) * USLOSS_NUM_INTS);
// USLOSS_IntVec[USLOSS_CLOCK_INT] = clockHandler; // TODO

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
    if ((USLOSS_PSR_CURRENT_MODE & USLOSS_PsrGet() )== 0){
        return 1;
    }
    return 0; 
 }

/*
 * If we're not in kernel mode, prints an error message and halts usloss.
 */
 void assertKernelMode() {
    if ((USLOSS_PSR_CURRENT_MODE & USLOSS_PsrGet() )== 0){
        USLOSS_Console("Error: Not in kernel mode.");
        USLOSS_Halt(1);
    }
 }

/**
 * Disables interrupts.
*/
void disableInterrupts() {
    assertKernelMode();
    // mask to set the interrupts to 0 (disabled)
    USLOSS_PsrSet(USLOSS_PsrGet() & ~USLOSS_PSR_CURRENT_INT);
}

/**
 * Restores interrupts to previous state
*/
void restoreInterrupts() {
    assertKernelMode();
    // set the interrupts to its previous state
    // gets the previous interrupt value and right shifts
    int new_mask= (USLOSS_PsrGet() & USLOSS_PSR_PREV_INT) >> 2;
    USLOSS_PsrSet(USLOSS_PsrGet() | new_mask);
}

void init() {
    //bootstrapping
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

int testcase_main_caller(char* nothing){
    // call other function (provided by testcase?)
    return 0;
}

int sentinel(char* nothing){
    assertKernelMode();
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

PCB * getProcess(int num) {
    // TODO : this is a placeholder
    return 0;
}

int fork1(char *name, int (*startFunc)(char*), char *arg, int stackSize, int priority){
    assertKernelMode();
    if (stackSize<USLOSS_MIN_STACK){
        return -2;
    }
    if (processTable_size >= MAXPROC || priority<1 || (priority>5 && strcmp(name, "sentinel")!=0) || startFunc==NULL || name==NULL || strlen(name)>MAXNAME){
        return -1;
    }
    PCB* child= malloc(sizeof(PCB));
    child->name= name;
    child->pid= PID;
    PID++;
    child-> status=0; // runnable
    child->priority= priority;
    child->parent = current;
    child->firstChild = NULL;
    child->nextChild  = NULL;
    // child->startTime = ??? // TODO
    // child->totalTime = ??? // TODO
    child->zapped = 0;
    child->isDead = 0;


    // Make a Wrapper Function
    // TODO how?

    // Initiate the Context!
    // TODO: make a Wrapper function
    USLOSS_Context * newContext = malloc(sizeof(USLOSS_Context));
    void * newStack = malloc(sizeof(stackSize));
    // TODO use the wrapper function
    USLOSS_ContextInit(newContext, newStack, stackSize, NULL/*dummy page table???*/, startFunc(arg));

    // add child to PCB
    int slotIndex = child->pid % MAXPROC;
    int startingIndex = slotIndex;
    while (processTable[slotIndex] != NULL) {
        slotIndex++;
        if (slotIndex >= MAXPROC) {
            slotIndex = slotIndex - MAXPROC;
        }
        if (slotIndex = startingIndex) {
            // processTable is full, failure to add
            USLOSS_Console("Process Table is full, cannot add process.");
            return -1;
        }
    }
    processTable[slotIndex] = child;

    mmu_init_proc(child->pid); // dummy function, obligatory

    // add child to current process (parent)
    fork1Helper_addChild(child);

    // TODO: wrapper function
    dispatcher();

    return child->pid;
}

/*
 * Adds a child to the list of children for the current process.
 */
void fork1Helper_addChild(PCB * child) {
    if (current->firstChild == NULL) {
        current->firstChild = child;
    } else {
        PCB * tempChild = current->firstChild;
        while (tempChild->nextChild != NULL) {
            tempChild = tempChild->nextChild;
        }
        tempChild->nextChild = child;
    }
}

/*
 * Wrapper Function
 */ 
void fork1Helper_wrapperFunction (void * (startFunc)(char* arg)) {
    // TODO
}

/*
 * Called by a parent,
 * Blocks the parent until one of its children dies.
 * Removes the child from the processTable and from its family tree.
 * Returns the dead child's PID.
 */
int join(int *status){
    assertKernelMode();
    if (current->firstChild == NULL) {
        return -2;
    }

    // search list of children starting at current->firstChild
    PCB* child = current->firstChild;
    while (child != NULL) {
        if (child->isDead) {

            break;
        }
        child = child->nextChild;
    }

    // if all children are still alive:
    if (child == NULL) {
        // block current process
        blockMe(11); // blockMe calls the dispatcher, 11 is a generic status for blocked
        // search again, find the child
        PCB* child = current->firstChild;
        while (child != NULL) {
            if (child->isDead) {
                break;
            }
            child = child->nextChild;
        }
        if (child == NULL) {
            // This should never happen
            USLOSS_Console("ERROR: join() fail, no dead child found after blocking parent");
            assert(0);
        }
    }
    // you have found the dead child
    // set parameter *status to exit status of the child
    status = &(child->exitStatus);
    int returnValue = child->pid;
    // remove child from the processTable
    for (int i = 0; i < MAXPROC; i++) {
        if (processTable[i] == child) {
            processTable[i] = NULL;
            processTable_size--;
            break;
        }
    }
    // remove child from list of children
    if (current->firstChild->isDead) {
        current->firstChild = current->firstChild->nextChild;
    } else {
        child = current->firstChild;
        PCB* prevChild = child;
        while (child->nextChild != NULL) {
            child = child->nextChild;
            if (child->isDead) {
                prevChild->nextChild = child->nextChild;
                break;
            }
            prevChild = child;
        }
    }
    return returnValue;
}

/**
 * The Dispatcher:
 * builds a new priorityArray each time it is called, based on the processTable.
 * Uses two helper functions, below.
 */
void dispatcher() {
    assertKernelMode();
    disableInterrupts();
    // this function builds a new priority array each time it is called
    dispatchHelper_buildArray();
    // search for first runnable process according to priority
    PCB * nextProcess = dispatchHelper_findNextProcess();
    if (nextProcess == NULL) {
        // This should never happen
        USLOSS_Console("ERROR: dispatcher(): no runnable process found, not evern sentinel().");
        assert(0);
    }
    if (nextProcess != current) {
        USLOSS_ContextSwitch(current->context, nextProcess->context);
        // TODO: What about time slicing? (I'd assume we switch if current process has exceeded its time slice)
    }
    restoreInterrupts();
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
    memset(priorityArray, 0, sizeof(priorityArray));
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
            if (head->node->status == 0 
                    && head->node->isDead == 0) {
                return head->node;
            }
            head = head->next;
        }
    }
    return NULL;
}

/*
 * Called by a process,
 * Terminates the process and unblocks its parent
 */
void quit(int status) {
    assertKernelMode();
    // if parent is blocked because of join(), unblock them
    current->exitStatus = status;
    current->isDead = 1;
    if (current->parent->status == 11) {
        current->parent->status = 0;
    }
    dispatcher();
    // TODO: unblock anything trying to zap() the current process???

    // NOTE: this function should never return
}

int zap(int pid){
    assertKernelMode();
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
    assertKernelMode();
    return current->zapped;
}

int getpid(void){
    assertKernelMode();
    return current->pid;
}

/*
 * Prints out info on the Process Table, for debugging
 */
void dumpProcesses(){
    assertKernelMode();
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
    assertKernelMode();
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
    assertKernelMode();
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
    assertKernelMode();
    return current->startTime;
}

/*
 * Returns current wall-clock time.
*/
int currentTime(){
    assertKernelMode();
    // todo: what argumenst are needed in USLOSS function?
    // int result = USLOSS_DeviceInput(USLOSS_CLOCK_DEV, );
    // return result;
    return 0; // temp
}

/*
 * Returns total CPU time of the process since it was created.
 */
int readtime(){
    assertKernelMode();
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
    assertKernelMode();
    int t= currentTime();
    int st= readCurStartTime();
    if (t- st >=80){
        current->totalTime+=t-st;
        dispatcher();
    }
}