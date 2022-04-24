/*
 * The Process Control Block
 */
struct PCB {
	char name[7];  		// 6 chars for filename + 1 for null terminator.
	int state;		// One of the constants above.
	int segment;		// memory segment occupied by the process.
	int stackPointer;	// address of the stack pointer for the process.
	
	/*
	 * Pointers to allow PCB's to be stored in doubly linked lists.
 	 */
	struct PCB *next;
	struct PCB *prev;
};

