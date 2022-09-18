#include "phase1.h"

typedef struct PCB{
    int pid;
    int status;
    int priority;
    ListNode* firstChild;
    PCB* parent;
    int time;
}PCB;

typedef struct ListNode{
    PCB* node;
    ListNode* next;
}ListNode;

PCB * processTable[MAXPROC];
int PID=1;

// Priority Array - what the dispatcher uses.
// The first slot of the array (#0) is not used.
ListNode * priorityArray[8];

void phase1_init(){
    PCB* init= malloc(sizeof(PCB));
    init->pid= PID;
    init->priority= 6;
    processTable[1]= init;
}


void startprocesses(){
}

int fork1(char *name, int (*startFunc)(char*), char *arg, int stackSize, int priority){
}

int join(int *status){
    // TODO - this is a placeholder

    return 0;
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
}

int isZapped(){
}

int getpid(void){
}

/*
 * Prints out info on the Process Table, for debugging
 */
void dumpProcesses(){

}

int blockMe(int newStatus){
}

/*
 * Unblocks a process
 */
int unblockProc(int pid){
}

int readCurStartTime(){
}

int currentTime(){
}

int readTime(){
}

void timeSlice(){
}