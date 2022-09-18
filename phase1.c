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

PCB processTable[MAXPROC];
int PID=1;

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
}

void quit(int status){
}

int zap(int pid){
}

int isZapped(){
}

int getpid(void){
}

void dumpProcesses(){
}

int blockMe(int newStatus){
}

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