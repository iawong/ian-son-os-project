#include "userLib.h"

int main() {
    int i;

    char buffer[13312];
    char line[100];
    char filename[7];

    while(1) {
        printString("Shell> \0");
        readString(line);
        printString("\n");

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

            executeProgram(filename, 0x2000);
            printString("\n");
        } else {
            printString("Unrecognized command\0");
        }
    }
}