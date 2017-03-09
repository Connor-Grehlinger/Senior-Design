#include<p32xxxx.h>
#include<plib.h>
#include<time.h>
#include<stdlib.h>
#define SYS_FREQ     80000000L
#pragma config FPLLMUL = MUL_20, FPLLIDIV = DIV_2, FPLLODIV = DIV_1, FWDTEN = OFF
#pragma config POSCMOD = HS, FNOSC = PRIPLL, FPBDIV = DIV_1

/*          Board I/O Pins          */

// KeyPad Pmod jumper JJ
// Columns (PORT for input)
#define C1   PORTBbits.RB3
#define C2   PORTBbits.RB2
#define C3   PORTBbits.RB1
#define C4   PORTBbits.RB0
// Rows (LAT for output)
#define R1   LATBbits.LATB9 //4
#define R2   LATBbits.LATB8 //5
#define R3   LATBbits.LATB5 //8
#define R4   LATBbits.LATB4 //9


// 7 Segment Display Pmod for using the TOP JA & JB jumpers
#define SegA    LATEbits.LATE0
#define SegB    LATEbits.LATE1
#define SegC    LATEbits.LATE2
#define SegD    LATEbits.LATE3
#define SegE    LATGbits.LATG9
#define SegF    LATGbits.LATG8
#define SegG    LATGbits.LATG7
#define DispSel_1 LATGbits.LATG6    // SSD selector, 0 = right digit, 1 = left digit

// 7 Segment Display Pmod for using the top of JC & JB jumpers
#define SegA2    LATGbits.LATG12
#define SegB2    LATGbits.LATG13
#define SegC2    LATGbits.LATG14
#define SegD2    LATGbits.LATG15
#define SegE2    LATDbits.LATD7
#define SegF2    LATDbits.LATD1
#define SegG2    LATDbits.LATD9
#define DispSel_2 LATCbits.LATC1    // SSD selector, 0 = right digit, 1 = left digit

// 8LED Pmod jumper JH
#define PLed1   LATFbits.LATF12
#define PLed2   LATFbits.LATF5
#define PLed3   LATFbits.LATF4
#define PLed4   LATFbits.LATF13
#define PLed5   LATEbits.LATE8
#define PLed6   LATDbits.LATD0
#define PLed7   LATDbits.LATD8
#define PLed8   LATDbits.LATD13

// LS1 Pmod jumper JF

#define LS11    PORTAbits.RA14  // pretty sure only this one needs to be used

// Mic Pmod (also happens to be same PORTB pins for on-board LEDs)
#define MICSIG	PORTBbits.RB11  // RB11 is analog output pin

/*          Global Variables            */

unsigned char number[]={
    0b0111111,   //0
    0b0000110,   //1
    0b1011011,   //2
    0b1001111,   //3
    0b1100110,   //4
    0b1101101,   //5
    0b1111101,   //6
    0b0000111,   //7
    0b1111111,   //8
    0b1101111,   //9
    0b1110111,   //A
    0b1111100,   //b
    0b0111001,   //C
    0b1011110,   //d
    0b1111001,   //E
    0b1110001,   //F
    0b0000000,   //clear
    0b0111000,   //L
    0b1110110,   //H
    0b1111001,   //E
    0b0111001    //C
};

enum Mode {Left,Right}; // controls which digit to display on SSDs
int dummyRead;  // to help manage CN pins
int inputKey = -1;  // -1 should not be recognized (null value)
unsigned int dig0 = 0;
unsigned int dig1 = 0;
unsigned int dig2 = 0;
unsigned int dig3 = 0;
unsigned char buttonLock = 0;
unsigned int i; // used in loop functions 
unsigned int secondCount; // used in Timer ISR
enum Mode mode = Left;
unsigned int ticketDig3;
unsigned int ticketDig2;
unsigned int ticketDig1;
unsigned int ticketDig0;
unsigned int inputTicket;
unsigned int ticketArrayLength;
unsigned char ControllerMode = 1;
unsigned int capacity = 0;
unsigned char ModeChange = 0;
unsigned char ClearKey = 0;
unsigned char DeleteKey = 0;
unsigned char NumberKey = 0;
unsigned char activateCountDown = 0;
unsigned char ticketTrigger = 0;
int PLedCounter;
unsigned short arrayGenerate = 1;
unsigned short generateTicket = 1;
int ticket;
unsigned int error = 0;
int tickets[100];
unsigned int openSpaces;
unsigned char mode3Hit = 0;
unsigned int seed = 2;
unsigned char correctTicket = 0;
unsigned char wrongPasscode = 0;
unsigned char pledOn = 1;
unsigned int mode5Counter = 0;
unsigned short mode5Hit = 0;
unsigned short mode4Hit = 0;

/*          Function Declarations       */

void displayDigit(unsigned char, unsigned char, unsigned char, unsigned char, unsigned char);
void generateTicketDisplay(int);
void shiftValue(int);
unsigned char checkTicketInput();
unsigned char checkTicketInputMode4();
unsigned char isInArray(int, int*);
void removeFromArray(int, int*);
unsigned char isFull(int*);
unsigned int freeSpaces(int*);
unsigned int arrayLength;
int readADC();

/*          Functions                   */

// Functions for ticket handling 
unsigned char isInArray(int num, int* array){
    for ( i = 0; i < ticketArrayLength; i++){
        if (array[i] == num){
            return 1;
        }
    }
    return 0;
}

void removeFromArray(int num, int* array){
    if (isInArray(num, array) == 1){
        //execute removal code
        for(i = 0; i<ticketArrayLength ; i++){
            if (array[i] == num){
                //make array negative. It could be set to either value
                array[i] = -1;
            }
        }
    }
}

void addToArray(int num, int* array){
    unsigned short added = 0;
    for(i = 0; i <ticketArrayLength; i++){
        if(array[i] == -1 && !added){
            array[i] = num;
            added = 1;
        }
    }
}

//returns 0 if it is not full, else returns 1
unsigned char isFull(int* array){
    for (i = 0; i < ticketArrayLength; i++){
        if (array[i] == -1){
            return 0;
        }
    }
    return 1;
}

unsigned int freeSpaces(int* array){
    int counter = 0;
    for (i = 0; i < ticketArrayLength; i++){
        if (array[i] == -1){
            counter++;
        }
    }
    return counter;
}

void initializeArray(int* array){
    for (i = 0; i < 100; i++){
        array[i] = -1;
    }
}


void displayDigit (unsigned char mode, unsigned char digit3, unsigned char digit2, unsigned char digit1, unsigned char digit0){
    DispSel_1 = mode;   // digits 0 and 1
    DispSel_2 = mode;   // digits 2 and 3
    if(DispSel_1 == Right){
        SegA    = digit1 & 1;
        SegB    = (digit1 >> 1) & 1;
        SegC    = (digit1 >> 2) & 1;
        SegD    = (digit1 >> 3) & 1;
        SegE    = (digit1 >> 4) & 1;
        SegF    = (digit1 >> 5) & 1;
        SegG    = (digit1 >> 6) & 1;
    }
    else if(DispSel_1 == Left){
        SegA    = digit0 & 1;
        SegB    = (digit0 >> 1) & 1;
        SegC    = (digit0 >> 2) & 1;
        SegD    = (digit0 >> 3) & 1;
        SegE    = (digit0 >> 4) & 1;
        SegF    = (digit0 >> 5) & 1;
        SegG    = (digit0 >> 6) & 1;
    }
    if (DispSel_2 == Right){
        SegA2    = digit3 & 1;
        SegB2    = (digit3 >> 1) & 1;
        SegC2    = (digit3 >> 2) & 1;
        SegD2    = (digit3 >> 3) & 1;
        SegE2    = (digit3 >> 4) & 1;
        SegF2    = (digit3 >> 5) & 1;
        SegG2    = (digit3 >> 6) & 1;
    }
    else if (DispSel_2 == Left){
        SegA2    = digit2 & 1;
        SegB2    = (digit2 >> 1) & 1;
        SegC2    = (digit2 >> 2) & 1;
        SegD2    = (digit2 >> 3) & 1;
        SegE2    = (digit2 >> 4) & 1;
        SegF2    = (digit2 >> 5) & 1;
        SegG2    = (digit2 >> 6) & 1; 
    }
}

void generateTicketDisplay(int value){
    dig0 = value%10;
    int nextDig1 = value/10;
    dig1 = nextDig1%10;
    int nextDig2 = nextDig1/10;
    dig2 = nextDig2%10;
    int nextDig3 = nextDig2/10;
    dig3 = nextDig3%10;
}


unsigned char checkTicketInput(){
    if (buttonLock){
        if ((inputTicket == ticket) && ModeChange){
            buttonLock = ModeChange = 0;
            return 1;
        }
        ticketDig3 = ticketDig2;
        ticketDig2 = ticketDig1;
        ticketDig1 = ticketDig0;
        ticketDig0 = inputKey;
        inputTicket = (ticketDig3 * 1000) + (ticketDig2 * 100) + (ticketDig1 * 10) + (ticketDig0);
        ModeChange = 0;
        return 0;
    }
    ModeChange = 0;
    return 0;
}

unsigned char checkTicketInputMode4(){
    if (buttonLock){
        if ((isInArray(inputTicket,tickets)) && ModeChange){
            removeFromArray(inputTicket,tickets);
            buttonLock = ModeChange = 0;
            return 1;
        }
        ticketDig3 = ticketDig2;
        ticketDig2 = ticketDig1;
        ticketDig1 = ticketDig0;
        ticketDig0 = inputKey;
        dig0 = ticketDig0;
        dig1 = ticketDig1;
        dig2 = ticketDig2;
        dig3 = ticketDig3;
        inputTicket = (ticketDig3 * 1000) + (ticketDig2 * 100) + (ticketDig1 * 10) + (ticketDig0);
        ModeChange = 0;
        return 0;
    }
    ModeChange = 0;
    return 0;
}

void shiftValue(int newDig){
    if (ControllerMode == 1){
        dig1 = dig0;
        dig0 = newDig;
    }
}


/* Interrupt Service Routines   */
void __ISR(_CHANGE_NOTICE_VECTOR, IPL6) ChangeNotice_Handler(void){
    IEC1CLR	= 0x0001; //disable interrupt
    // Row 1 (R1) is the top row
    // Column 1 (C1) is the leftmost column
    R1=0;R2=R3=R4=1;    // check row 1
    for(i = 0; i < 1000; i++);
    if (C1 == 0){
        //select 1
        if(!buttonLock){
            buttonLock = 1;
            inputKey = 1;
            NumberKey = 1;
            if (ControllerMode == 3){
                checkTicketInput();
            }
            if (ControllerMode == 4){
                checkTicketInputMode4();
            }
        }
    }
    else if (C2 == 0){
        //select 2
        if(!buttonLock){
            buttonLock = 1;
            inputKey = 2;
            NumberKey = 1;
            if (ControllerMode == 3){
                checkTicketInput();
            }
           if (ControllerMode == 4){
                checkTicketInputMode4();
            }
        }
    }
    else if (C3 == 0){
        //select 3
        if(!buttonLock){
            buttonLock = 1;
            inputKey = 3;
            NumberKey = 1;
            if (ControllerMode == 3){
                checkTicketInput();
            }         
            if (ControllerMode == 4){
                checkTicketInputMode4();
            }
        }
    }
    else if (C4 == 0){
        //select A (invalid)
    }
    R2 = 0; R1 = R3 = R4 =1;    // check row 2
    for(i = 0; i < 1000; i++);
    if (C1 == 0){
        //select 4
        if(!buttonLock){
            buttonLock = 1;
            inputKey = 4;
            NumberKey = 1;
            if (ControllerMode == 3){
                checkTicketInput();
            }
            if (ControllerMode == 4){
                checkTicketInputMode4();
            }
        }
    }
    else if (C2 == 0){
        //select 5
        if(!buttonLock){
            buttonLock = 1;
            inputKey = 5;
            NumberKey = 1;
            if (ControllerMode == 3){
                checkTicketInput();
            }
            if (ControllerMode == 4){
                checkTicketInputMode4();
            }
        }
    }
    else if (C3 == 0){
        //select 6
        if(!buttonLock){
            buttonLock = 1;
            inputKey = 6;
            NumberKey = 1;
            if (ControllerMode == 3){
                checkTicketInput();
            }
            if (ControllerMode == 4){
                checkTicketInputMode4();
            }
        }
    }
    else if (C4 == 0){
        // select B (invalid)
    }
    R3 = 0; R1 = R2 = R4 = 1;   // check row 3
    for(i = 0; i < 1000; i++);
    if (C1 == 0){
        //select 7
        if(!buttonLock){
            buttonLock = 1;
            inputKey = 7;
            NumberKey = 1;
            if (ControllerMode == 3){
                checkTicketInput();
            }
            if (ControllerMode == 4){
                checkTicketInputMode4();
            }
        }
    }
    else if (C2 == 0){
        //select 8
        if(!buttonLock){
            buttonLock = 1;
            inputKey = 8;
            NumberKey = 1;
            if (ControllerMode == 3){
                checkTicketInput();
            }
            if (ControllerMode == 4){
                checkTicketInputMode4();
            }
        }
    }
    else if (C3 == 0){
        //select 9
        if(!buttonLock){
            buttonLock = 1;
            inputKey = 9;
            NumberKey = 1;
            if (ControllerMode == 3){
                checkTicketInput();
            }
            if (ControllerMode == 4){
                checkTicketInputMode4();
            }
        }
    }
    else if (C4 == 0){
        //select C
        if(!buttonLock){
            buttonLock = 1;
            ClearKey = 1;
        }
    }
    R4 = 0; R1 = R2 = R3 = 1;   // check row 4
    for(i = 0; i < 2000; i++);
    if (C1 == 0){
        //select 0
        if(!buttonLock){
            buttonLock = 1;
            inputKey = 0;
            NumberKey = 1;
            if (ControllerMode == 3){
                checkTicketInput();
            }
            if (ControllerMode == 4){
                checkTicketInputMode4();
            }
        }
    }
    else if (C2 == 0){
        //select F (invalid)
    }
    else if (C3 == 0){
        //select E
        if(!buttonLock){
            buttonLock = 1;
            ModeChange = 1; // change mode or enter ticket
            if (ControllerMode == 3){
                correctTicket = checkTicketInput();
                if (correctTicket == 0){
                    wrongPasscode = 1;
                }
            }
            if (ControllerMode == 4){
                correctTicket = checkTicketInputMode4();
                if (correctTicket == 0){
                    wrongPasscode = 1;
                }
            }
        }
    }
    else if (C4 == 0){
        //select D
        if(!buttonLock){
            buttonLock = 1;
            DeleteKey = 1;    // delete digit
        }
    }
    R1 = R2 = R3 = R4 = 0;  // reset rows
    for(i = 0; i < 1000; i++);
    dummyRead = PORTB;
    IFS1CLR  = 0x0001;  // Clear the interrupt flag
    IEC1SET	= 0x0001;   // Re-enable interrupts
}


void __ISR(_TIMER_5_VECTOR,ipl7) _TIMER5_HANDLER(void){
    if (mode == Left){
        mode = Right;
    }
    else{
        mode = Left;
    }
    displayDigit(mode,number[dig3],number[dig2],number[dig1],number[dig0]);
    if (ticketTrigger){
        secondCount++;
        if(secondCount > 99){   // put 1 second operations in this conditional
            secondCount = 0;
            PLedCounter--;
        }
    }
    if (ControllerMode == 5){
        secondCount++;
        if (secondCount > 99){
            secondCount = 0;
            pledOn = !pledOn;
            mode5Counter++;
        }
    }
    IFS0CLR = 0x00100000;  // Clear the Timer5 interrupt status flag 
}

/* Analog to digital conversion     */
int readADC(){
    AD1CON1bits.SAMP = 1; //Start sampling
    while(!AD1CON1bits.DONE); //wait while sampling takes place
    return ADC1BUF0;
}


int main(){
    
    INTDisableInterrupts();
    AD1PCFG = 0xF7FF;   // set pins to digital except RB11
    AD1CON1 = 0x00E0;   //automatic conversion afer sampling
    AD1CHS = 0x000B0000; //connect RB11 to mux
    AD1CSSL = 0;        //no scanning
    AD1CON2 = 0;        //Configure voltages
    AD1CON3 = 0x1F3F;   //Sampling rate
    AD1CON1bits.ADON = 1; //Turn on sampling
    
    //initialize TRISx
    TRISA = 0x4000; // set PORTA LS11 (light sensor input)
    TRISB = 0x080F; // set PORTB bits 0,1,2,3 as input (keypad) and bits 10,11,12,13 as input (MIC)
    TRISC = 0;      // set PORTC as output
    TRISD = 0;      // set PORTD as output
    TRISE = 0;      // set PORTE as output
    TRISF = 0;      // Outputs: RF4,5,12,13
    TRISG = 0;      // set PORTG as output
    
    //initialize PORTx
    PORTA = 0;
    PORTB = 0;
    PORTC = 0;
    PORTD = 0;
    PORTE = 0;
    PORTF = 0;
    PORTG = 0;
    
    //PBCLK = 80MHz
    T4CON = 0x0;           // Stop 16-bit Timer4 and clear control register
    T5CON = 0x0;           // Stop 16-bit Timer5 and clear control register
    T4CONSET = 0x0068;     // Enable 32-bit mode, prescaler at 1:64,    
    TMR4 = 0x0;            // Clear contents of the TMR4 and TMR5
    PR4 = 0x000030D3;      // Load PR4 and PR5 registers with 32-bit value, 1E847 for 10hz, 1312CF for 1Hz
    // 30D3 for 100Hz
    IPC5SET = 0x0000001C;  // Set priority level=7 and
    IPC5SET = 0x00000001;  // Set sub-priority level=1

    IFS0CLR = 0x00100000;  // Clear the Timer5 interrupt status flag 
    IEC0SET = 0x00100000;  // Enable Timer5 interrupts
    T4CONSET = 0x8000;     // Start Timer 
    
    // Initialize CNCON, CNEN, CNPUE
    CNCON = 0x08000;    // enable CN module
    CNEN = 0x003C;      // CN2-5
    CNPUE = 0x003C;     // CN2-5
    // Initialize CN interrupts
    IPC6SET = 0x00180000; 	// Set priority level=6
    IFS1CLR = 0x0001; 		// Clear the interrupt flag status bit
    IEC1SET	= 0x0001; 		// Enable Change Notice interrupts
    
    dummyRead = PORTB;
    
    //Enable multi-vector interrupt (CN interrupts are multi-vectored)
    INTEnableSystemMultiVectoredInt();
    INTEnableInterrupts();
    R1 = R2 = R3 = R4 = 0;

    // ControllerMode1 settings C__0
    PLed1 = PLed2 = PLed3 = PLed4 = PLed5 = PLed6 = PLed7 = PLed8 = 0;
    dig0 = 0;
    dig1 = 16;  // clear
    dig2 = 16;  // clear
    dig3 = 20;   // C
    
    int analogRead;
    while(1){
        if(ControllerMode == 1){        // if the controller is in Mode 1 (capacity set
            if (buttonLock){            // if any valid key is pressed
                if (DeleteKey){         // if delete was pressed
                    dig0 = dig1;
                    dig1 = 0;
                }
                else if (ClearKey){     // if clear was pressed
                    dig0 = dig1 = 0;
                    capacity = 0;
                }
                else if (NumberKey){    // if a number was entered
                    shiftValue(inputKey);
                }
                else if(ModeChange){    // E was pressed (ModeChange == 1)
                    ControllerMode = 2;
                }
                if (dig1 == 0){         // eliminate unneccessary 0s
                    dig1 = 16;
                }
                if (dig0 == 16){        // ensure at least 0 is displayed in the first digit
                    dig0 = 0;
                }
                DeleteKey = ClearKey = NumberKey = ModeChange = 0;
                arrayGenerate = 1;      // allow new capacity to be set everytime you're in mode 1
            }
        }
        else if(ControllerMode == 2){
            dig3 = 15;  // F
            dig2 = 16;  // clear
            PLed1 = PLed2 = PLed3 = PLed4 = PLed5 = PLed6 = PLed7 = PLed8 = 0;
            if (dig1 < 16){     // make sure dig1 is not clear on SSD
                capacity = (dig1 * 10) + dig0;
            }
            else{
                capacity = dig0;
            }
            //makes sure that array is not overwritten everytime mode 2 happens
            //dig1 and dig0 have to be the number of spaces open
            if (mode3Hit || mode4Hit || mode5Hit){
                correctTicket = 0;
                openSpaces = freeSpaces(tickets);
                dig0 = openSpaces%10;
                dig1 = (openSpaces/10)%10;
                if (dig1 == 0){
                    dig1 = 16;
                }
                mode3Hit = mode4Hit = mode5Hit = 0;
            }
            if (arrayGenerate == 1){
                ticketArrayLength = capacity;  // set the length the array of tickets equal to the capacity
                // need to generate the handle tickets now
                initializeArray(tickets);
                arrayGenerate = 0;
            }
        }
        else if(ControllerMode == 3){
            /* Begin countdown code */
            mode3Hit = 1;
            if (activateCountDown){
                PLed1 = PLed2 = PLed3 = PLed4 = PLed5 = PLed6 = PLed7 = PLed8 = 1;
            }
            if (ticketTrigger){
                seed++;
                if (wrongPasscode){
                    correctTicket = 0;
                    error = 2;
                    ControllerMode = 5;
                    wrongPasscode = 0;
                }
                if(correctTicket){
                    addToArray(ticket,tickets);
                    ControllerMode = 2;
                    correctTicket = 0;
                }
                if (ClearKey){
                    ticketDig0 = 0;
                    ticketDig1 = ticketDig2 = ticketDig3 = 16;
                    ClearKey = 0;
                }
                if (DeleteKey){
                    ticketDig0 = ticketDig1;
                    ticketDig1 = ticketDig2;
                    ticketDig2 = ticketDig3;
                    ticketDig3 = 0;
                    if (ticketDig0 == 16){
                        ticketDig0 = 0;
                    }
                    DeleteKey = 0;
                }
                if (PLedCounter > 7){   // if counter is still counting down
                     PLed1 = PLed2 = PLed3 = PLed4 = PLed5 = PLed6 = PLed7 = PLed8 = 1;
                }
                else if (PLedCounter == 7){
                    PLed1 = PLed2 = PLed3 = PLed4 = PLed5 = PLed6 = PLed7 = 1;
                    PLed8 = 0;
                }
                else if (PLedCounter == 6){
                    PLed1 = PLed2 = PLed3 = PLed4 = PLed5 = PLed6 = 1;
                    PLed8 = PLed7 = 0;
                }
                else if (PLedCounter == 5){
                    PLed1 = PLed2 = PLed3 = PLed4 = PLed5 = 1;
                    PLed8 = PLed7 = PLed6 = 0;
                }
                else if (PLedCounter == 4){
                    PLed1 = PLed2 = PLed3 = PLed4 = 1;
                    PLed5 = PLed6 = PLed7 = PLed8 = 0;
                }
                else if (PLedCounter == 3){
                    PLed1 = PLed2 = PLed3 = 1;
                    PLed4 = PLed5 = PLed6 = PLed7 = PLed8 = 0;
                }
                else if (PLedCounter == 2){
                    PLed1 = PLed2 = 1;
                    PLed3 = PLed4 = PLed5 = PLed6 = PLed7 = PLed8 = 0;
                }
                else if (PLedCounter == 1){
                    PLed1 = 1;
                    PLed2 = PLed3 = PLed4 = PLed5 = PLed6 = PLed7 = PLed8 = 0;
                }
                else{
                    PLed1 = PLed2 = PLed3 = PLed4 = PLed5 = PLed6 = PLed7 = PLed8 = 0;  // Timeout error
                    ticketTrigger = 0;
                    error = 1;
                    ControllerMode = 5;
                }
            }
            /* End Countdown Code */
            
            /* Begin logic */
            if (generateTicket){
                unsigned int validTicket = 0;
                while (!validTicket){
                    srand(seed);      // seed random number generator
                    ticket = rand() % 10000;
                    //if (ticket < 1000){
                    //    ticket += 1000;
                    //}
                    if (!isInArray(ticket,tickets)){
                        //addToArray(ticket,tickets);
                        validTicket = 1;
                    }
                    else{
                        seed++;
                    }
                    seed++;
                }
                generateTicket = 0;
                generateTicketDisplay(ticket);
            }
            
            /* End state logic */
        }
        else if(ControllerMode == 4){
            if (!mode4Hit){
                mode4Hit = 1;
                dig1 = dig2 = dig3 = 16;
                dig0 = 0;
                ticketDig1 = ticketDig2 = ticketDig3 = 16;
                ticketDig0 = 16;
            }
            if (activateCountDown){
                PLed1 = PLed2 = PLed3 = PLed4 = PLed5 = PLed6 = PLed7 = PLed8 = 1;
                activateCountDown = 0;
            }
            if(ticketTrigger){
                seed++;
                if (dig3 == 0 && ticketDig3 != 0){
                    dig3 = 16;
                    if (dig2 == 0 && ticketDig2 != 0){
                        dig2 = 16;
                        if (dig1 ==  0 && ticketDig1 != 0){
                            dig1 = 16;
                        }
                    }
                }
                else if (dig3 == 16 && dig2 == 0 && ticketDig3 != 0 && ticketDig2 != 0){
                    dig2 = 16;
                    if (dig1 == 0 && ticketDig1 != 0){
                        dig1 = 16;
                    }
                }
                else if (dig3 == 16 && dig2 == 16 && dig1 == 0 && ticketDig3 != 0 && ticketDig2 != 0 && ticketDig1 != 0){
                    dig1 = 16;
                }
                if(wrongPasscode){
                    error = 2;
                    ControllerMode = 5;
                    wrongPasscode = 0;
                }
                if (correctTicket){
                    ControllerMode = 2;
                    correctTicket = 0;
                }
                if (ClearKey){
                    dig0 = ticketDig0 = 0;
                    dig1 = ticketDig1 = dig2 = ticketDig2 = dig3 = ticketDig3 = 16;
                    ClearKey = 0;
                }
                if (DeleteKey){
                    dig0 = dig1;
                    dig1 = dig2;
                    dig2 = dig3;
                    dig3 = 0;
                    ticketDig0 = ticketDig1;
                    ticketDig1 = ticketDig2;
                    ticketDig2 = ticketDig3;
                    ticketDig3 = 0;
                    if (ticketDig0 == 16){
                        ticketDig0 = 0;
                    }
                    if (dig0 == 16){
                        dig0 = 0;
                    }
                    DeleteKey = 0;
                }
                if (PLedCounter > 7){   // if counter is still counting down
                     PLed1 = PLed2 = PLed3 = PLed4 = PLed5 = PLed6 = PLed7 = PLed8 = 1;
                }
                else if (PLedCounter == 7){
                    PLed1 = PLed2 = PLed3 = PLed4 = PLed5 = PLed6 = PLed7 = 1;
                    PLed8 = 0;
                }
                else if (PLedCounter == 6){
                    PLed1 = PLed2 = PLed3 = PLed4 = PLed5 = PLed6 = 1;
                    PLed8 = PLed7 = 0;
                }
                else if (PLedCounter == 5){
                    PLed1 = PLed2 = PLed3 = PLed4 = PLed5 = 1;
                    PLed8 = PLed7 = PLed6 = 0;
                }
                else if (PLedCounter == 4){
                    PLed1 = PLed2 = PLed3 = PLed4 = 1;
                    PLed5 = PLed6 = PLed7 = PLed8 = 0;
                }
                else if (PLedCounter == 3){
                    PLed1 = PLed2 = PLed3 = 1;
                    PLed4 = PLed5 = PLed6 = PLed7 = PLed8 = 0;
                }
                else if (PLedCounter == 2){
                    PLed1 = PLed2 = 1;
                    PLed3 = PLed4 = PLed5 = PLed6 = PLed7 = PLed8 = 0;
                }
                else if (PLedCounter == 1){
                    PLed1 = 1;
                    PLed2 = PLed3 = PLed4 = PLed5 = PLed6 = PLed7 = PLed8 = 0;
                }
                else{
                    PLed1 = PLed2 = PLed3 = PLed4 = PLed5 = PLed6 = PLed7 = PLed8 = 0;  // Timeout error
                    activateCountDown = 0;
                    error = 1;
                    ControllerMode = 5;
                }
            }
        }
        else{   // ControllerMode == 5
            if(!mode5Hit){
                mode5Hit = 1;
                pledOn = 1;
            }
            ticketTrigger = 0;
            dig3 = 14;
            if (error == 1){
                dig2 = 1;
                dig1 = 1;
                dig0 = 1;
            }
            else if (error == 2){
                dig2 = 2;
                dig1 = 2;
                dig0 = 2;
            }
            else if (error == 3){
                dig2 = 3;
                dig1 = 3;
                dig0 = 3;
            }
            else if (error == 4){
                dig2 = 4;
                dig1 = 4;
                dig0 = 4;
            }
            if(pledOn){
                PLed1 = PLed2 = PLed3 = PLed4 = PLed5 = PLed6 = PLed7 = PLed8 = 1;
            }
            else{
                PLed1 = PLed2 = PLed3 = PLed4 = PLed5 = PLed6 = PLed7 = PLed8 = 0;
            }
            if (mode5Counter > 5){
                mode5Counter = 0;
                ControllerMode = 2; 
            }
        }
        buttonLock = activateCountDown = 0;
        /*-------  Non-ISR Next-State Logic  ---------*/
      
        analogRead = readADC();
        if (analogRead > 400 && ControllerMode == 2){
            if (isFull(tickets)){
                error = 3;
                ControllerMode = 5;
            }
            else{
                ControllerMode = 3; //CarIn
                activateCountDown = 1;
                ticketTrigger = 1;
                PLedCounter = 8;
                generateTicket = 1;
            }
        }
        if (LS11 && ControllerMode == 2){
            if (freeSpaces(tickets) == ticketArrayLength){
                //throw false leaving error
                error = 4;
                ControllerMode = 5;
            }
            else{
                ControllerMode = 4;     // CarOut
                activateCountDown = 1;
                ticketTrigger = 1;
                PLedCounter = 8;
            }
        }
    }
    return 0;
}
