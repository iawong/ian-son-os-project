void printString(char buf[]) {
    interrupt(0x21, 0x00, buf, 0, 0);
}

void readChar() {
    interrupt(0x21, 0x11, 0, 0, 0);
}

void readString(char *buf) {
    interrupt(0x21, 0x01, buf, 0, 0);
}

void readFile(char filename[], char buffer[]) {
    interrupt(0x21, 0x03, filename, buffer, 0);
}

void executeProgram(char filename[], int sector) {
    interrupt(0x21, 0x04, filename, sector, 0);
}

void terminate() {
    interrupt(0x21, 0x05, 0, 0, 0);
}