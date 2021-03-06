/*
 *  testproc.c
 
 */

#define MAIN

#include "stdio.h"
#include "assert.h"
#include "proc.h"

void testInit() {	
	initializeProcStructures();
	
	assert(running == &idleProc);
	assert(readyHead == NULL);
	assert(readyTail == NULL);
	int i=0;
	for (i=0; i<8; i++) {
		assert(memoryMap[i] == FREE);
		assert(pcbPool[i].name[0] == 0x00);
		assert(pcbPool[i].state == DEFUNCT);
		assert(pcbPool[i].segment == 0x00);
		assert(pcbPool[i].stackPointer == 0x00);
	}
	
	assert(strcmp(idleProc.name, "IDLE\0") == 0);
	assert(idleProc.segment == 0x0000);
	assert(idleProc.stackPointer == 0x0000); 
}

void testGetFreeMemorySegment() {
	int index, i;

	initializeProcStructures();

	index = getFreeMemorySegment();
	assert(index == 0);
	assert(memoryMap[index] == USED);

	for(i = 0; i < 8; i++) {
		memoryMap[i] = USED;
	}
	index = getFreeMemorySegment();
	assert(index == -1);

}

void testReleaseMemorySegment() {
	int index;

	initializeProcStructures();

	index = getFreeMemorySegment();
	assert(memoryMap[index] == USED);
	releaseMemorySegment(index);
	assert(memoryMap[index] == FREE);
}

void testGetFreePCB() {
	struct PCB *pcb;
	int i;

	initializeProcStructures();

	pcb = getFreePCB();

	assert(pcb->state == STARTING);
	assert(pcbPool[0].state == STARTING);

	for(i = 0; i < 8; i++) {
		pcbPool[i].state = STARTING;
	}
	pcb = getFreePCB();
	assert(pcb == NULL);
}

void testReleasePCB() {
	struct PCB *pcb;

	initializeProcStructures();

	pcb = getFreePCB();
	
	releasePCB(pcb);
	assert(pcb->name[0] == 0x00);
	assert(pcb->state == DEFUNCT);
	assert(pcb->next == NULL);
	assert(pcb->prev == NULL);
}

void testAddToReady() {
	struct PCB *pcb, *pcb1;

	initializeProcStructures();

	pcb = getFreePCB();
	pcb1 = getFreePCB();

	addToReady(pcb);
	assert(pcb->next == NULL);
	assert(pcb->prev == NULL);
	assert(pcb == readyHead);
	assert(pcb == readyTail);

	addToReady(pcb1);
	assert(pcb->next == pcb1);
	assert(pcb1->prev == pcb);
	assert(pcb1->next == NULL);
}

void testRemoveFromReady() {
	struct PCB *head, *pcb;

	initializeProcStructures();

	head = removeFromReady();
	assert(head == NULL);

	pcb = getFreePCB();
	addToReady(pcb);

	head = removeFromReady();
	assert(head == pcb);
}

int main() {
	printf("Testing initializeProcStructures\n");
	testInit();
	printf("Testing getFreeMemorySegment\n");
	testGetFreeMemorySegment();
	printf("Testing releaseMemorySegment\n");
	testReleaseMemorySegment();
	printf("Testing getFreePCB\n");
	testGetFreePCB();
	printf("Testing releasePCB\n");
	testReleasePCB();
	printf("Testing addToReady\n");
	testAddToReady();
	printf("Testing removeFromReady\n");
	testRemoveFromReady();
	printf("done\n");	
}
