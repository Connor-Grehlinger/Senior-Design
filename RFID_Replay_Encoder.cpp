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


// -------- Output signal development --------
const int ocr1aPin = 9;

// Set the frequency that we will get on pin OCR1A
void setFrequency(uint32_t freq)
{
  uint32_t requiredDivisor = (F_CPU/2)/(uint32_t)freq;
  uint16_t prescalerVal;
  uint8_t prescalerBits;
  if (requiredDivisor < 65536UL)
  {
    prescalerVal = 1;
    prescalerBits = 1;
  }
  else if (requiredDivisor < 8 * 65536UL)
  {
    prescalerVal = 8;
    prescalerBits = 2;
  }
  else if (requiredDivisor < 64 * 65536UL)
  {
    prescalerVal = 64;
    prescalerBits = 3;
  }
  else if (requiredDivisor < 256 * 65536UL)
  {
    prescalerVal = 256;
    prescalerBits = 4;
  }
  else
  {
    prescalerVal = 1024;
    prescalerBits = 5;
  }

  uint16_t top = ((requiredDivisor + (prescalerVal/2))/prescalerVal) - 1;
  TCCR1A = 0;
  TCCR1B = (1 << WGM12) | prescalerBits;
  TCCR1C = 0;
  OCR1A = (top & 0xFF);
}

// Turn the frequency on
void on()
{
  TCNT1H = 0;
  TCNT1L = 0;  
  TCCR1A |= (1 << COM1A0);
}

// Turn the frequency off and turn of the IR LED
void off()
{
  TCCR1A &= ~(1 << COM1A0);
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

    
    /*
    timingValues = SD.open("test.txt", FILE_READ);
    
    if (timingValues) {
      Serial.println("File opened correcrly: test.txt");
      timingValues.close();
    } else {
      Serial.println("Error opening test.txt");
    }
    */
    
    Serial.println("Trigger pin 4 for replay data bitset...");
    
    pinMode(EX_INT_PIN, INPUT);                         // set external interrupt as input
    pinMode(INPUT_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    
    /*
    GICR |= (1 << INTO);                                // Enable INTO interrupt on general interrupt control register
    
    MCUCR |= (1 << ISC00);                              // MCU control register
    MCUCR |= (0 << ISC01);                              // 1 << ISC00 and 1 << ISC01 = rising edge, 0 << ISC00 and 1 << ISC01 = falling edge, 
                                                        // 1 << ISC00 and 0 << ISC01 = both rising and falling edges  
    */

    digitalWrite(ocr1aPin, LOW);
    pinMode(ocr1aPin, OUTPUT);
    
    PORTD &= B11101111;                                 // Set LED to LOW

    setFrequency(12500);
    off();
   
}


void loop() {
  
    if(PIND & (1 << PD4) && !lock)
    {
        lock = 1;
        delay(2000);
        PORTB |= B00100000;         // LED to HIGH
        
        // Re-open for reading 
        timingValues = SD.open("test.txt", FILE_READ);
        
        if (timingValues) {
          Serial.println("Re-opening for read, test.txt file contents:");

          // read from the file until there's nothing else in it:
          int i = 0;
          while (timingValues.available()) {
              
              //periodLengths[i] = (volatile unsigned int)timingValues.read();
              periodLengths[i] = (volatile unsigned int)timingValues.println();
              //Serial.write(timingValues.read());
              i++;
          }
          timingValues.close();
        } else {
          Serial.println("Error opening test.txt");
        }
        delay(3000);
        Serial.println("Now displaying values from newly populated on-board array:");
        for (int i = 0; i < 100; i++)
        {
          Serial.println(periodLengths[i]);
          if (periodLengths[i] == 0)
          {
            setFrequency(12500);
          }
          else if(periodLengths[i] == 1)
          {
            setFrequency(15600);
          }
          else 
          {
            //errror has occurred 
          }
        }
        lock = 0;
    }
}
