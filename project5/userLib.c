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

void executeProgram(char filename[], int sector) {
    interrupt(0x21, 0x04, filename, sector, 0);
}

void terminate() {
    interrupt(0x21, 0x05, 0, 0, 0);
}

void mod(int a, int b) {
    interrupt(0x21, 0x06, a, b, 0);
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