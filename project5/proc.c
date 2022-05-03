#include "proc.h"
#include "string.h"

/*
 * This function initializes all of the global variables and 
 * structures defined above.  All entries in the memory map should be marked as FREE.
 * All of the PCBs in the pcbPool should have the first character of their 
 * names set to null, their state set to DEFUNCT and their segment and stack
 * pointers set to 0x0000. The PCB for the idle process should be initialized 
 * with the name IDLE, the state READY, and both the segment and stack pointers
 * set to 0x0000.  running should refer to the PCB for the idle process and the ready queue
 * should be empty.
 */  
void initializeProcStructures() {
    int i, j;

    // marks all entries in the memory map as FREE
    for(i = 0; i < 8; i++) {
        memoryMap[i] = FREE;
    }

    // sets the values of all PCBs in the pcbPool
    for(j = 0; j < 8; j++) {
        pcbPool[j].name[0] = NULL;
        pcbPool[j].state = DEFUNCT;
        pcbPool[j].segment = 0x0000;
        pcbPool[j].stackPointer = 0x0000;
    }

    idleProc.name[0] = 'I';
    idleProc.name[1] = 'D';
    idleProc.name[2] = 'L';
    idleProc.name[3] = 'E';
    idleProc.name[4] = '\0';
    idleProc.state = READY;
    idleProc.segment = 0x0000;
    idleProc.stackPointer = 0x0000;

    running = &idleProc;

    readyHead = NULL;
    readyTail = NULL;
}

/*
 * Returns the index of the first free memory segment or -1 if 
 * all of the memory segments are used.  The returned memory segment
 * should be marked as used.
 */
int getFreeMemorySegment() {
    int i = 0;

    for(i = 0; i < 8; i++) {
        if(memoryMap[i] == FREE) {
            memoryMap[i] = USED;
            return i;
        }
    }

    return -1;
}

/*
 * Release the memory segment indicated by the given index into
 * the memory map.
 */
void releaseMemorySegment(int seg) {
    memoryMap[seg] = FREE;
}

/*
 * Return a pointer to an available PCB from the PCB pool.  All of the
 * available PCBs should be marked as DEFUNCT.  The returned PCB should
 * be marked as STARTING. Return NULL if there are no PCBs available.
 */
struct PCB *getFreePCB() {
    int i;

    for(i = 0; i < 8; i++) {
        if(pcbPool[i].state == DEFUNCT) {
            pcbPool[i].state = STARTING;
            return &pcbPool[i];
        }
    }

    return NULL;
}

/*
 * Release the provided PCB by setting its state to DEFUNCT, setting
 * its next and prev pointers to NULL and setting the first character
 * of its name to 0x00.
 */
void releasePCB(struct PCB *pcb) {
    pcb->state = DEFUNCT;
    // pcb->prev->next = pcb->next;
    pcb->next = NULL;
    pcb->prev = NULL;
    pcb->name[0] = NULL;
}

/*
 * Add the provided PCB to the tail of the ready queue.
 */
void addToReady(struct PCB *pcb) {
    if(readyHead == NULL) {
        readyHead = pcb;
        readyTail = pcb;
        pcb->next = NULL;
        pcb->prev = NULL;
    } else {
        readyTail->next = pcb;
        pcb->prev = readyTail;
        pcb->next = NULL;
        readyTail = pcb;
    }
}

/*
 * Remove the PCB at the head of the ready queue and return a 
 * pointer to it.
 */
struct PCB *removeFromReady() {
    struct PCB *head;

    if(readyHead == NULL) {
        return NULL;
    } else if(readyHead == readyTail) {
        head = readyHead;
        readyHead = NULL;
        readyTail = NULL;
        return head;
    } else {
        head = readyHead;
        readyHead = readyHead->next;
        readyHead->prev = NULL;
        return head;
    }
}