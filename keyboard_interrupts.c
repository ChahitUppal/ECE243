/* This files provides address values that exist in the system */

#define BOARD             	"DE1-SoC"

/* Memory */
#define DDR_BASE          	0x00000000
#define DDR_END           	0x3FFFFFFF
#define A9_ONCHIP_BASE    	0xFFFF0000
#define A9_ONCHIP_END     	0xFFFFFFFF
#define SDRAM_BASE        	0xC0000000
#define SDRAM_END         	0xC3FFFFFF
#define FPGA_ONCHIP_BASE  	0xC8000000
#define FPGA_ONCHIP_END   	0xC803FFFF
#define FPGA_CHAR_BASE    	0xC9000000
#define FPGA_CHAR_END     	0xC9001FFF

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
#define LEDR_BASE         	0xFF200000
#define HEX3_HEX0_BASE    	0xFF200020
#define HEX5_HEX4_BASE    	0xFF200030
#define SW_BASE           	0xFF200040
#define KEY_BASE          	0xFF200050
#define JP1_BASE          	0xFF200060
#define JP2_BASE          	0xFF200070
#define PS2_BASE          	0xFF200100
#define PS2_DUAL_BASE     	0xFF200108
#define JTAG_UART_BASE    	0xFF201000
#define JTAG_UART_2_BASE  	0xFF201008
#define IrDA_BASE         	0xFF201020
#define TIMER_BASE        	0xFF202000
#define AV_CONFIG_BASE    	0xFF203000
#define PIXEL_BUF_CTRL_BASE   0xFF203020
#define CHAR_BUF_CTRL_BASE	0xFF203030
#define AUDIO_BASE        	0xFF203040
#define VIDEO_IN_BASE     	0xFF203060
#define ADC_BASE          	0xFF204000

/* Cyclone V HPS devices */
#define HPS_GPIO1_BASE    	0xFF709000
#define HPS_TIMER0_BASE   	0xFFC08000
#define HPS_TIMER1_BASE   	0xFFC09000
#define HPS_TIMER2_BASE   	0xFFD00000
#define HPS_TIMER3_BASE   	0xFFD01000
#define FPGA_BRIDGE       	0xFFD0501C

/* ARM A9 MPCORE devices */
#define   PERIPH_BASE     	0xFFFEC000	// base address of peripheral devices
#define   MPCORE_PRIV_TIMER   0xFFFEC600	// PERIPH_BASE + 0x0600

/* Interrupt controller (GIC) CPU interface(s) */
#define MPCORE_GIC_CPUIF  	0xFFFEC100	// PERIPH_BASE + 0x100
#define ICCICR            	0x00      	// offset to CPU interface control reg
#define ICCPMR            	0x04      	// offset to interrupt priority mask reg
#define ICCIAR            	0x0C      	// offset to interrupt acknowledge reg
#define ICCEOIR           	0x10      	// offset to end of interrupt reg
/* Interrupt controller (GIC) distributor interface(s) */
#define MPCORE_GIC_DIST   	0xFFFED000	// PERIPH_BASE + 0x1000
#define ICDDCR            	0x00      	// offset to distributor control reg
#define ICDISER           	0x100     	// offset to interrupt set-enable regs
#define ICDICER           	0x180     	// offset to interrupt clear-enable regs
#define ICDIPTR           	0x800     	// offset to interrupt processor targets regs
#define ICDICFR           	0xC00     	// offset to interrupt configuration regs

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
double score = 0.0; //global variable
bool keyboard_interrupt = false;

/*-----Start of interrupt stuff-----*/
void config_PS2(){
    volatile int * PS2_Control = (int *) 0xFF200104;
    *PS2_Control = 0x00000001;  //enable interrupts for PS2
    printf("%#010x\n", *PS2_Control);
}
 
void __attribute__ ((interrupt)) __cs3_isr_irq (void){
    printf("INTERRUPT DETECTED!\n");

    // Read the ICCIAR from the processor interface
    int int_ID = *((int *) 0xFFFEC10C);
    if (int_ID == 79) { // check if interrupt is from the HPS timer
   	 PS2_ISR();
    }
    else {
   	 while (1) { // if unexpected, then stay here
   	 
   	 }
    }

    // Write to the End of Interrupt Register (ICCEOIR)
    *((int *) 0xFFFEC110) = int_ID;
    return;
}

// Define the remaining exception handlers */
void __attribute__ ((interrupt)) __cs3_isr_undef (void) {
 while (1);
}
void __attribute__ ((interrupt)) __cs3_isr_swi (void) {
 while (1);
}
void __attribute__ ((interrupt)) __cs3_isr_pabort (void) {
 while (1);
}
void __attribute__ ((interrupt)) __cs3_isr_dabort (void) {
 while (1);
}
void __attribute__ ((interrupt)) __cs3_isr_fiq (void) {
 while (1);
}


void disable_A9_interrupts(void){
    int status = 0b11010011;
    asm("msr cpsr, %[ps]" : : [ps]"r"(status));
}

void enable_A9_interrupts(void){
    int status = 0b01010011;
    asm("msr cpsr, %[ps]" : : [ps]"r"(status));
}

void config_GIC(void) {
    config_interrupt (79, 1); // configure the KEYs parallel port (Interrupt ID = 79)
    // Set Interrupt Priority Mask Register (ICCPMR). Enable interrupts of all priorities
    *((int *) 0xFFFEC104) = 0xFFFF;
    // Set CPU Interface Control Register (ICCICR). Enable signaling of interrupts
    *((int *) 0xFFFEC100) = 1;
    // Configure the Distributor Control Register (ICDDCR) to send pending interrupts to CPUs
    *((int *) 0xFFFED000) = 1;
}

void config_interrupt (int N, int CPU_target) {
    int reg_offset, index, value, address;

    /* Configure the Interrupt Set-Enable Registers (ICDISERn).
     * reg_offset = (integer_div(N / 32) * 4
     * value = 1 << (N mod 32) */

    reg_offset = (N >> 3) & 0xFFFFFFFC;
    index = N & 0x1F;
    value = 0x1 << index;
    address = 0xFFFED100 + reg_offset;

    /* Now that we know the register address and value, set the appropriate bit */ *(int *)address |= value;
    *(int *)address |= value;

    /* Configure the Interrupt Processor Targets Register (ICDIPTRn)
     * reg_offset = integer_div(N / 4) * 4
     * index = N mod 4 */

    reg_offset = (N & 0xFFFFFFFC);
    index = N & 0x3;
    address = 0xFFFED800 + reg_offset + index;

    /* Now that we know the register address and value, write to (only) the appropriate byte */
    *(char *)address = (char) CPU_target;
}

void set_A9_IRQ_stack(void){
    int stack, mode;
    stack = 0xFFFFFFFF - 7;// top of A9 on-chip memory, aligned to 8 bytes

    /* change processor to IRQ mode with interrupts disabled */
    mode = 0b11010010;
    asm("msr cpsr, %[ps]" : : [ps] "r" (mode));
    /* set banked stack pointer */
    asm("mov sp, %[ps]" : : [ps] "r" (stack));

    /* go back to SVC mode before executing subroutine return! */
    mode = 0b11010011;
    asm("msr cpsr, %[ps]" : : [ps] "r" (mode));

    //printf("Done setting up stack\n");
}


void PS2_ISR(){
    printf("IN PS2_ISR\n");
    volatile int * PS2_Control = (int *) 0xFF200104;
    *PS2_Control = 1;  // clear RI bit

    volatile int * PS2_ptr = (int *) 0xFF200100; // PS/2 port address
    int PS2_data, RVALID;
    char key_pressed = 0;

    PS2_data = *(PS2_ptr);
    RVALID = PS2_data & 0x8000;
    
    if (RVALID){
   	 key_pressed = PS2_data & 0xFF;
   	 printf("%#010x\n", key_pressed);
   	 keyboard_interrupt = true;
    }
}
/*-----End of interrupt stuff-----*/

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
    volatile int *pixel_ctrl_ptr = 0xFF203020;
    register int status;
    
    *pixel_ctrl_ptr = 1;
    
    status = *(pixel_ctrl_ptr + 3);
    while((status & 0x01) != 0) {
   	 status = *(pixel_ctrl_ptr + 3);    
    }
}

void clear_screen() {
    for(int x = 0; x <= 319; x++) {
   	 for(int y = 0; y <= 239; y++) {
   		 plot_pixel(x, y, 0x0000);    
   	 }
    }
}

/*
Draw circle - draws a pixelated circle on the screen
Parameters: (x,y) - coordinates of circle center
r - radius of circle
color - color of circle in hex
*/
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

/*
Draw ring and circle - draws a concentric circle and ring, with the ring radius converging to 0
Parameters: (x,y) - coordinates of circle & ring center
r_circle - radius of circle
r_ring - starting radius of ring
color - color of both circle and ring in hex
*/
void draw_circle_ring(int x, int y, int r_circle, int r_ring, int color) {
    //Draw the circle to make sure it's always visible
    draw_circle(x, y, r_circle, color);
    
    while(r_ring > 0) {
   	 //Draw the ring
   	 draw_circle(x, y, r_ring, color);
   	 wait_for_vsync();
   	 if(r_ring != r_circle) {
   		 //Erase the ring (unless it's the same as circle, bc we want to keep circle visible at all times)
   		 draw_circle(x, y, r_ring, BLACK);
   	 }
   	 //Decease ring radius
   	 r_ring--;
   	 
   	 //Get score if interrupted
   	 if(keyboard_interrupt) {
   		 score = score + 20.0 - ABS(r_ring - r_circle);
   		 printf("Score: %f", score);
   	 }
    }
    //Erase the circle
    draw_circle(x, y, r_circle, BLACK);
}

int main(void)
{
    disable_A9_interrupts();
    set_A9_IRQ_stack();
    config_GIC();
    config_PS2();

    enable_A9_interrupts();
    
	volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
	/* Read location of the pixel buffer from the pixel buffer controller */
	pixel_buffer_start = *pixel_ctrl_ptr;
    
    clear_screen();
    
    //ps2 keyboard stuff
    volatile int * ps2_ptr = (int *)PS2_BASE;
    int ps2_data, RVALID, ps2_key;
    *(ps2_ptr) = 0xFF; //reset
    
    //main loop
    while(1) {
   	 /*ps2_data = *(ps2_ptr);
   	 RVALID = ps2_data & 0x8000;
   	 ps2_key = ps2_data & 0xFF;
   	 
   	 if(RVALID) {
   		 printf("%d\n", ps2_key);
   	 }*/
   	 draw_circle_ring(rand()%320, rand()%240, 20, 40, rand()%0x0FFF + 0xF000);
    }
    
    
}
    
	

