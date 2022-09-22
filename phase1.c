#include "phase1.h"

typedef struct PCB{
    char* name;
    int pid;
    // 0 for runnable, greater for blocked
    int status;
    int priority;
    ListNode* firstChild;
    PCB* parent;
    int startTime;
    // might be necessary
    int totalTime;
    // 1== is zapped
    int zapped;
}PCB;

typedef struct ListNode{
    PCB* node;
    ListNode* next;
}ListNode;

PCB * processTable[MAXPROC];
int PID=1;
int processTable_size=0;
PCB* current;

// Priority Array - what the dispatcher uses.
// The first slot of the array (#0) is not used.
ListNode * priorityArray[8];

// todo: make functions to enable/disable interrupts
void init(){

}

void sentinel(){
}

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


void startprocesses(){
    dispatcher();
}

int fork1(char *name, int (*startFunc)(char*), char *arg, int stackSize, int priority){
    if (stackSize<USLOSS_MIN_STACK){
        return -2;
    }
    if (processTable_size== MAXPROC || priority<1 || priority>7|| startFunc==NULL || name==NULL || strlen(name)>MAXNAME){
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
    add_child(child);
    // todo: wrapper function
    dispatcher();
}


int join(int *status){
    if (current->firstChild == NULL){
        return -2;
    }
    
}

void dispatcher () {
    // uses our Priority Array - an array of size 7 that holds linked lists of PCBs
    // this function builds a new priority array each time it is called

    *priorityArray = dispatchHelper_buildArray();
    // search for first runnable process according to priority
    PCB * nextProcess = dispatchHelper_findNextProcess();




    // if this process is currently running: keep it running (?)
    // else: switch

    dumpProcesses(); // print Process Table info for debugging

    // TODO: switch if there is no current process
    // check if current process should keep running or switch
    // perform switch if necessary
        // search for any runnable process
        // switch to it by contextswitch

}

/* 
 * Builds the priorityArray based on the current Process Table,
 * considering each process's priority value
 */
ListNode * dispatchHelper_buildArray() { // TODO should this return a double pointer?
    PCB * lastNodeP1 = 0;
    PCB * lastNodeP2 = 0;
    PCB * lastNodeP3 = 0;
    PCB * lastNodeP4 = 0;
    PCB * lastNodeP5 = 0;

    // iterate through the Process Table
    for (int i = 0; i < MAXPROC; i++) {

    }
        // link each to the end of its place in the array
    return 0;
}

/*
 * Uses the Priority Array to determine the next process to call
 */
PCB * dispatchHelper_findNextProcess() {
    for (int priority = 1; priority <= 5; priority++) {
        ListNode * head = priorityArray[priority];
        while (head != 0) {
            // Q: how many statuses do we have?
            // TODO: if a process is runnable (e.g. not blocked), return its PCB
            head = head->next;
        }
    }
    // TODO: for priority 6 and 7, return init or sentinel?
}

void quit(int status){
    // write status out to PCB
    // if parent is blocked, wake them

    // tell dispatcher that there is no currently running process
}

int zap(int pid){
    Process* item= getProcess(pid);
    if (pid==1 || pid== current->pid || item== null){
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
        Process* cur= processTable[i];
        if (cur != NULL){
            USLOSS_CONSOLE("Process Name: %s\n", cur->name);
            USLOSS_CONSOLE("PID %d\n", cur->pid);
            // is this even possible? probably not lol
            USLOSS_CONSOLE("Parent PID: %d\n", cur->parent->pid);
            USLOSS_CONSOLE("Priority: %d\n", cur->priority);
            USLOSS_CONSOLE("Status: %d\n", cur->status);
            // todo: add number of children
            USLOSS_CONSOLE("CPU time consumed: %d", cur->totalTime);
        }
   }
}

int blockMe(int newStatus){
    if (newStatus<10){
        USLOSS_CONSOLE("Error: status message incorrect.");
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
    Process* item= getProcess(pid);
    // checking status
    if (item== void || item->status<2|| item->status<=10 ){
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
    return USLOSS_DeviceInput(USLOSS_CLOCK_DEV, 0, 0);
}

int readTime(){
}

void timeSlice(){
    int t= currentTime();
    int st= readCurStartTime();
    if (t- st >=80){
        dispatcher();
    }
}
