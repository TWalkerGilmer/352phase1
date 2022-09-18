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

PCB processTable[MAXPROC];
int PID=1;
int processTable_size=0;
PCB* current;

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

void quit(int status){
    
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
