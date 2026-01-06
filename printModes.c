/*
    a simple helper program to get all available video modes used by gope->setmodes
*/
#include <efi.h>
#include <efilib.h>

EFI_SYSTEM_TABLE *gST;
EFI_BOOT_SERVICES *gBS;
EFI_GUID gEfiGraphicsOutputProtocolGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;

void PrintNum(UINT64 Value) {
    CHAR16 Buffer[50];
    int i = 0;
    
    if (Value == 0) {
        gST->ConOut->OutputString(gST->ConOut, L"0");
        return;
    }
    while (Value > 0) {
        Buffer[i++] = (Value % 10) + '0';
        Value /= 10;
    }

    CHAR16 Single[2];
    Single[1] = '\0';
    while (i > 0) {
        Single[0] = Buffer[--i];
        gST->ConOut->OutputString(gST->ConOut, Single);
    }
}

EFI_STATUS
EFIAPI
efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    
    gST = SystemTable;
    gBS = SystemTable->BootServices;
    
    EFI_STATUS Status;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop;
    
    gST->ConOut->ClearScreen(gST->ConOut);
    gST->ConOut->OutputString(gST->ConOut, L"--- Available GOP Modes ---\r\n\r\n");

    Status = gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, NULL, (VOID**)&Gop);
    if (EFI_ERROR(Status)) {
        gST->ConOut->OutputString(gST->ConOut, L"Error: GOP not found.\r\n");
        return Status;
    }

    UINT32 MaxMode = Gop->Mode->MaxMode;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;
    UINTN SizeOfInfo;

    for (UINT32 i = 0; i < MaxMode; i++) {
        Status = Gop->QueryMode(Gop, i, &SizeOfInfo, &Info);
        if (EFI_ERROR(Status)) continue;

        gST->ConOut->OutputString(gST->ConOut, L"Mode ");
        PrintNum(i);
        gST->ConOut->OutputString(gST->ConOut, L": ");
        
        PrintNum(Info->HorizontalResolution);
        gST->ConOut->OutputString(gST->ConOut, L" x ");
        PrintNum(Info->VerticalResolution);

        if (i == Gop->Mode->Mode) {
            gST->ConOut->OutputString(gST->ConOut, L"  <-- CURRENT");
        }

        gST->ConOut->OutputString(gST->ConOut, L"\r\n");
    }

    gST->ConOut->OutputString(gST->ConOut, L"\r\nPress any key to exit...");

    UINTN Index;
    gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, &Index);

    return EFI_SUCCESS;
}
