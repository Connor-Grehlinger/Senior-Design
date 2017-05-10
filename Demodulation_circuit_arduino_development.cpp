/*  Team Breach Prototype Testing 
  Connor Grehlinger, Keith Doggett, Mathew Schlogel, John Oribioye, Matt Leung
  --------------------------------------------------------------
  #define PWM_SOURCE 34
  pwmin = pulseIn(PWM_SOURCE, HIGH, 20000);

  This waits for the digital pin to go from LOW to HIGH to LOW, 
  Very slow 
  --------------------------------------------------------------
  Use interrupts for better result 

  Trying with digital pin 4 first (PORTD = B00010000)
  PORTD = B10101000; // sets digital pins 7,5,3 HIGH
  
  
  DDRD &= ~(1<<PD2);    //Configure PORTD pin 2 as an input
  PORTD |= (1<<PD2);    //Activate pull-ups in PORTD pin 2
  
*/
 
// Include IO and interrupt libraries
#include <avr/io.h >
// ISR interrupt service routine
#include <avr/interrupt.h >

#define INPUT_PIN 4         // Will be used to output collected pulse times
#define EX_INT_PIN 2        // INTO uses pin 2, external interrupt
                            // This is port D2
    
#define LED_PIN 13


volatile unsigned long periodLength;
volatile unsigned long periodLengths[100];
volatile unsigned long risingEdgeTime;

volatile unsigned long firstRisingEdgeTime;
volatile unsigned long secondRisingEdgeTime;
volatile int state = 0;

unsigned int index = 0;

// ISR for external interrupt
/*
ISR(INT0_vect){
    if (PIND & (1 << PD2))         // Rising edge
    {
        risingEdgeTime = micros();
        PORTB |= B00100000;         // LED to HIGH
    }
    else
    {
        periodLength = ((volatile unsigned long)micros() - risingEdgeTime);
        
        PORTB &= B11011111;         // LED to LOW
        
        if (index < 100)
        {
            periodLengths[index] = periodLength;
            index++;
        }
    }
}
*/


void calcPulseTime1()
{
    if(!state)
    {
        firstRisingEdgeTime = micros();
        state ^= 1;
        PORTB |= B00100000;         // LED to HIGH
    }
    else
    {
        secondRisingEdgeTime = micros();
        periodLength = (secondRisingEdgeTime - firstRisingEdgeTime);
        firstRisingEdgeTime = secondRisingEdgeTime;
        
        PORTB &= B11011111;         // LED to LOW
        
        if (index < 100)
        {
            periodLengths[index] = periodLength;
            index++;
        }
        state ^= 1;
    }
}

void setup() {
    
    Serial.begin(115200);
    Serial.println("Initializing interrupt service routine");
    
    pinMode(EX_INT_PIN, INPUT);                         // set external interrupt as input
    pinMode(INPUT_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    
    Serial.println("Processing initialization");
    
    /*
    GICR |= (1 << INTO);                                // Enable INTO interrupt on general interrupt control register
    
    MCUCR |= (1 << ISC00);                              // MCU control register
    MCUCR |= (0 << ISC01);                              // 1 << ISC00 and 1 << ISC01 = rising edge, 0 << ISC00 and 1 << ISC01 = falling edge, 
                                                        // 1 << ISC00 and 0 << ISC01 = both rising and falling edges 
    
    */
                                                        
    PORTD &= B11101111;                                 // Set LED to LOW
    
    //attachInterrupt(digitalPinToInterrupt(EX_INT_PIN), calcPulseTime, CHANGE);
    attachInterrupt(digitalPinToInterrupt(EX_INT_PIN), calcPulseTime1, RISING);
    
    Serial.println("Finished initialization");
}


void loop() {
  
    if(PIND & (1 << PD4))
    {
        PORTB |= B00100000;         // LED to HIGH
        
        delay(10000);
        
        for (int i = 0; i < 25; i++)
        {
            Serial.println(periodLengths[i]);
        }
    }

}



