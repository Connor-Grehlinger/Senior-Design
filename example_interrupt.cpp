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
 
// Definition of interrupt names
#include <avr/io.h >
// ISR interrupt service routine
#include <avr/interrupt.h >

#define INPUT_PIN 4         // Will be used to output collected pulse times
#define EX_INT_PIN 2        // INTO uses pin 2, external interrupt
                            // This is port D2
    
#define LED_PIN 13          // PortB 5


volatile unsigned long periodLength;
volatile unsigned long periodLengths[100];
volatile unsigned long risingEdgeTime;
volatile int fallingEdgeTime;

unsigned int index = 0;


void calcPulseTime()
{
    if (PIND & (1 << PD2))         // Rising edge (PORTD pin 2 is HIGH)
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

void setup() {
    
    Serial.begin(115200);
    Serial.println("Initializing interrupt service routine");
    
    pinMode(EX_INT_PIN, INPUT);                         // set external interrupt as input
    pinMode(INPUT_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    
    Serial.println("Processing initialization");
    
    
    PORTD &= B11101111;                                 // set LED to low
    
    attachInterrupt(digitalPinToInterrupt(EX_INT_PIN), calcPulseTime, CHANGE);
    
    Serial.println("Finished initialization");
}


void loop() {
  
    if(PIND & (1 << PD4))
    {
        PORTB |= B00100000;         // LED to HIGH
        
        delay(10000);
        
        for (int i = 0; i < 50; i++)
        {
            Serial.println(periodLengths[i]);
        }
        
        delay(100000);
    }

}







