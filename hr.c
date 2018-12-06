#include <pic.h>

// Definitions
#define HRIN RA0  // Signal in
#define HROUT RB2 // Pulse
#define STAT RA1  // Status
#define LED0 RB4  // LSB
#define LED1 RB5
#define LED2 RB1
#define LED3 RB0
#define LED4 RA4
#define LED5 RA3
#define LED6 RA2 // MSB



// macros for a C bit array
// http://www.mathcs.emory.edu/~cheung/Courses/255/Syllabus/1-C-intro/bit-array.html
#define setBit(A,k)     ( A[(k/32)] |= (1 << (k%32)) )
#define clearBit(A,k)   ( A[(k/32)] &= ~(1 << (k%32)) )
#define getBit(A,k)     ( A[(k/32)] & (1 << (k%32)) )

// Variables
unsigned int threshold = 191;
int hrHistory[2];
int histSize = 64;
int frameScale = 5;
int hrLoc = 0;
int hrPulses;
int hrBPM;

// parsing ADC input
unsigned int readADC(void) {
    GO_DONE = 1;
    while (GO_DONE); // wait until conversion done
    return ADRESH;   // get and return stored ADC val (upper 8 bits of 10)
}

// delay for testing
void delay(unsigned int amount){
    unsigned int index;
    for (index = 0; index < amount; index++);
}

// looping section of code
void loop(){

    // get current value of signal
    unsigned int hrSig = readADC();

    // move signal into array, shift
    if(hrSig > threshold){
        setBit(hrHistory, hrLoc); HROUT = 1;
    } else {
        clearBit(hrHistory, hrLoc);  HROUT = 0;
    }

    // look for pulse detections in array
    hrPulses = 0;

    // increment HR count - rising edge detect
    for(int i = 1; i<histSize; i++ ){
        // if cur hist value high, and last val low, increment
        if(getBit(hrHistory,i) && (!getBit(hrHistory,i-1))) hrPulses++;
    }

    // scale frame HR to BPM
    hrBPM = hrPulses * frameScale;

    // turn status LED on (RB3), if weird HR
    STAT = ( hrBPM > 220 | hrBPM < 30 )? 1 : 0;

    // print out BPM - binary to LEDs
    // http://www.programmingsimplified.com/c/source-code/c-program-convert-decimal-to-binary
    for (int c = 6; c >= 0; c--) {

        unsigned int k = hrBPM >> c;
        unsigned int status = k & 1;

        if (c == 6){
            LED6 = k;
        }else if (c == 5){
            LED5 = k;
        }else if (c == 4){
            LED4 = k;
        }else if (c == 3){
            LED3 = k;
        }else if (c == 2){
            LED2 = k;
        }else if (c == 1){
            LED1 = k;
        }else if (c == 0){
            LED0 = k;
        }

    }

    // check if index needs resetting
    hrLoc++; // increment value
    if(hrLoc == histSize){
        hrLoc = 0;
    }

}

// main code, declarations, loops
void main(void) {
    CMCON  = 7;          //turn off comparator
    ADCON0 = 0b00000001; //to read from AN0
    ANSEL = 0b00011000;  //0=digital,1=analog
    TRISA = 0b00000001;  //0=output, 1=input
    TRISB = 0b00000000;  //0=output, 1=input
    while(1) loop();
}
