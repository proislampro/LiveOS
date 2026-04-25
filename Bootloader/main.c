#include <efi.h>
#include <efilib.h>

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    InitializeLib(ImageHandle, SystemTable);

    // Mode 0 is guaranteed to be 80x25 by the UEFI spec
    EFI_STATUS Status = uefi_call_wrapper(ST->ConOut->SetMode, 2, ST->ConOut, 0);

    if (EFI_ERROR(Status)) {
        Print(L"Error: Could not set 80x25 mode.\n");
    } else {
        Print(L"80x25 VGA-style text mode set successfully.\n");
    }

    // Clear screen to initialize the buffer
    uefi_call_wrapper(ST->ConOut->ClearScreen, 1, ST->ConOut);

    for (;;) {
        // Wait for a key press
        EFI_INPUT_KEY Key;
        Status = uefi_call_wrapper(ST->ConIn->ReadKeyStroke, 2, ST->ConIn, &Key);
        if (!EFI_ERROR(Status)) {
            Print(L"Key pressed: %c\n", Key.UnicodeChar);
        }
    }
}
