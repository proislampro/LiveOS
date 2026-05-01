#include <uefi.h>

int main(int argc, char **argv)
{
    (void)argc; (void)argv;
    FILE *f;
    char *buff;
    long int size;

    // Open in binary mode to ensure accurate byte count
    if((f = fopen("/hello.txt", "rb"))) {
        fseek(f, 0, SEEK_END);
        size = ftell(f);
        fseek(f, 0, SEEK_SET);

        printf("File size: %ld bytes\n", size);

        buff = malloc(size + 1);
        if(!buff) {
            fprintf(stderr, "Error: Memory allocation failed\n");
            fclose(f); // Always close before returning
            return 1;
        }

        // fread returns the number of items read; good for error checking
        if(fread(buff, size, 1, f) == 1) {
            buff[size] = '\0';
            printf("File contents:\n%s\n", buff);
        } else {
            fprintf(stderr, "Error: Could not read file data\n");
        }

        free(buff);
        fclose(f);
    } else {
        fprintf(stderr, "Error: Unable to open /hello.txt\n");
    }

    return 0;
}