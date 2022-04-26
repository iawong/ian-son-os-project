void putChar(int row, int column, char c, char color);
void putStr(int row, int column, char string[], char color);
void printString(char *str);
int readChar();
int readString(char *buf);
int readSector(char *buf, int absSector);
int mod(int a, int b);
int handleInterrupt21(int ax, int bx, int cx, int dx);


int main() {
    // char *buf = "\0";
    // char ch;

    // printString("Hello World\0");
    
    // printString("Type a char: \0");
    // ch = readChar();
    // buf[0] = ch;
    // printString("Read: \0");
    // printString(buf);
    // printString("\n\r\0");

    char line[20];
    printString("Enter a line: \0");
    readString(line);
    printString("\n\0");
    printString(line);

    // char buffer[512];
    // readSector(buffer, 30);
    // printString(buffer);

    // char line[80];
    // char ch[1];
    // makeInterrupt21();
    // interrupt(0x21, 0x00, 0, 0, 0);
    // interrupt(0x21, 0x00, "Type>\0", 0, 0);
    // interrupt(0x21, 0x11, ch, 0, 0);
    // line[0] = ch[0];
    // line[1] = 0x00;
    // interrupt(0x21, 0x00, line, 0, 0);


    while(1) {}

    return 0;
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
void printString(char *str) {
    int i = 0;
    while(str[i] != '\0') {
        char al = str[i];
        char ah = 0x0E;
        int ax = ah * 256 + al;
        interrupt(0x10, ax, 0, 0, 0);
        i += 1;
    }    
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

int handleInterrupt21(int ax, int bx, int cx, int dx) {
    // printString("Quit Interrupting!");
    char *str;
    int i = 0;
    if(ax == 0x00) {
        str = bx;
        printString(str);
        while(str[i] != '\0') {
            i += 1;
        }
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
    } else {
        return -1;
    }
}