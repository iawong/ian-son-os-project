main() {
    int k, sectors;
    char filename[7];
    char buffer[13312];
    char ch;

    enableInterrupts(); // enable concurrency

    printString("Enter File Name: \r\n\0");
    readString(filename);
    printString("\r\n");
    printString("Enter File Content: \r\n\0");
    readText(buffer);
    printString("\r\n");

    // loop through buffer and each 512 bytes that exist will add 1 sector
    while(buffer[k] != '\0') {
        if(mod(k, 512) == 0) {
            sectors++;
        }
        k++;
    }
    // add an extra sector for any extra bytes that don't add up to 512
    sectors++;

    writeFile(filename, buffer, sectors);

    terminate();
    while(1);
}

// read the text for text editor program
// quit if CTRL+D is followed up by ENTER
int readText(char *buf) {
    int i = 0;
    char *ch;
    char ah = 0x0E;
    int ax;
    int exit = 0;

    while(exit != 2) {
        readCharShell(ch);
        if (ch[0] == 0x08) {
            if(i != 0) {
                buf[i - 1] = 0x20;
            }
            ax = ah * 256 + ch[0];
            interrupt(0x10, ax, 0, 0, 0);

            ax = ah * 256 + 0x20;
            interrupt(0x10, ax, 0, 0, 0);

            ax = ah * 256 + ch[0];
            interrupt(0x10, ax, 0, 0, 0);
            i -= 1;
            exit = 0;
        } else if(ch[0] == 0x04) {
            exit++;
        } else if(ch[0] == 0x0D) {
            if(exit == 1) {
                exit++;
            } else {
            printString("\r\n");
            }
        } else {
            buf[i] = ch[0];
            ax = ah * 256 + ch[0];
            interrupt(0x10, ax, 0, 0, 0);
            i += 1;
            exit = 0;
        }
    }

    buf[i] = 0x00;

    return buf;
}