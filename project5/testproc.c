/*
 *  testproc.c
 
 */

#define MAIN

#include "stdio.h"
#include "assert.h"
#include <string.h>
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

void testReleasePCB() { //segmentation fault
	struct PCB *head, *middle, *tail;

	head->next = middle;
	middle->prev = head;
	middle->next = tail;
	tail->prev = middle;

	middle->name[0] = 'a';
	middle->state = STARTING;

	releasePCB(middle);
	assert(middle->name[0] == 0x00);
	assert(middle->state == DEFUNCT);
	assert(middle->next == NULL);
	assert(middle->prev == NULL);
}

void testAddToReady() {

}

void testRemoveFromReady() {

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
	printf("done\n");	
}
