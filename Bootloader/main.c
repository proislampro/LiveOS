#include <efi.h>
#include <efilib.h>

// Standard UEFI entry point
EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    // Initialize the UEFI library
    InitializeLib(ImageHandle, SystemTable);

    // Print a simple message to the screen
    Print(L"Hello, UEFI Bootloader!\n");

    // Keep the message on screen (waits for a keystroke)
    SystemTable->ConIn->Reset(SystemTable->ConIn, FALSE);
    EFI_INPUT_KEY Key;
    while (SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &Key) != EFI_SUCCESS);

    return EFI_SUCCESS;
}
