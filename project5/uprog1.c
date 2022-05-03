main() {
    enableInterrupts();
    interrupt(0x21, 0x00, "WooHoo! I'm a user program!\r\n\0", 0, 0);
    while(1);
}