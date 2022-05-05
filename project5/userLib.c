#include "userLib.h"

void printString(char buf[]) {
    interrupt(0x21, 0x00, buf, 0, 0);
}

void readChar() {
    interrupt(0x21, 0x11, 0, 0, 0);
}

void readString(char *buf) {
    interrupt(0x21, 0x01, buf, 0, 0);
}

int readFile(char filename[], char buffer[]) {
    return interrupt(0x21, 0x03, filename, buffer, 0);
}

void executeProgram(char filename[]) {
    interrupt(0x21, 0x04, filename, 0, 0);
}

void terminate() {
    interrupt(0x21, 0x05, 0, 0, 0);
}

int deleteFile(char filename[]) {
    return interrupt(0x21, 0x07, filename, 0, 0);
}

int writeFile(char filename[], char buffer[], int sectors) {
    return interrupt(0x21, 0x08, filename, buffer, sectors);
}

int readCharShell(char *buf) {
    return interrupt(0x21, 0x11, buf, 0, 0);
}

void showProcesses() {
    interrupt(0x21, 0x0A, 0, 0, 0);
}

int kill(int segment) {
    return interrupt(0x21, 0x0B, 0, 0, 0);
}