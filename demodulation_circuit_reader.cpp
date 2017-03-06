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
 */
 
// Definition of interrupt names
#include <avr/io.h >
// ISR interrupt service routine
#include <avr/interrupt.h >

#define INPUT_PIN 4

volatile unsigned long pulseRiseTime;
volatile int lastInterruptTime;
volatile int pulseTime;
volatile int pulseValues[100];
unsigned int index = 0;
int i = 0;


// calcPulseTime is the interrupt handler (ISR)
void calcPulseTime()
{
    lastInterruptTime = micros();       // returns the time passed since the program has started running 

    // check if the signal pin has gone high 
    if (PORTB == B00010000)             // digitalRead(INPUTPIN) == HIGH
    {
        pulseRiseTime = micros();       // capture rise time
    }
    else                                // it's low
    {
        if (pulseRiseTime != 0)         // if the timer has started 
        {
            pulseTime = ((volatile int)micros() - pulseRiseTime);
            pulseRiseTime = 0;
        }
    }
}


void setup() {

    
    pinMode(INPUT_PIN, INPUT);                           // set pin mode
    
    pulseRiseTime = 0; 
    attachInterrupt(INPUT_PIN, calcPulseTime, CHANGE);   // attach interrupt 
    
    
    Serial.begin(115200);
    Serial.println("Finished initialization");
}

void loop() {

    /* Will need to store pulse times
    
    pulseValues[index] = pulseRiseTime;
    index++;
    
    */
                    
    Serial.println(pulseRiseTime);            // Printing out to serial monitor for now
    //delay(10);                              // Will need to store data values for replay
}




