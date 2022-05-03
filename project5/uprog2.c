main() {
    enableInterrupts();
    interrupt(0x21, 0x00, "UPROG2\r\n\0", 0, 0);
    interrupt(0x21, 0x05, 0, 0, 0);
    while(1);
}