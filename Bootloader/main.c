#include <uefi.h>

int main(int argc, char **argv) {
    // Standard printf works for UEFI console output
    printf("Hello, World!\n");

    // Wait for a single keystroke
    getchar();

    return 0;
}
