#include "userLib.h"

int mod(int a, int b);
int readSector(char *buf, int absSector);

int main() {
    while(1) {
        int i, j, k, l, sectors, segment, index;

        typedef char byte;

        struct dirEntry {
            char name[6];
            byte sectors[26];
        };

        struct directory {
            struct dirEntry entries[16];
        };

        char buffer[13312];
        char line[100];
        char filename[7];
        char filename2[7];

        enableInterrupts();

        printString("Shell> \0");
        readString(line);
        printString("\r\n");

        if(line[0] == 't' && line[1] == 'y' && line[2] == 'p' && line[3] == 'e') {
            for(i = 0; i < 6; i++) {
                filename[i] = line[5 + i];
            }
            filename[6] = '\0';
            readFile(filename, buffer);
            
            printString(buffer);
        } else if(line[0] == 'e' && line[1] == 'x' && line[2] == 'e' && line[3] == 'c' && 
        line[4] == 'u' && line[5] == 't' && line[6] == 'e') {
            for(i = 0; i < 6; i++) {
                filename[i] = line[8 + i];
            }
            filename[6] = '\0';

            executeProgram(filename);
        } else if(line[0] == 'd' && line[1] == 'e' && line[2] == 'l' && line[3] == 'e' &&
        line[4] == 't' && line[5] == 'e') {
            for(i = 0; i < 6; i++) {
                filename[i] = line[7 + i];
            }
            filename[6] = '\0';
            deleteFile(filename);
        } else if(line[0] == 'c' && line[1] == 'o' && line[2] == 'p' && line[3] == 'y') {
            for(i = 0; i < 6; i++) {
                filename[i] = line[5 + i];
            }
            filename[6] = '\0';

            // get second filename
            for(j = 0; j < 6; j++) {
                filename2[j] = line[6 + i + j];
            }

            filename2[6] = '\0';
            
            readFile(filename, buffer); // read file contents into buffer

            sectors = 0;
            k = 0;

            // loop through buffer and each 512 bytes that exist will add 1 sector
            while(buffer[k] != '\0') {
                if(mod(k, 512) == 0) {
                    sectors++;
                }
                k++;
            }
            // add an extra sector for any extra bytes that don't add up to 512
            sectors++;

            l = writeFile(filename2, buffer, sectors);
            if(l == -1) {
                printString("Disk directory is full\r\n\0");
            } else if(l == -2) {
                printString("Disk directory now full\r\n\0");
            }
        } else if(line[0] == 'd' && line[1] == 'i' && line[2] == 'r') {
            // print all files on disk
            struct directory diskDir;

            readSector(&diskDir, 2);

            for(i = 0; i < 16; i++) {
                if(diskDir.entries[i].name[0] != 0x00) {
                    printString(diskDir.entries[i].name);
                    printString("\r\n");
                }
            }
        } else if(line[0] == 'p' && line[1] == 's') {
            showProcesses();
        } else if(line[0] == 'k' && line[1] == 'i', line[2] == 'l', line[3] == 'l') {
            i = kill(line[5] - '0');
            if(i == 1) {
                printString("Successful\r\n\0");
            } else {
                printString("Unsuccessful\r\n\0");
            }
        } else if(line[0] == 's' && line[1] == 'l' && line[2] == 'e' && line[3] == 'e' &&
        line[4] == 'p') {
            sleep(line[6] - '0');
        } else {
            printString("Unrecognized command\0\r\n");
        }
    }
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