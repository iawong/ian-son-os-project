#include "userLib.h"

int printString(char *str) {
    interrupt(0x21, 0x00, str, 0, 0);
}

void readChar() {
    interrupt(0x21, 0x11, 0, 0, 0);
}

void readString(char *buf) {
    interrupt(0x21, 0x01, buf, 0, 0);
}

int readFile(char *filename, char *buf) {
    interrupt(0x21, 0x03, filename, buf, 0);
}

int executeProgram(char* name, int segment) {
    interrupt(0x21, 0x04, name, segment, 0);
}

void terminate() {
    interrupt(0x21, 0x05, 0, 0, 0);
}