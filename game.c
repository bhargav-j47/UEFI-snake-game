/* 
    simple snake game using gnu-efi that runs as boot service on a UEFI
*/
#include <efi.h>
#include <efilib.h>

//constants
#define MAX_SNAKE_LENGTH 1024
#define BLOCK_SIZE 20

//colors
#define COL_ATTR(fg, bg) ((fg) | (bg))
#define COLOR_BLACK  0x00000000
#define COLOR_WHITE  0x00FFFFFF
#define COLOR_RED    0x00FF0000
#define COLOR_GREEN  0x0000FF00
#define COLOR_BLUE   0x000000FF

typedef struct{ 
    UINTN x;
    UINTN y;
}Point;

Point snake[MAX_SNAKE_LENGTH];
Point dir;
Point food;
UINTN SnakeLen;

EFI_SYSTEM_TABLE *est;
EFI_BOOT_SERVICES *ebs; 
EFI_GUID gEfiGopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;


EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
UINT32 *VideoBuffer;
UINT32 ScreenWidth;
UINT32 ScreenHeight;

INT16 GridWidth;
INT16 GridHeight;

void draw_block(UINT16 x,UINT16 y,UINTN col){
    
    UINT32 sX=x*BLOCK_SIZE; 
    UINT32 sY=y*BLOCK_SIZE; 

    if (sX + BLOCK_SIZE > ScreenWidth || sY + BLOCK_SIZE > ScreenHeight) return;

    for(UINT32 iy=0;iy<BLOCK_SIZE;iy++){    
        for(UINT32 ix=0;ix<BLOCK_SIZE;ix++){
            
            UINT32 offset = (sY+iy)*ScreenWidth +(sX+ix);
            VideoBuffer[offset]=col;
        }
    }
}

void draw_border(){
    for(int x=0;x<GridWidth;x++){
        draw_block(x, 0, COLOR_BLUE);               
        draw_block(x, GridHeight - 1, COLOR_BLUE); 
    }
    for (int y=0;y<GridHeight;y++){
        draw_block(0, y, COLOR_BLUE);               
        draw_block(GridWidth - 1, y,COLOR_BLUE); 
    }
}

static UINT64 seed = 12345;
INT16 Rand(INT16 limit) {
    seed = seed * 6364136223846793005ULL + 1;
    return (INT16)((seed >> 32) % limit);
}

void spawn_food(){
    UINTN isValid=0;
    
    while(!isValid){
        isValid=1;

        food.x = Rand(GridWidth - 2) + 1;
        food.y = Rand(GridHeight - 2) + 1;
        
        for(UINTN i=0;i<SnakeLen;i++){
            if(snake[i].x==food.x && snake[i].y==food.y){
                isValid=0;
                break;
            }
        }
    }

    draw_block(food.x, food.y, COLOR_RED);
}

void clear_screen(){
    UINT32 totalPixels = ScreenWidth * ScreenHeight;
    for(UINT32 i=0;i<totalPixels;i++){
        VideoBuffer[i] = COLOR_BLACK;
    }
}

void reset_game(){
    SnakeLen=5;
    dir=(Point){1,0};
    snake[0]=(Point){GridWidth/2 , GridHeight/2};
    for(int i = 1; i < SnakeLen; i++) {
        snake[i] = snake[0];
        snake[i].x -= i;
    }
    
    clear_screen();
    draw_border();
    spawn_food();
}

EFI_STATUS
EFIAPI
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable){

    //InitializeLib(ImageHandle, SystemTable);
    est=SystemTable;
    ebs=SystemTable->BootServices;
    EFI_STATUS status;
    EFI_INPUT_KEY key;

    status = ebs->LocateProtocol(&gEfiGopGuid, NULL, (VOID**)&gop);
    if (EFI_ERROR(status)) return status;

    est->ConOut->OutputString(est->ConOut,L"");
    
    status = gop->SetMode(gop,0); // 22:1920*1080 , 23:1920*1200, 28:2560*1440 29:2560*1600  on real hardware use mode 0 only
    if (EFI_ERROR(status)) return status;
 
    VideoBuffer = (UINT32*)(UINTN)gop->Mode->FrameBufferBase;
    ScreenWidth = gop->Mode->Info->HorizontalResolution;
    ScreenHeight = gop->Mode->Info->VerticalResolution;

    //for real hardware use below
    GridWidth = (ScreenWidth/BLOCK_SIZE);       
    GridHeight = (ScreenHeight/BLOCK_SIZE);
    
    /* for qemu use below
    GridWidth = (ScreenWidth/BLOCK_SIZE) - 65;
    GridHeight = (ScreenHeight/BLOCK_SIZE) - 44;
    */

    ebs->SetWatchdogTimer(0, 0, 0, NULL);

    est->ConOut->EnableCursor(est->ConOut,FALSE);
    reset_game();

    while(1){
        
        //handle keystrokes
        status=est->ConIn->ReadKeyStroke(est->ConIn,&key);
        if(status==EFI_SUCCESS){
            if(key.UnicodeChar=='w'|| key.ScanCode == SCAN_UP){
                if(dir.y==0) dir=(Point){0,-1};
            }
            else if(key.UnicodeChar=='a' || key.ScanCode == SCAN_LEFT){
                if(dir.x==0) dir=(Point){-1,0};
            } 
            else if(key.UnicodeChar=='s' || key.ScanCode == SCAN_DOWN){
                if(dir.y==0) dir=(Point){0,1};
            }
            else if(key.UnicodeChar=='d' || key.ScanCode == SCAN_RIGHT){
                if(dir.x==0) dir=(Point){1,0};
            }
        }
        
        //str tail
        Point Tail=snake[SnakeLen-1];

        for (int i = SnakeLen - 1; i > 0; i--) {
            snake[i] = snake[i - 1];
        }
        snake[0].x += dir.x;
        snake[0].y += dir.y;
        
        //handle wall collision

        if(snake[0].x<=0 ||snake[0].x>=GridWidth-1 || snake[0].y<=0 || snake[0].y>=GridHeight-1) goto Game_Over;

        for(int i=1;i<SnakeLen;i++){
            if(snake[0].x==snake[i].x && snake[0].y==snake[i].y) goto Game_Over;
        }

        //handle food collision
        if(snake[0].x==food.x && snake[0].y==food.y){
            spawn_food();
            SnakeLen++;
            snake[SnakeLen-1]=Tail;
        }else{
            draw_block(Tail.x,Tail.y,COLOR_BLACK);
        }

        //draw newhead
        draw_block(snake[0].x,snake[0].y,COLOR_GREEN);
        
        ebs->Stall(90000);
    }

Game_Over:
    UINTN Index;
    est->ConOut->SetAttribute(est->ConOut, EFI_TEXT_ATTR(EFI_WHITE, EFI_BACKGROUND_BLACK));
    est->ConOut->ClearScreen(est->ConOut);
    clear_screen();
    est->ConOut->EnableCursor(est->ConOut,TRUE);
    ebs->WaitForEvent(1, &est->ConIn->WaitForKey, &Index);

    return EFI_SUCCESS;
}

