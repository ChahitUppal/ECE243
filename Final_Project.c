/* This files provides address values that exist in the system */

#define BOARD                 "DE1-SoC"

/* Memory */
#define DDR_BASE              0x00000000
#define DDR_END               0x3FFFFFFF
#define A9_ONCHIP_BASE        0xFFFF0000
#define A9_ONCHIP_END         0xFFFFFFFF
#define SDRAM_BASE            0xC0000000
#define SDRAM_END             0xC3FFFFFF
#define FPGA_ONCHIP_BASE      0xC8000000
#define FPGA_ONCHIP_END       0xC803FFFF
#define FPGA_CHAR_BASE        0xC9000000
#define FPGA_CHAR_END         0xC9001FFF

/* VGA colors */
#define WHITE 0xFFFF
#define YELLOW 0xFFE0
#define RED 0xF800
#define GREEN 0x07E0
#define BLUE 0x001F
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define GREY 0xC618
#define PINK 0xFC18
#define ORANGE 0xFC00
#define BLACK 0x0000
    
/* Cyclone V FPGA devices */
#define LEDR_BASE             0xFF200000
#define HEX3_HEX0_BASE        0xFF200020
#define HEX5_HEX4_BASE        0xFF200030
#define SW_BASE               0xFF200040
#define KEY_BASE              0xFF200050
#define JP1_BASE              0xFF200060
#define JP2_BASE              0xFF200070
#define PS2_BASE              0xFF200100
#define PS2_DUAL_BASE         0xFF200108
#define JTAG_UART_BASE        0xFF201000
#define JTAG_UART_2_BASE      0xFF201008
#define IrDA_BASE             0xFF201020
#define TIMER_BASE            0xFF202000
#define AV_CONFIG_BASE        0xFF203000
#define PIXEL_BUF_CTRL_BASE   0xFF203020
#define CHAR_BUF_CTRL_BASE    0xFF203030
#define AUDIO_BASE            0xFF203040
#define VIDEO_IN_BASE         0xFF203060
#define ADC_BASE              0xFF204000

/* Cyclone V HPS devices */
#define HPS_GPIO1_BASE        0xFF709000
#define HPS_TIMER0_BASE       0xFFC08000
#define HPS_TIMER1_BASE       0xFFC09000
#define HPS_TIMER2_BASE       0xFFD00000
#define HPS_TIMER3_BASE       0xFFD01000
#define FPGA_BRIDGE           0xFFD0501C

/* ARM A9 MPCORE devices */
#define   PERIPH_BASE         0xFFFEC000    // base address of peripheral devices
#define   MPCORE_PRIV_TIMER   0xFFFEC600    // PERIPH_BASE + 0x0600

/* Interrupt controller (GIC) CPU interface(s) */
#define MPCORE_GIC_CPUIF      0xFFFEC100    // PERIPH_BASE + 0x100
#define ICCICR                0x00          // offset to CPU interface control reg
#define ICCPMR                0x04          // offset to interrupt priority mask reg
#define ICCIAR                0x0C          // offset to interrupt acknowledge reg
#define ICCEOIR               0x10          // offset to end of interrupt reg
/* Interrupt controller (GIC) distributor interface(s) */
#define MPCORE_GIC_DIST       0xFFFED000    // PERIPH_BASE + 0x1000
#define ICDDCR                0x00          // offset to distributor control reg
#define ICDISER               0x100         // offset to interrupt set-enable regs
#define ICDICER               0x180         // offset to interrupt clear-enable regs
#define ICDIPTR               0x800         // offset to interrupt processor targets regs
#define ICDICFR               0xC00         // offset to interrupt configuration regs

#define ABS(x) (((x) > 0) ? (x) : -(x))
    
/* Screen size. */
#define RESOLUTION_X 320
#define RESOLUTION_Y 240
    
#define PI 3.1415926535
    
#define FALSE 0
#define TRUE 1

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

//Begin code
volatile int pixel_buffer_start; // global variable
int TimeOut = 10; //game timeout timer

void delay() {
    int i,j;
    for(i = 0; i < 10000000; i++) {
        for(j = 0; j < 10000000; j++) {
            
        }
    }
}

void plot_pixel(int x, int y, short int line_color)
{
    *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
}

void swap(int* a, int* b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

void wait_for_vsync() {
    volatile int *pixel_ctrl_ptr = (int*)0xFF203020;
    register int status;
    
    *pixel_ctrl_ptr = 1;
    
    status = *(pixel_ctrl_ptr + 3);
    while((status & 0x01) != 0) {
        status = *(pixel_ctrl_ptr + 3);    
    }
}

//code for HEX_display
void HEX_PS2(char b1, char b2, char b3) {
    volatile int * HEX3_HEX0_ptr = (int *)HEX3_HEX0_BASE;
    volatile int * HEX5_HEX4_ptr = (int *)HEX5_HEX4_BASE;
    /* SEVEN_SEGMENT_DECODE_TABLE gives the on/off settings for all segments in
    * a single 7-seg display in the DE1-SoC Computer, for the hex digits 0 - F
    */
    unsigned char seven_seg_decode_table[] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07,
    0x7F, 0x67, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71};
    unsigned char hex_segs[] = {0, 0, 0, 0, 0, 0, 0, 0};
    unsigned int shift_buffer, nibble;
    unsigned char code;
    int i;
    shift_buffer = (b1 << 16) | (b2 << 8) | b3;
    for (i = 0; i < 6; ++i) {
        nibble = shift_buffer & 0x0000000F; // character is in rightmost nibble
        code = seven_seg_decode_table[nibble];
        hex_segs[i] = code;
        shift_buffer = shift_buffer >> 4;
    }
    /* drive the hex displays */
    *(HEX3_HEX0_ptr) = *(int *)(hex_segs);
    *(HEX5_HEX4_ptr) = *(int *)(hex_segs + 4);
}


void clear_screen() {
    for(int x = 0; x <= 319; x++) {
        for(int y = 0; y <= 239; y++) {
            plot_pixel(x, y, 0x00ff);    
        }
    }
}


void characterType(int xPosition, int yPosition, char typingString) {
    volatile char *charBuffer = (char*) (0xC9000000 +(yPosition<<7) + xPosition);
    *charBuffer = typingString;
    plot_pixel(xPosition, yPosition, WHITE);
}

void clear_text() {
    char *stringTyping = "                                            ";
        int x = 25;

        while(*stringTyping) {
            characterType(x, 15, *stringTyping);
            x++;
            stringTyping++;
        }
    
    char *stringTyping2 = "                                             ";
    x=27;
    while(*stringTyping2) {
        characterType(x, 25, *stringTyping2);
           x++;
           stringTyping2++;
    }
    
    char *stringCourse = "                       ";
       int course_x = 65;
        while(*stringCourse) {
            characterType(course_x, 55, *stringCourse);
            course_x++;
            stringCourse++;
        }
    
    char *stringName = "                         ";
       int name_x = 0;
        while(*stringName) {
            characterType(name_x, 55, *stringName);
            name_x++;
            stringName++;
        }
    
    char *stringGame = "                           ";
       int game_x = 5;
        while(*stringGame) {
            characterType(game_x, 5, *stringGame);
            game_x++;
            stringGame++;
        }
    
    char *scorePrint = "                            ";
    int score_x = 65;
    int score_y = 3;
         while(*scorePrint) {
                characterType(score_x, score_y, *scorePrint);
                score_x++;
                scorePrint++;
             }
    
    char *endString = "                                         ";
    int mid_x = 25;
    while(*endString){
         characterType(mid_x, 30, *endString);
         mid_x++;
         endString++;
    }
            
    char *score_String = "                                   ";
    int final_x = 32;
    while(*score_String){
         characterType(final_x, 35, *score_String);
         final_x++;
         score_String++;
    }
}

/*
Draw circle - draws a pixelated circle on the screen
Parameters: (x,y) - coordinates of circle center
r - radius of circle
color - color of circle in hex
*/

void draw_solidCircle_rightBottom(int x, int y, int radius, short int line_color){
    int counter_x = 0;
    int counter_y = 0;
    
    
    for(counter_x = -radius; counter_x <= radius; ++counter_x){
        for(counter_y = -radius; counter_y <=radius; ++counter_y){
        
            if(counter_x*counter_x + counter_y*counter_y <= radius*radius){
                    if(x+counter_x < x && y+counter_y < y){
                    plot_pixel(x+counter_x,y+counter_y,line_color);
                    }
            }
        
        }
    }
}

void draw_solidCircle_leftBottom(int x, int y, int radius, short int line_color){
    int counter_x = 0;
    int counter_y = 0;
    
    
    for(counter_x = -radius; counter_x <= radius; ++counter_x){
        for(counter_y = -radius; counter_y <=radius; ++counter_y){
        
            if(counter_x*counter_x + counter_y*counter_y <= radius*radius){
                    if(x+counter_x > x && y+counter_y < y){
                    plot_pixel(x+counter_x,y+counter_y,line_color);
                    }
            }
        
        }
    }
}

void draw_solidCircle_leftTop(int x, int y, int radius, short int line_color){
    int counter_x = 0;
    int counter_y = 0;
    
    
    for(counter_x = -radius; counter_x <= radius; ++counter_x){
        for(counter_y = -radius; counter_y <=radius; ++counter_y){
        
            if(counter_x*counter_x + counter_y*counter_y <= radius*radius){
                    if(x+counter_x > x && y+counter_y > y){
                    plot_pixel(x+counter_x,y+counter_y,line_color);
                    }
            }
        
        }
    }
}

void draw_circle(int x, int y, int r, int color) {
    double angle, x1, y1;
    
    int cirx, ciry, cirx1, ciry1, cirx2, ciry2, cirx3, ciry3;
    
    for(angle = 0; angle < 90; angle+=1) {
        x1 = r * cos(angle * PI/180);
        y1 = r * sin(angle * PI/180);
        
        cirx = (int)x+x1; //Circumference coordinates of x and y
        ciry = (int)y+y1;
        
        cirx1 = (int)x-x1; //Circumference coordinates of x and y
        ciry1 = (int)y-y1;
        
        cirx2 = (int)x+x1; //Circumference coordinates of x and y
        ciry2 = (int)y-y1;
        
        cirx3 = (int)x-x1; //Circumference coordinates of x and y
        ciry3 = (int)y+y1;
        
        if(cirx >= 0 && cirx < 320 && ciry >= 0 && ciry < 240) {
            plot_pixel(cirx, ciry, color);
        }
        if(cirx1 >= 0 && cirx1 < 320 && ciry1 >= 0 && ciry1 < 240) {
            plot_pixel(cirx1, ciry1, color);
        }
        if(cirx2 >= 0 && cirx2 < 320 && ciry2 >= 0 && ciry2 < 240) {
            plot_pixel(cirx2, ciry2, color);
        }
        if(cirx3 >= 0 && cirx3 < 320 && ciry3 >= 0 && ciry3 < 240) {
            plot_pixel(cirx3, ciry3, color);
        }
    }
}

//function to convert int to string
//from stackoverflow


void clear_FIFO() {
    volatile int * ps2_base = (int *)0xFF200100;
    int ps2_data, ps2_key_pressed = 0;
    
    for(int i = 0; i < 50; i++) {
        ps2_data = (*ps2_base);
        ps2_key_pressed = ps2_data & 0xFF;
    }
}

/*
Draw ring and circle - draws a concentric circle and ring, with the ring radius converging to 0
Parameters: (x,y) - coordinates of circle & ring center
r_circle - radius of circle
r_ring - starting radius of ring
color - color of both circle and ring in hex
*/
bool toggle = true;
int score = 0;
int x_count = 0;

void draw_circle_ring(int x, int y, int r_circle, int r_ring, int color,int speed) {
    volatile int * ps2_base = (int *)0xFF200100;
    int ps2_data, ps2_key_pressed = 0;
    
    int score_x = 68;
    int score_y = 3;
    int count = 60;
    
    //Draw the circle to make sure it's always visible
    draw_circle(x, y, r_circle, color);
    
    while(r_ring > 0) {
        if(count == 50 || count ==40){
             TimeOut--;
             printf("time out: %d\n", TimeOut);
             HEX_PS2(0,0,TimeOut);
         }
        //Draw the ring
        draw_circle(x, y, r_ring, color);
        wait_for_vsync();
        if(r_ring != r_circle) {
            //Erase the ring (unless it's the same as circle, bc we want to keep circle visible at all times)
            draw_circle(x, y, r_ring, 0x00ff);
        }
        //Decease ring radius
        r_ring-=speed;
        
        ps2_data = (*ps2_base);
        ps2_key_pressed = ps2_data & 0xFF;
        if(ps2_key_pressed == 34) {
            toggle = !toggle;   
            x_count++;
        }
        
        //delay();
        count--;
     //printf("%d\n", count);
     
     if(TimeOut == 0){//game ended
         clear_screen();
        clear_text();
        printf("Final score: %d\n", score);
         while(1){//infinite loop to end the game
             char *endString = "GAME OVER : PRESS R TO RESTART";
             int mid_x = 25;
             while(*endString){
                 characterType(mid_x, 30, *endString);
                 mid_x++;
                 endString++;
            }
            
            char *score_String = "Final Score: ";
            char *score_print = "";
             itoa(score, score_print, 10);
             int final_x = 32;
             while(*score_String){
                 characterType(final_x, 35, *score_String);
                if(*(score_print-11)){
                    characterType(final_x+16, 35, *score_print);
                }
                 
                 final_x++;
                 score_String++;
                 score_print++;
             }
            ps2_data = (*ps2_base);
            ps2_key_pressed = ps2_data & 0xFF;
            if(ps2_key_pressed == 45 && toggle) {
                clear_text();
                score = 0;
                TimeOut = 10;
                x_count = 0;
                goto END_GAME;
                
            }
         }
     }
        //printf("%d\n", ps2_key_pressed);
     score_x = 65;
        if(ps2_key_pressed == 34 && toggle) { //34 is x
            *(ps2_base) = 0xFF; //reset
            clear_FIFO();
            //calculation of score
            score = score + 20 - ABS(r_ring - r_circle);
            //printf("%d\n", score);
            printf("Score: %d\n", score);
            char *scorePrint = "Score: ";
            char *score_print = "";
            itoa(score, score_print, 10);

            //while(*scorePrint) {
           //     characterType(score_x, score_y, *scorePrint);
                
           //     score_x++;
           //     scorePrint++;
                
           //}
            
         //while(*score_print){
        //     characterType(score_x, score_y, *score_print);
           //     score_x++;
               
           //     score_print++;
        // }
            
            while(*scorePrint) {
                characterType(score_x, score_y, *scorePrint);
                if(*(score_print-3)){
                    characterType(score_x+7, score_y, *score_print);
                    
                }
                
                score_x++;
             score_print++;
                scorePrint++;
            }

            //printf("%d\n", score);
            /*if(my_itoa(score, *scoreNum) != NULL){
                    while(*scoreNum){
                        characterType(score_x, score_y, *scoreNum);
                        score_x++;
                        scoreNum++;
                     }
             }*/
            break;    
        }
     else if(r_ring == 0){
         
    }
    
  }
    END_GAME:
    //Erase the circle
    draw_circle(x, y, r_circle, 0xff1111);
}

int main(void)
{
    
    
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
    volatile int * ps2_base = (int *)0xFF200100;
    
    START_GAME:
    
    /* Read location of the pixel buffer from the pixel buffer controller */
    pixel_buffer_start = *pixel_ctrl_ptr;
    score = 0;
    int speed = 1;
    clear_screen();
    clear_text();
    
    int ps2_data, ps2_key_pressed;
    clear_screen();
    //while(1){
    draw_solidCircle_rightBottom(320,240, 65, 0x0000);
    char *stringCourse = "ECE243 PROJECT";
       int course_x = 65;
        while(*stringCourse) {
            characterType(course_x, 55, *stringCourse);
            course_x++;
            stringCourse++;
        }
    
    draw_solidCircle_leftBottom(0,240,65,0x0000);
    char *stringName = "SMARAN & CHAHIT";
       int name_x = 0;
        while(*stringName) {
            characterType(name_x, 55, *stringName);
            name_x++;
            stringName++;
        }
    
    draw_solidCircle_leftTop(0,0,65,0x0000);
    char *stringGame = "OSU";
       int game_x = 5;
        while(*stringGame) {
            characterType(game_x, 5, *stringGame);
            game_x++;
            stringGame++;
        }
    //}
    
    //Start screen
    while(1) {
        
        char *stringTyping = "CHOOSE SPEED LEVEL TO START";
        int x = 25;

        while(*stringTyping) {
            characterType(x, 15, *stringTyping);
            x++;
            stringTyping++;
        }
        
     char *stringTyping2 = "LEVELS: 1      2      3";
        int x2 = 27;

        while(*stringTyping2) {
            characterType(x2, 25, *stringTyping2);
            x2++;
            stringTyping2++;
        }
        
     
        
        ps2_data = (*ps2_base);
        ps2_key_pressed = ps2_data & 0xFF;
        
        if(ps2_key_pressed == 90) { //90 is enter
            clear_text();
         break;    
        }
     else if(ps2_key_pressed == 105 || ps2_key_pressed == 22){
         speed =1;
         clear_text();
         break;
        }
     else if(ps2_key_pressed == 114 || ps2_key_pressed == 30){
         speed = 2;
         clear_text();
         break;
     }
     else if(ps2_key_pressed == 122 || ps2_key_pressed == 38){
         speed = 3;
         clear_text();
         break;
     }
    }
    
    int toggle = 0;
    
    //Start of game
    clear_text();
    while(1) {
         clear_screen();
        if(toggle == 1) {
            draw_circle_ring(rand()%320, rand()%240, 20, 40, rand()%0x0FFF + 0xF000, speed);
            toggle = 0;
        }
        toggle++;
        
        ps2_data = (*ps2_base);
           ps2_key_pressed = ps2_data & 0xFF;
        if(ps2_key_pressed ==  45){
            goto START_GAME;
        }
    }
    
}
    














