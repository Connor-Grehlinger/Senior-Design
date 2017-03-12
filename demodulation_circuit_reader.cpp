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

#define INPUT_PIN 4         // Will be used to output collected pulse times
#define EX_INT_PIN 2        // INTO uses pin 2, external interrupt
                            // This is port D2
    
#define LED_PIN 13


volatile int pulseTime;
volatile int pulseTimes[100];
volatile int risingEdgeTime;
volatile int fallingEdgeTime;
volatile int state = 0;

unsigned int index = 0;


void calcPulseTime()
{
    state = digitalRead(LED_PIN);
    if (state == HIGH)         // HIGH
    {
        digitalWrite(LED_PIN, LOW);
        //PORTD &= B11101111;  
    }
    else
    {
        digitalWrite(LED_PIN, HIGH);
        //PORTD |= B00010000;  
    }
}

void setup() {
    
    Serial.begin(115200);
    Serial.println("Initializing interrupt service routine");
    
    pinMode(EX_INT_PIN, INPUT);                         // set external interrupt as input
    pinMode(INPUT_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    
    Serial.println("Processing initialization");
    
    
    PORTD &= B11101111;                                 // set LED to low
    
    attachInterrupt(digitalPinToInterrupt(EX_INT_PIN), calcPulseTime, RISING);
    
    Serial.println("Finished initialization");
}


void loop() {
    

}







