#define MAIN
#include "proc.h"

void putChar(int row, int column, char c, char color);
void putStr(int row, int column, char string[], char color);
int printString(char *str);
int readChar();
int readString(char *buf);
int readSector(char *buf, int absSector);
int mod(int a, int b);
int handleInterrupt21(int ax, int bx, int cx, int dx);
void printInt(int i);
int readfile(char *filename, char *buf);
int findFile(struct directory *dir, char *filename, char *buf);
int fileNameLen(char *filename);
int executeProgram(char *filename);
void terminate();
int writeSector(char *buffer, int sector);
int deleteFile(char *fname);
int writeFile(char *filename, char *buffer, int sectors);
int readText(char *buf);
void handleTimerInterrupt(int segment, int stackPointer);
void kStrCopy(char *src, char *dest, int len);
void yield();
void showProcesses();
int kill(int segment);
void printHello();

typedef char byte;

struct dirEntry {
    char name[6];
    byte sectors[26];
};

struct directory {
    struct dirEntry entries[16];
};


int main() {
    setKernelDataSegment();
    initializeProcStructures();
    restoreDataSegment();

    makeInterrupt21();
    interrupt(0x21, 0x04, "shell\0", 0, 0);
    makeTimerInterrupt();

    while(1);
}

// places a character at a specifc location on the screen given
// some row and column value
// also selects the character color and background color for the character
void putChar(int row, int column, char c, char color) {
    int segment = 0xB000;
    int offset = 0x8000 + (80 * (row - 1) * 2) + ((column - 1) * 2);
    putInMemory(segment, offset, c);
    putInMemory(segment, offset + 1, color);
}

// places a string at some row and column value on the screen
// and selects the character colors and background color
void putStr(int row, int column, char string[], char color) {
    int i = 0;

    while(string[i] != '\0') {
        if(column > 80) {
            row = row + 1;
            column = 1;
            putChar(row, column, string[i], color);
        } else {
            putChar(row, column, string[i], color);
            column += 1;
        }

        i += 1;
    }
}

// calls an interrupt to print a char array onto the screen
int printString(char *str) {
    int i = 0;
    while(str[i] != '\0') {
        char al = str[i];
        char ah = 0x0E;
        int ax = ah * 256 + al;
        interrupt(0x10, ax, 0, 0, 0);
        i++;
    }
    return i;
}

// calls an interrupt to read a single character from the keyboard
int readChar() {
    return interrupt(0x16, 0x00, 0, 0, 0);
}

// reads input from the keyboard and saves it into a char array until
// the enter key is pressed
// also displays each key that is pressed so the user knows what they
// are typing
int readString(char *buf) {
    int i = 0;
    char ch;
    char ah = 0x0E;
    int ax;

    do {
        ch = readChar();
        if (ch == 0x08) {
            if(i != 0) {
                buf[i - 1] = 0x20;
            }
            ax = ah * 256 + ch;
            interrupt(0x10, ax, 0, 0, 0);

            ax = ah * 256 + 0x20;
            interrupt(0x10, ax, 0, 0, 0);

            ax = ah * 256 + ch;
            interrupt(0x10, ax, 0, 0, 0);
            i -= 1;
        } else {
            buf[i] = ch;
            ax = ah * 256 + ch;
            interrupt(0x10, ax, 0, 0, 0);
            i += 1;
        }
    } while (ch != 0x0D);

    buf[i] = 0x00;

    return buf;
}

// self defined mod function
int mod(int a, int b) {
    while(a > b) {
        a = a - b;
    }

    return a;
}

// reads from a sector on the disk
int readSector(char *buf, int absSector) {
    int relSector = mod(absSector, 18) + 1;
    int head = mod((absSector / 18), 2);
    int track = absSector / 36;
    int ax = 0x02 * 256 + 0x01;
    int cx = track * 256 + relSector;
    int dx = head * 256 + 0x00;

    interrupt(0x13, ax, buf, cx, dx);

    return 1;
}

// system calls for user programs
int handleInterrupt21(int ax, int bx, int cx, int dx) {
    char *str;
    int j;
    int i = 0;
    if(ax == 0x00) {
        str = bx;
        return printString(str);
    } else if(ax == 0x11) {
        str = bx;
        str[0] = readChar();
        return 1;
    } else if(ax == 0x01) {
        str = bx;
        readString(str);
        while(str[i] != '\0') {
            i += 1;
        }
        return i;
    } else if(ax == 0x03) {
        char *buf = cx;
        str = bx;        
        return readfile(str, buf);
    } else if(ax == 0x04) {
        str = bx;
        return executeProgram(str);
    } else if(ax == 0x05) {
        terminate();
    } else if(ax == 0x07) {
        str = bx;
        return deleteFile(bx);
    } else if(ax == 0x08) {
        char *buffer = cx;
        str = bx;
        return writeFile(str, buffer, dx);        
    } else if(ax == 0x09) {
        yield();
    } else if(ax == 0x0A) {
        showProcesses();
    } else if(ax == 0x0B) {
        return kill(bx);
    } else if(ax == 0x0C) {
        printHello(); // shell will try to run this and print hello
    } else {
        return -1;
    }
}

void printInt(int i) {
    int *numb[2];
    numb[0] = i;
    numb[1] = '\0';
    printString(numb);
}

// find if a file exists in the disk directory
// if the file exists, then return the sector
// in which the contents of the file are stored
int readfile(char *filename, char *buf) {
    int dirEntryNum, count, sectorNum;
    
    struct directory diskDir;

    count = 0;
    sectorNum = 0;

    readSector(&diskDir, 2);

    dirEntryNum = findFile(&diskDir, filename, buf);

    if(dirEntryNum == -1) {
        printString("rF error: File does not exist\0");
        //interrupt(0x21, 0x00, "rf file not found\0", 0, 0);
        return -1;
    }

    while(diskDir.entries[dirEntryNum].sectors[sectorNum] != 0x00 && sectorNum <= 25) {
        readSector(buf + count, diskDir.entries[dirEntryNum].sectors[sectorNum]);
        count = count + 512;
        sectorNum++;
    }

    return sectorNum;
}

// helper method to readfile
// returns the directory number
int findFile(struct directory *dir, char *filename, char *buf) {
    int i, j;

    for(i = 0; i < 16; i++) {
        for(j = 0; j < 6; j++) {
            if(dir -> entries[i].name[j] != filename[j]) {
                break;
            }
        }

        if(j == 6) {
            return i;
        }  
    }
    return -1;
}

// helper method to findFile
int fileNameLen(char *filename) {
    int i = 0;

    while(filename[i] != '\0') {
        i+=1;
    }

    return i;
}

// given a program name, and segment
// check if the program file exists
// check if the segment is valid
// and if both are true, execute the program
int executeProgram(char* filename) {
    int file, i, j, index, segment, offset;
    struct PCB *program;
    char buf[13312];

    offset = 0x000;
    file = readfile(filename, buf);

    if(file == -1) {
        printString("eP error: File does not exist\0");
        return -1;
    }
    
    setKernelDataSegment();
    index = getFreeMemorySegment(); // store index of free memory from memoryMap
    restoreDataSegment();

    if(index == -1) {
        printString("eP error: no free memory segments\0");
        return -2;
    }

    segment = 0x1000 * (index + 2); // calculate the segment number

    for(i = 0; i < 13312; i++) {
        putInMemory(segment, offset, buf[i]);
        offset++;
    }

    setKernelDataSegment();
    program = getFreePCB(); // find free PCB from pcbPool
    program->state = STARTING;
    program->segment = segment;
    program->stackPointer = 0xFF00;
    addToReady(program);
    restoreDataSegment();

    if(program == NULL) {
        printString("eP error: no free pcbs\0");
        return -2;
    }

    kStrCopy(filename, program->name, 7);

    // launchProgram(segment);
    initializeProgram(segment);
    return 1;
}

// terminate a running program by resetting the segments
void terminate() {
    int segment;
    setKernelDataSegment();
    segment = running->segment;
    releaseMemorySegment((segment - 0x2000) / 0x1000);
    releasePCB(running);
    restoreDataSegment();

    while(1);
}

// write the contents of buffer into a sector
int writeSector(char *buffer, int sector) {
    int relSector = mod(sector, 18) + 1;
    int head = mod((sector / 18), 2);
    int track = sector / 36;
    int ax = 0x03 * 256 + 0x01;
    int cx = track * 256 + relSector;
    int dx = head * 256 + 0x00;

    interrupt(0x13, ax, buffer, cx, dx);

    return 1;
}

// search for a file and delete it by replacing the first
// character of the file name with 0x00
int deleteFile(char *fname) {
    int dirEntryNum, i;
    char *buf;

    struct directory diskDir;
    readSector(&diskDir, 2);
    
    dirEntryNum = findFile(&diskDir, fname, buf);

    if(dirEntryNum != -1) {
        for(i = 0; i < 26; i++) {
            diskDir.entries[dirEntryNum].sectors[i] = 0x00;
        }
        diskDir.entries[dirEntryNum].name[0] = 0x00;
        writeSector(&diskDir, 2);
        return 1;
    } else {
        printString("dF error: File does not exist\0");
        return -1;
    }
}

// given a filename and the file contents stored in a buffer
// find an empty directory entry, and a number of empty sectors
// to store the file in
int writeFile(char *filename, char *buffer, int sectors) {
    int dirEntryNum, i, j, k, l, m, sectorsWritten;
    char *buf;
    struct directory diskDir;
    char map[512];
    char emptySectors[26];

    j = 0;
    m = 0; // map indicies from 0 to 512
    sectorsWritten = 0; // keep track of how many sectors we've written into

    readSector(&diskDir, 2);
    readSector(map, 1);

    dirEntryNum = findFile(&diskDir, filename, buf);

    if(sectors > 26) {
        sectors = 26;
    }

    if(dirEntryNum == -1) {
        // search all the directory entries for an empty one
        for(i = 0; i < 16; i++) {
            if(diskDir.entries[i].name[0] == 0x00) {
                break;
            }
        }

        // update dirEntryNum to a free entry
        dirEntryNum = i;

        // return -1 if no empty dirEntry
        if(i == 16) {
            return -1;
        }
    }

    // looking for free sectors
    while(j < sectors) {
        while(m < 512) {
            if(map[m] == 0x00) {
                emptySectors[j] = m;
                //m = 1000;
                m++;
                break;
            } else {
                m++;
            }
        }
        j++;
    }
    

    // if there are insufficient free sectors, return -2
    // save as much of a file as possible????
    if(m == 512) {
        // writing file name to diskdir
        for(k = 0; k < 6; k++) {
            if(filename[k] != 0x00 || filename[k] != '\0') {
                diskDir.entries[dirEntryNum].name[k] = filename[k];
            } else {
                diskDir.entries[dirEntryNum].name[k] = 0x00;
            }
        }

        // fill out which sectors to use
        for(l = 0; l < sectors; l++) {
            diskDir.entries[dirEntryNum].sectors[l] = emptySectors[l];

            // write contents of buffer into the sectors
            writeSector(&buffer[l * 512], emptySectors[l]);
            map[emptySectors[l]] = 0xFF;
            sectorsWritten++;
        }
        writeSector(&diskDir, 2);
        writeSector(map, 1);
        return -2;
    } else {
        // writing file name to diskdir
        for(k = 0; k < 6; k++) {
            if(filename[k] != 0x00 || filename[k] != '\0') {
                diskDir.entries[dirEntryNum].name[k] = filename[k];
            } else {
                diskDir.entries[dirEntryNum].name[k] = 0x00;
            }
        }

        // fill out which sectors to use
        for(l = 0; l < sectors; l++) {
            diskDir.entries[dirEntryNum].sectors[l] = emptySectors[l];

            // write contents of buffer into the sectors
            writeSector(&buffer[l * 512], emptySectors[l]);
            map[emptySectors[l]] = 0xFF;
            sectorsWritten++;
        }
        writeSector(&diskDir, 2);
        writeSector(map, 1);
        return sectorsWritten;
    }
}

// saves running process and starts a new one
void handleTimerInterrupt(int segment, int sp) {
	struct PCB *pcbToRun; // this is the new process that will be run
    int newSeg, newSP;

    setKernelDataSegment();
    running->segment = segment;
    running->stackPointer = sp;
    running->state = READY;

    if(running != &idleProc) {
        addToReady(running);
    }

    running = NULL;

    pcbToRun = removeFromReady();

    if(pcbToRun != NULL) {
        running = pcbToRun;
        newSeg = pcbToRun->segment;
        newSP = pcbToRun->stackPointer;
    } else {
        running = &idleProc;
    }
    restoreDataSegment();
    returnFromTimer(newSeg, newSP);
}

/* kStrCopy(char *src, char *dest, int len) copy at most len
 * characters from the src which is addressed relative to the current 
 * data segment into dest which is addressed relative to the 
 * kernel's data segment (0x1000).
 */
void kStrCopy (char *src, char *dest, int len) {
    int i = 0;
    for(i = 0; i < len; i++) {
        putInMemory(0x1000, dest + i, src[i]);
        if(src[i] == 0x00) {
            return;
        }
    }
}

// causes executing process to give up remaining time and go back to
// the ready queue
void yield() {
    interrupt(0x08, 0, 0, 0, 0);
    // running = running->segment;
    // running = running->stackPointer;
    // running->state = READY;

    // addToReady(running);
}

// lists all processes
void showProcesses() {
    // int i, segment;
    // struct PCB *pcb;

    // char *one = "One\n";
    // char *two = "Two\n";
    // char *three = "Three\n";
    // char *four = "Four\n";
    // char *five = "Five\n";
    // char *six = "Six\n";
    // char *seven = "Seven\n";
    // char *eight = "Eight\n";

    // char numbers[8];

    // numbers[0] = one;
    // numbers[1] = two;
    // numbers[2] = three;
    // numbers[3] = four;
    // numbers[4] = five;
    // numbers[5] = six;
    // numbers[6] = seven;
    // numbers[7] = eight;

    // setKernelDataSegment();
    // for(i = 0; i < 8; i++) {
    //     if(memoryMap[i] == USED) {
    //         segment = 0x1000 * (i + 2);
    //         pcb = readyHead;
    //         while(pcb != NULL) {
    //             if(pcb->segment == segment) {
    //                 printString(numbers[i]);
    //                 printString(pcb->name);
    //                 printString("\r\n");
    //             } else {
    //                 pcb = pcb->next;
    //             }
    //         }
    //     }
    // }
    // restoreDataSegment();
    printString("test\0");
}

// kill the process running on the given segment
int kill(int segment) {
    int index, i;

    index = (segment / 0x1000) - 2;
    setKernelDataSegment();
    if(memoryMap[index] == USED) {
        for(i = 0; i < 8; i++) {
            if(pcbPool[i].segment == segment) {
                releaseMemorySegment(index);
                releasePCB(&pcbPool[i]);
                return 1;
            }
        }
    } else {
        return -1;
    }
    restoreDataSegment();
}

//added to system calls
void printHello() {
    printString("Hello\0");
}