#include "api/screen.c"

int main(int count, char** argv) {
    print_string("&aAHello from the shell app!&0F\n");
    print_string(argv[0]);
    while (1) {}
    return 0;
}