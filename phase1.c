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
 * TODO: disable and restore interrupts for every function
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

// Dummies
void phase2_create_service_processes() {} 
void phase3_create_service_processes() {}
void phase4_create_service_processes() {}
void phase5_create_service_processes() {}

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

}
// Dummies
void phase2_init() {}
void phase3_init() {}
void phase4_init() {}
void phase5_init() {}


void startProcesses(){
    assertKernelMode();
    PCB* initPCB= makeDefaultPCB("init", 6);
    // make a context for init
    // function? init()?
    // TODO: make a wrapper function??
    void (*init_ptr)(int) = &init;
    USLOSS_Context * newContext = malloc(sizeof(USLOSS_Context));
    USLOSS_ContextInit(newContext, NULL /*dummy stack?*/, USLOSS_MIN_STACK, NULL/*dummy page table???*/, init_ptr);
    initPCB->context = newContext;

    processTable[1]= init;
    processTable_size++;
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
    if (processTable_size >= MAXPROC 
            || priority<1 
            || (priority>5 && strcmp(name, "sentinel")!=0) 
            || startFunc==NULL 
            || name==NULL 
            || strlen(name)>MAXNAME){
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
        USLOSS_Console("ERROR: dispatcher(): no runnable process found, not even sentinel().");
        dumpProcesses();
        dumpPriorityArray();
        // assert(0);
    }
    USLOSS_Console("Dispatcher: found %s\n", nextProcess->name);
    if (nextProcess != current) {
        USLOSS_Console("Performing Context Switch...\n");
        if (current == NULL) {
            USLOSS_ContextSwitch(NULL, nextProcess->context);    
        } else {
            USLOSS_ContextSwitch(current->context, nextProcess->context);
        }
        USLOSS_Console("Context switch success.\n");
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
    memset(priorityArray, 0, sizeof(priorityArray));
    // iterate through the Process Table and add each PCB* to priorityArray
    int count = 0;
    int i = 0;
    for (i = 0; i < MAXPROC; i++) {
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
        count++;
    }
    USLOSS_Console("dispatchBuildArray: iterated %d, added %d\n", i, count); // TODO remove
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
 * For Debugging:
 * prints info on the dispatcher's priorityArray
 */
void dumpPriorityArray() {
    USLOSS_Console("~~~~~~~ Priority Array Info ~~~~~~~\n");
    for (int i = 1; i <= 7; i++) {
        USLOSS_Console("Priority Slot (i): %d\n", i);
        ListNode * tempNode = priorityArray[i];
        while (tempNode != NULL) {
            USLOSS_Console("Process: %s\n", tempNode->node->name);
            USLOSS_Console("Priority: %d\n", tempNode->node->priority);
            USLOSS_Console("PID: %d\n", tempNode->node->pid);
            USLOSS_Console("\n");
            tempNode = tempNode->next;
        }
    }
    USLOSS_Console("Done printing priorityArray.\n");
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
            USLOSS_Console("Slot in processTable: %d\n", i);
            USLOSS_Console("Process Name: %s\n", cur->name);
            USLOSS_Console("PID %d\n", cur->pid);
            // is this even possible? probably not lol
            if (cur->parent != NULL)
                USLOSS_Console("Parent name: %d\n", cur->parent->name);
            else 
                USLOSS_Console("No parent.\n");
            USLOSS_Console("Priority: %d\n", cur->priority);
            USLOSS_Console("Status: %d\n", cur->status);
            USLOSS_Console("IsDead: %d\n", cur->isDead);
            USLOSS_Console("firstChild: %s\n", cur->firstChild);
            // todo: add number of children?
            USLOSS_Console("CPU time consumed: %d\n", cur->totalTime);
        }
   }
   USLOSS_Console("Done printing processTable.\n");
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