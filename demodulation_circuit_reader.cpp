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

const int INPUT_PIN 4         // Will be used to output collected pulse times
const int EX_INT_PIN 3        // INTO uses pin 2, external interrupt
                            // This is port D2
    
                            
const int LED_PIN 13



volatile int pulseTime;
volatile int pulseTimes[100];
volatile int risingEdgeTime;
volatile int fallingEdgeTime;

unsigned int index = 0;


void calcPulseTime()
{
    
    if (digitalRead(LED_PIN))         // HIGH
    {
        digitalWrite(LED_PIN, LOW);
    }
    else
    {
        digitalWrite(LED_PIN, HIGH);
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
    //GICR |= (1 << INTO);                                // Enable INTO interrupt on general interrupt control register
    sei();
    MCUCR |= (1 << ISC00);                              // MCU control register
    MCUCR |= (0 << ISC01);                              // 1 << ISC00 and 1 << ISC01 = rising edge, 0 << ISC00 and 1 << ISC01 = falling edge, 
                                                        // 1 << ISC00 and 0 << ISC01 = both rising and falling edges 
    */
    
    PORTD &= B11101111;                                 // set LED to low
    
    attachInterrupt(EX_INT_PIN, calcPulseTime, CHANGE);
    //sei();
    
    
    Serial.println("Finished initialization");
}


void loop() {
    
    /*
    if(/*PORTD == B00010000 button press digitalRead(INPUT_PIN))
    {
        for (int i = 0; i < 10; i++)
        {
            Serial.println(pulseTimes[i]);
        }
    }
    PORTD &= B11101111;
    */
}