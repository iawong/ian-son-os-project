void putChar(int row, int column, char c, char color);
void putStr(int row, int column, char string[], char color);
int printString(char *str);
int readChar();
int readString(char *buf);
int readSector(char *buf, int absSector);
int mod(int a, int b);
int handleInterrupt21(int ax, int bx, int cx, int dx);
int readfile(char *filename, char *buf);
int findFile(struct directory *dir, char *filename, char *buf);
int fileNameLen(char *filename);
int executeProgram(char* name, int segment);
void terminate();
void print();

typedef char byte;

struct dirEntry {
    char name[6];
    byte sectors[26];
};

struct directory {
    struct dirEntry entries[16];
};


int main() {
    makeInterrupt21();
    interrupt(0x21, 0x04, "shell\0", 0x2000, 0);

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
        i++;
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

// system calls
int handleInterrupt21(int ax, int bx, int cx, int dx) {
    char *str;
    int i = 0;
    if(ax == 0x00) {
        str = bx;
        printString((char*) bx);
        return i;
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
        return executeProgram(str, cx);
    } else if(ax == 0x05) {
        terminate();
    } else {
        return -1;
    }
}

// find if a file exists in the disk directory
// if the file exists, then return the sector
// in which the contents of the file are stored
int readfile(char *filename, char *buf) {
    int dirEntryNum, count, sectorNum, i;
    char *error;
    
    struct directory diskDir;

    count = 0;
    sectorNum = 0;

    readSector(&diskDir, 2);

    dirEntryNum = findFile(&diskDir, filename, buf);

    if(dirEntryNum == -1) {
        error = ("rf error: File not found\0");
        // while(error[i] != '\0') {
        //     interrupt(0x10, 0x0E * 256 + error[i], 0, 0, 0);
        //     i++;
        // }
        print();
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
int findFile(struct directory *dir, char *filename, char *buf) {
    int i, j, bool, len;
    bool = 0;

    len = fileNameLen(filename);

    for(i = 0; i < 16; i++) {
        for(j = 0; j < 6; j++) {
            if(dir -> entries[i].name[j] != filename[j]) {
                bool = 0;
                break;
            }
            if(dir -> entries[i].name[j] == 0x00 || dir -> entries[i].name[j] == '\0') {
                bool = 0;
                break;
            }
        }

        if(len == j) {
            bool = 1;
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
int executeProgram(char* name, int segment) {
    int file, i;
    char buf[13312];

    file = readfile(name, buf);

    if(file == -1) {
        printString("File does not exist\0");
        return -1;
    }

    if(segment == 0x0000 || segment >= 0xA000 || segment == 0x1000) {
        printString("Invalid segment\0");
        return -2;
    }

    while(i < 13312) {
        putInMemory(segment, i, buf[i]);
        i += 1;
    }

    launchProgram(segment);
}

// terminate a running program by resetting the segments
void terminate() {
    resetSegments();
    interrupt(0x21, 0x04, "shell\0", 0x2000, 0);
}

void print() {
    int i = 0;
    char *str = "rf\0";
    char r = 'r';
    char f = 'f';
    // while loop doesn't work for some reason
    // could be the str[i]
    // while(*str != '\0') {
    //     interrupt(0x10, 0x0E * 256 + *str, 0, 0, 0);
    //     ++str;
    // }

    interrupt(0x10, 0x0E * 256 + str[0], 0, 0, 0);
    interrupt(0x10, 0x0E * 256 + str[1], 0, 0, 0);
}