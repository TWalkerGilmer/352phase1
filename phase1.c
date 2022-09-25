#include <stdlib.h>
#include <string.h>
#include "phase1.h"


typedef struct PCB{  // this needs contexts
    char *name;
    int pid;
    // 0 for runnable, greater for blocked
    int status;
    int priority;
    struct PCB* firstChild;
    struct PCB* parent;
    int startTime;
    // might be necessary
    int totalTime;
    // 1== is zapped
    int zapped;
    USLOSS_Context context;
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
void dispatchHelper_buildArray();
PCB* dispatchHelper_findNextProcess();
PCB* getProcess(int); // TODO: unmade


/*
 * According to the spec, every function in Phase1 needs to disable interrupts at start,
 * and "Restore" interrupts before returning,
 * Additionally, Phase1 is only supposed to run in kernel mode, so each function
 * has to check if kernel mode is enabled, otherwise return an error immediately.
 * These checks can both be done by accessing the PSR, with: 
 * unsigned int USLOSS_PsrGet();   and 
 * void USLOSS_PsrSet(unsigned int);
 */

void disableInterrupts() {
    // int prevPSR = USLOSS_PsrGet();
    // USLOSS_PsrSet(???);
}

void restoreInterrupts() {
    
}

void init() {
    dispatcher();
}

void sentinel(){
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
    PCB* init= malloc(sizeof(PCB));
    // add the main function pointer
    char* n= "init";
    init->name= n;
    init->pid= PID;
    init->priority= 6;
    init->status= 0;
    init-> zapped= 0;
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
    // TODO: I think this priority check should be priority>5 instead of priority>7
    if (processTable_size== MAXPROC || priority<1 || priority>7 || startFunc==NULL || name==NULL || strlen(name)>MAXNAME){
        return -1;
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
    
    // TEMP
    return 0;
    // TEMP

    // search list of children starting at current->firstChild
    // if all children are blocked:
        // block current process:
            // change current->status to blocked (this would automatically be resolved upon termination of a child)
        // call dispatcher();
        // search again, find the child
        // set parameter *status to exit status of the child
        // return found child's PID value
    // else
        // find the dead child
        // set parameter *status to exit status of the child
        // return found child's PID value
}

void dispatcher() {
    // TODO: block interrupts?

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
    // write status out to PCB
    // if parent is blocked, wake them

    // tell dispatcher that there is no currently running process
    dispatcher();
}

int zap(int pid){
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
    return current->zapped;
}

int getpid(void){
    return current->pid;
}

/*
 * Prints out info on the Process Table, for debugging
 */
void dumpProcesses(){
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

int readCurStartTime(){
    return current->startTime;
}

int currentTime(){
    // todo: what argumenst are needed in USLOSS function?
    int result = USLOSS_DeviceInput(USLOSS_CLOCK_DEV, 0, 0);
    return result;
}

int readTime(){
    // TODO: this is a placeholder
    return 0;
}

void timeSlice(){
    int t= currentTime();
    int st= readCurStartTime();
    if (t- st >=80){
        dispatcher();
    }
}
