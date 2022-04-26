main() {
    while(1) {
        int i;

        char buffer[13312];
        char line[100];
        char filename[7];

        printString("Shell> \0");
        readString(line);
        // interrupt(0x21, 0x01, line, 0, 0); // type filename
        printString("\n");
        // interrupt(0x21, 0x00, "\n", 0, 0);

        if(line[0] == 't' && line[1] == 'y' && line[2] == 'p' && line[3] == 'e') {
            for(i = 0; i < 6; i++) {
                if(line[5 + i] == 0x00) { // if the current character is empty, add \0 and break
                    filename[i] = '\0';
                    break;
                } else {
                    filename[i] = line[5 + i];
                }
            }
            if(i == 6) { // if we've reached the end of for loop, add \0 to the end
                filename[6] = '\0';
            }
            // interrupt(0x21, 0x03, filename, buffer, 0);
            readFile(filename, buffer);
            // interrupt(0x21, 0x00, buffer, 0, 0);
            printString(buffer);
        } else if(line[0] == 'e' && line[1] == 'x' && line[2] == 'e' && line[3] == 'c' && 
        line[4] == 'u' && line[5] == 't' && line[6] == 'e') {
            for(i = 0; i < 6; i++) {
                if(line[8 + i] == 0x00) {
                    filename[i] = '\0';
                    break;
                } else {
                    filename[i] = line[8 + i];
                }
            }
            if(i == 6) {
                filename[6] = '\0';
            }
            // interrupt(0x21, 0x04, filename, 0x2000, 0);
            executeProgram(filename, 0x2000);
            // interrupt(0x21, 0, "\n", 0, 0);
            printString("\n");
        } else {
            // interrupt(0x21, 0, "Unrecognized command\0", 0, 0);
            printString("Unrecognized command\0");
            // interrupt(0x21, 0, "\n", 0, 0);
            printString("\n");
        }
    }
}