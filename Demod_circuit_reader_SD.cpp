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
// Include SD card libraries 
#include <SPI.h>
#include <SD.h>

File timingValues;          // SD card file for data storage

#define INPUT_PIN 4         // Will be used to output collected pulse times
#define EX_INT_PIN 2        // INTO uses pin 2, external interrupt
                            // This is port D2
    
#define LED_PIN 13

// changed from long to int
volatile unsigned int periodLength;
volatile unsigned int periodLengths[100];

volatile unsigned int firstRisingEdgeTime;
volatile unsigned int secondRisingEdgeTime;
volatile char state = 0;

unsigned int index = 0;
char lock = 0;


// ISR function
void calcPulseTime()
{
    if(!state)
    {
        firstRisingEdgeTime = micros();
        state ^= 1;
    }
    else
    {
        secondRisingEdgeTime = micros();
        periodLength = (secondRisingEdgeTime - firstRisingEdgeTime);
        firstRisingEdgeTime = secondRisingEdgeTime;
        
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
    
    while(!Serial);

    Serial.print("Initializing SD card...");

    if (!SD.begin(4)) {
      Serial.println("initialization failed!");
      return;
     }
    Serial.println("Initialization done.");

    // Clear contents before writing to same file again
    SD.remove("test.txt");
    
    timingValues = SD.open("test.txt", FILE_WRITE);
    
    if (timingValues) {
      Serial.println("Writing period lengths to test.txt file.");
      timingValues.close();
    } else {
      Serial.println("Error opening test.txt");
    }

    Serial.println("Initializing interrupt service routine.");
    
    pinMode(EX_INT_PIN, INPUT);                         // set external interrupt as input
    pinMode(INPUT_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    
    /*
    GICR |= (1 << INTO);                                // Enable INTO interrupt on general interrupt control register
    
    MCUCR |= (1 << ISC00);                              // MCU control register
    MCUCR |= (0 << ISC01);                              // 1 << ISC00 and 1 << ISC01 = rising edge, 0 << ISC00 and 1 << ISC01 = falling edge, 
                                                        // 1 << ISC00 and 0 << ISC01 = both rising and falling edges  
    */
                                                        
    PORTD &= B11101111;                                 // Set LED to LOW
    
    attachInterrupt(digitalPinToInterrupt(EX_INT_PIN), calcPulseTime, RISING);
    
    Serial.println("Finished interrupt service routine initialization...");
}


void loop() {
  
    if(PIND & (1 << PD4) && !lock)
    {
        lock = 1;
        PORTB |= B00100000;         // LED to HIGH
        delay(5000);

        timingValues = SD.open("test.txt", FILE_WRITE);
        if (timingValues) {
          Serial.println("Writing to test.txt...");
            for (int i = 0; i < 100; i++)
            {
              if (periodLengths[i] <= 72 && periodLengths[i] > 55)
              {
                periodLengths[i] = 1;
              }
              else if (periodLengths[i] > 72 && periodLengths[i] < 90)
              {
                periodLengths[i] = 0;
              }
              else
              {
                // indicate an error
                periodLengths[i] = 2;
              }
              timingValues.println(periodLengths[i]);
              
            }
            // write array to SD card
            //timingValues.write(periodLengths);
            
            // close the file:
            timingValues.close();
            Serial.println("Done write process.");
            
        } else {
          // if the file didn't open, print an error:
          Serial.println("Error opening test.txt during write");
        }
        delay(5000);

        // --------WRITE COMPLETE--------

        
        // Re-open for reading 
        timingValues = SD.open("test.txt");
        
        if (timingValues) {
          Serial.println("Re-opening for read, test.txt file contents:");

          // read from the file until there's nothing else in it:
          while (timingValues.available()) {
              Serial.write(timingValues.read());
              // this function returns the next byte/charactrer 
          }
          timingValues.close();
        } else {
          Serial.println("Error opening test.txt");
        }
        lock = 0;
    }
}
