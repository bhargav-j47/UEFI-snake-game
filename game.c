/* 
*/

#include <efi.h>
#include <efilib.h>

//constants
#define SCREEN_WIDTH 100
#define SCREEN_HEIGHT 31
#define MAX_SNAKE_LENGTH 1024

//colors
#define COL_ATTR(fg, bg) ((fg) | (bg))

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

void draw_char(UINTN x,UINTN y,UINTN col){
    est->ConOut->SetCursorPosition(est->ConOut, x, y);
    est->ConOut->SetAttribute(est->ConOut,col);
    est->ConOut->OutputString(est->ConOut,L" ");
}

void draw_border(){
    for (int x = 0; x < SCREEN_WIDTH; x++) {
        draw_char(x, 0, COL_ATTR(EFI_WHITE,EFI_BACKGROUND_BLUE));               
        draw_char(x, SCREEN_HEIGHT - 1, COL_ATTR(EFI_WHITE,EFI_BACKGROUND_BLUE)); 
    }

    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        draw_char(0, y, COL_ATTR(EFI_WHITE,EFI_BACKGROUND_BLUE));               
        draw_char(SCREEN_WIDTH - 1, y,COL_ATTR(EFI_WHITE,EFI_BACKGROUND_BLUE)); 
    }
}

static UINT64 seed = 12345;
INT16 Rand(INT16 limit) {
    seed = seed * 6364136223846793005ULL + 1;
    return (INT16)((seed >> 32) % limit);
}

void spawn_food(){
    food.x = Rand(SCREEN_WIDTH - 2) + 1;
    food.y = Rand(SCREEN_HEIGHT - 2) + 1;
    draw_char(food.x, food.y, EFI_BACKGROUND_RED);
}

void reset_game(){
    SnakeLen=5;
    dir=(Point){1,0};
    snake[0]=(Point){SCREEN_WIDTH/2 , SCREEN_HEIGHT/2};
    for(int i = 1; i < SnakeLen; i++) {
        snake[i] = snake[0];
        snake[i].x -= i;
    }
    
    est->ConOut->SetAttribute(est->ConOut, EFI_TEXT_ATTR(EFI_WHITE, EFI_BACKGROUND_BLACK));
    est->ConOut->ClearScreen(est->ConOut);
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

    est->ConOut->EnableCursor(est->ConOut,FALSE);
    reset_game();

    while(1){
        
        //handle keystrokes
        status=est->ConIn->ReadKeyStroke(est->ConIn,&key);
        if(status==EFI_SUCCESS){
            if(key.UnicodeChar=='w'){
                if(dir.y==0) dir=(Point){0,-1};
            }
            else if(key.UnicodeChar=='a'){
                if(dir.x==0) dir=(Point){-1,0};
            } 
            else if(key.UnicodeChar=='s'){
                if(dir.y==0) dir=(Point){0,1};
            }
            else if(key.UnicodeChar=='d'){
                if(dir.x==0) dir=(Point){1,0};
            }
        }
        
        //clear tail
        draw_char(snake[SnakeLen-1].x, snake[SnakeLen-1].y, EFI_BACKGROUND_BLACK);

        for (int i = SnakeLen - 1; i > 0; i--) {
            snake[i] = snake[i - 1];
        }
        snake[0].x += dir.x;
        snake[0].y += dir.y;
        
        //handle wall collision

        if(snake[0].x<0 ||snake[0].x>=SCREEN_WIDTH || snake[0].y<0 || snake[0].y>=SCREEN_HEIGHT) break;

        for(int i=1;i<SnakeLen;i++){
            if(snake[0].x==snake[i].x && snake[0].y==snake[i].y) goto Game_Over;
        }

        //handle food collision
        
        if(snake[0].x==food.x && snake[0].y==food.y){
            spawn_food();
            SnakeLen++;
        }

        //draw newhead
        draw_char(snake[0].x,snake[0].y,EFI_BACKGROUND_GREEN);
        
        ebs->Stall(70000);
    }

Game_Over:
    UINTN Index;
    est->ConOut->SetAttribute(est->ConOut, EFI_TEXT_ATTR(EFI_WHITE, EFI_BACKGROUND_BLACK));
    est->ConOut->ClearScreen(est->ConOut);
    est->ConOut->EnableCursor(est->ConOut,TRUE);
    ebs->WaitForEvent(1, &est->ConIn->WaitForKey, &Index);

    return EFI_SUCCESS;
}



//snake:= array of points with x and y coordinates
//define a screen 
//food := using random 
//in while loop
//check for keys
//remove last snake trail
//move snake from head to tail
//draw snake
//handle collision with food and with walls
//End while
//wait for a key if r restart .. if q exit and return

