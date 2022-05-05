int printString(char *str);

int readChar();

int readString(char *buf);

int readFile(char *filename, char *buf);

int executeProgram(char* filename);

void terminate();

int deleteFile(char filename[]);

int writeFile(char *filename, char *buffer, int sectors);

int readCharShell(char *buf);

void showProcesses();

int kill(int segment);

void yield();

void printHello();