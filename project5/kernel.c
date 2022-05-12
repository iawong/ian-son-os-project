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
void sleep(int seconds);

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
    while(*str != '\0') {
        interrupt(0x10, 0x0E * 256 + *str, 0, 0, 0);
        ++str;
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
    } else if(ax == 0xA1) {
        sleep(bx);
    } else {
        return -1;
    }
}

void printInt(int i) {
    int *num[2];
    num[0] = i;
    num[1] = '\0';
    printString(num);
}

// find if a file exists in the disk directory
// if the file exists, then return the sector
// in which the contents of the file are stored
int readfile(char *filename, char *buf) {
    int dirEntryNum, count, sectorNum, i;
    char error[27];
    char *str;

    struct directory diskDir;

    count = 0;
    sectorNum = 0;

    readSector(&diskDir, 2);

    dirEntryNum = findFile(&diskDir, filename, buf);

    if(dirEntryNum == -1) {
        error[0] = 'r';
        error[1] = 'F';
        error[2] = ' ';
        error[3] = 'e';
        error[4] = 'r';
        error[5] = 'r';
        error[6] = 'o';
        error[7] = 'r';
        error[8] = ':';
        error[9] = ' ';
        error[10] = 'F';
        error[11] = 'i';
        error[12] = 'l';
        error[13] = 'e';
        error[14] = ' ';
        error[15] = 'n';
        error[16] = 'o';
        error[17] = 't';
        error[18] = ' ';
        error[19] = 'f';
        error[20] = 'o';
        error[21] = 'u';
        error[22] = 'n';
        error[23] = 'd';
        error[24] = '\r';
        error[25] = '\n';
        error[26] = '\0';
        printString(error);
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
    char error1[27];
    char error2[35];
    char error3[24];

    offset = 0x000;
    file = readfile(filename, buf);

    if(file == -1) {
        error1[0] = 'e';
        error1[1] = 'P';
        error1[2] = ' ';
        error1[3] = 'e';
        error1[4] = 'r';
        error1[5] = 'r';
        error1[6] = 'o';
        error1[7] = 'r';
        error1[8] = ':';
        error1[9] = ' ';
        error1[10] = 'F';
        error1[11] = 'i';
        error1[12] = 'l';
        error1[13] = 'e';
        error1[14] = ' ';
        error1[15] = 'n';
        error1[16] = 'o';
        error1[17] = 't';
        error1[18] = ' ';
        error1[19] = 'f';
        error1[20] = 'o';
        error1[21] = 'u';
        error1[22] = 'n';
        error1[23] = 'd';
        error1[24] = '\r';
        error1[25] = '\n';
        error1[26] = '\0';
        printString(error1);
        return -1;
    }
    
    setKernelDataSegment();
    index = getFreeMemorySegment(); // store index of free memory from memoryMap
    restoreDataSegment();

    if(index == -1) {
        error2[0] = 'e';
        error2[1] = 'P';
        error2[2] = ' ';
        error2[3] = 'e';
        error2[4] = 'r';
        error2[5] = 'r';
        error2[6] = 'o';
        error2[7] = 'r';
        error2[8] = ':';
        error2[9] = ' ';
        error2[10] = 'n';
        error2[11] = 'o';
        error2[12] = ' ';
        error2[13] = 'f';
        error2[14] = 'r';
        error2[15] = 'e';
        error2[16] = 'e';
        error2[17] = ' ';
        error2[18] = 'm';
        error2[19] = 'e';
        error2[20] = 'm';
        error2[21] = 'o';
        error2[22] = 'r';
        error2[23] = 'y';
        error2[24] = ' ';
        error2[25] = 's';
        error2[26] = 'e';
        error2[27] = 'g';
        error2[28] = 'm';
        error2[29] = 'e';
        error2[30] = 'n';
        error2[31] = 't';
        error2[32] = '\r';
        error2[33] = '\n';
        error2[34] = '\0';
        printString(error2);
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
        error3[0] = 'e';
        error3[1] = 'P';
        error3[2] = ' ';
        error3[3] = 'e';
        error3[4] = 'r';
        error3[5] = 'r';
        error3[6] = 'o';
        error3[7] = 'r';
        error3[8] = ':';
        error3[9] = ' ';
        error3[10] = 'n';
        error3[11] = 'o';
        error3[12] = ' ';
        error3[13] = 'f';
        error3[14] = 'r';
        error3[15] = 'e';
        error3[16] = 'e';
        error3[17] = ' ';
        error3[18] = 'p';
        error3[19] = 'c';
        error3[20] = 'b';
        error3[21] = '\r';
        error3[22] = '\n';
        error3[23] = '\0';
        printString(error3);
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
    char error[27];

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
        error[0] = 'd';
        error[1] = 'F';
        error[2] = ' ';
        error[3] = 'e';
        error[4] = 'r';
        error[5] = 'r';
        error[6] = 'o';
        error[7] = 'r';
        error[8] = ':';
        error[9] = ' ';
        error[10] = 'F';
        error[11] = 'i';
        error[12] = 'l';
        error[13] = 'e';
        error[14] = ' ';
        error[15] = 'n';
        error[16] = 'o';
        error[17] = 't';
        error[18] = ' ';
        error[19] = 'f';
        error[20] = 'o';
        error[21] = 'u';
        error[22] = 'n';
        error[23] = 'd';
        error[24] = '\r';
        error[25] = '\n';
        error[26] = '\0';
        printString(error);
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
}

// lists all processes
void showProcesses() {
    int i, j, segment;
    struct PCB *pcb;
    char *str;
    char numbers[8];
    char zero[5];
    char one[4];
    char two[4];
    char three[6];
    char four[6];
    char five[6];
    char six[6];
    char seven[6];

    zero[0] = 'Z';
    zero[1] = 'e';
    zero[2] = 'r';
    zero[3] = 'o';
    zero[4] = '\0';

    one[0] = 'O';
    one[1] = 'n';
    one[2] = 'e';
    one[3] = '\0';

    two[0] = 'T';
    two[1] = 'w';
    two[2] = 'o';
    two[3] = '\0';

    three[0] = 'T';
    three[1] = 'h';
    three[2] = 'r';
    three[3] = 'e';
    three[4] = 'e';
    three[5] = '\0';

    four[0] = 'F';
    four[1] = 'o';
    four[2] = 'u';
    four[3] = 'r';
    four[4] = '\0';

    five[0] = 'F';
    five[1] = 'i';
    five[2] = 'v';
    five[3] = 'e';
    five[4] = '\0';

    six[0] = 'S';
    six[1] = 'i';
    six[2] = 'x';
    six[3] = '\0';

    seven[0] = 'S';
    seven[1] = 'e';
    seven[2] = 'v';
    seven[3] = 'e';
    seven[4] = 'n';
    seven[5] = '\0';

    numbers[0] = zero;
    numbers[1] = one;
    numbers[2] = two;
    numbers[3] = three;
    numbers[4] = four;
    numbers[5] = five;
    numbers[6] = six;
    numbers[7] = seven;

    setKernelDataSegment();
    for(i = 0; i < 8; i++) {
        if(memoryMap[i] == USED) {
            interrupt(0x10, 0x0E * 256 + ' ', 0, 0, 0);

            pcb = &pcbPool[i];
            str = pcb->name;
            printString(str);
            interrupt(0x10, 0x0E * 256 + '\n', 0, 0, 0);
            interrupt(0x10, 0x0E * 256 + '\r', 0, 0, 0);
        }
    }
    restoreDataSegment();
}

// kill the process running on the given segment
int kill(int index) {
    if(memoryMap[index] == USED) {
        setKernelDataSegment();
        releaseMemorySegment(index);
        releasePCB(index);
        running = NULL;
        restoreDataSegment();
        return 1;
    } else {
        return -1;
    }
}

// pauses the running processes for x amount of seconds
void sleep(int seconds) { // how to implement seconds?
    int i;
    setKernelDataSegment();
    running->state = BLOCKED;
    for(i = 0; i < seconds * 1000; i++) {}
    restoreDataSegment();
    yield();
}