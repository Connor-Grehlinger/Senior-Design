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


volatile int pulseTime;
volatile int pulseTimes[100];
volatile int risingEdgeTime;
volatile int fallingEdgeTime;
volatile int state = 0;

unsigned int index = 0;


void calcPulseTime()
{
    //state = (PIND & (1 << PD2));
    if (PIND & (1 << PD2))         // Rising edge
    {
        //risingEdgeTime = micros();
        risingEdgeTime = millis();
        PORTB |= B00100000;         // LED to HIGH
    }
    else
    {
        //fallingEdgeTime = micros();
        fallingEdgeTime = millis();
        pulseTime = fallingEdgeTime - risingEdgeTime;
        
        PORTB &= B11011111;         // LED to LOW
        
        if (index < 100)
        {
            pulseTimes[index] = pulseTime;
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
  
    if(/*PORTD == B00010000 button press*/digitalRead(INPUT_PIN))
    {
        digitalWrite(LED_PIN, HIGH);
        
        for (int i = 0; i < 10; i++)
        {
            Serial.println(pulseTimes[i]);
        }
        delay(100000);
    }

}







