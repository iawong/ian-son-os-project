void putChar(int row, int column, char c, char color);
void putStr(int row, int column, char string[], char color);

int main() {
    putInMemory(0xB000, 0x8140, 'H');
    putInMemory(0xB000, 0x8141, 0x0F);
    putInMemory(0xB000, 0x8142, 'E');
    putInMemory(0xB000, 0x8143, 0x0F);
    putInMemory(0xB000, 0x8144, 'L');
    putInMemory(0xB000, 0x8145, 0x0F);
    putInMemory(0xB000, 0x8146, 'L');
    putInMemory(0xB000, 0x8147, 0x0F);
    putInMemory(0xB000, 0x8148, 'O');
    putInMemory(0xB000, 0x8149, 0x0F);
    putInMemory(0xB000, 0x814C, 'W');
    putInMemory(0xB000, 0x814D, 0x0F);
    putInMemory(0xB000, 0x814E, 'O');
    putInMemory(0xB000, 0x814F, 0x0F);
    putInMemory(0xB000, 0x8150, 'R');
    putInMemory(0xB000, 0x8151, 0x0F);
    putInMemory(0xB000, 0x8152, 'L');
    putInMemory(0xB000, 0x8153, 0x0F);
    putInMemory(0xB000, 0x8154, 'D');
    putInMemory(0xB000, 0x8155, 0x0F);

    putChar(13, 34, 0x48, 0x4F);
    putChar(13, 35, 0x45, 0x4F);
    putChar(13, 36, 0x4C, 0x4F);
    putChar(13, 37, 0x4C, 0x4F);
    putChar(13, 38, 0x4F, 0x4F);
    putChar(13, 40, 0x57, 0x4F);
    putChar(13, 41, 0x4F, 0x4F);
    putChar(13, 42, 0x52, 0x4F);
    putChar(13, 43, 0x4C, 0x4F);
    putChar(13, 44, 0x44, 0x4F);

    putStr(25, 70, "Hello World", 0x4F);    

    return 0;
}

void putChar(int row, int column, char c, char color) {
    int segment = 0xB000;
    int offset = 0x8000 + (80 * (row - 1) * 2) + ((column - 1) * 2);
    putInMemory(segment, offset, c);
    putInMemory(segment, offset + 1, color);
}

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