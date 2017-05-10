/*  Team Breach Prototype Testing 
  Connor Grehlinger, Keith Doggett, Mathew Schlogel, John Oribioye, Matt Leung
  --------------------------------------------------------------
  --------------------------------------------------------------
*/
 
// Include IO and interrupt libraries
#include <avr/io.h>
// ISR interrupt service routine
#include <avr/interrupt.h >
// Include SD card libraries 
#include <SPI.h>
#include <SD.h>

File timingValues;          // SD card file for data storage

#define INPUT_PIN 4         // Will be used to trigger output of captured period lengths
#define EX_INT_PIN 2        // Input pin for demodulation circuit signal
                            // INTO uses pin 2, external interrupt, (port D2)

    
#define LED_PIN 13          // On-board LED 

// Changed from long to int, saves on-board memory
volatile unsigned int periodLength;
volatile unsigned int periodLengths[144];

volatile unsigned int firstRisingEdgeTime;
volatile unsigned int secondRisingEdgeTime;
volatile char state = 0;

unsigned int index = 0;
char lock = 0;


// Interrupt service routine for demodulation circuit input 
void calcPulseTime()
{
    if(!state)
    {
        firstRisingEdgeTime = micros();     // capture first timestamp
        state ^= 1;
    }
    else
    {
        secondRisingEdgeTime = micros();    // capture second rising 

        // Determine difference and get ready for next timing capture
        periodLength = (secondRisingEdgeTime - firstRisingEdgeTime);
        firstRisingEdgeTime = secondRisingEdgeTime;
        
        if (index < 144)        // store period length in an array for writing later
        {
            periodLengths[index] = periodLength;
            index++;
        }
        state ^= 1;
    }
}


void setup() {
  
    Serial.begin(115200);   // Set data rate

    // Initialize SD card on Arduino shield 
    while(!Serial);         
    Serial.print("Initializing SD card...");
    if (!SD.begin(4)) {
      Serial.println("Initialization failed!");
      return;
     }
    Serial.println("Initialization done.");

    // Clear contents before writing to same file again
    SD.remove("test.txt");
    
    timingValues = SD.open("test.txt", FILE_WRITE);   // Open test.txt for writing 
    
    if (timingValues) {     // If the file opened properly 
      
      Serial.println("Writing Demodulation circuit output to test.txt file.");
      timingValues.close();
    } else {
      
      Serial.println("Error opening SD: test.txt file.");
    }

    Serial.println("Initializing interrupt service routine.");
    
    pinMode(EX_INT_PIN, INPUT);     // Set external interrupt pin as input
    pinMode(INPUT_PIN, INPUT);      // Set out trigger pin as input
    pinMode(LED_PIN, OUTPUT);       // Set on-board LED as output
    
    /*

    //Configuration option using AVR control register names
    GICR |= (1 << INTO);    // Enable INTO interrupt on general interrupt control register
    MCUCR |= (1 << ISC00);  // MCU control register
    MCUCR |= (0 << ISC01);  // 1 << ISC00 and 1 << ISC01 = rising edge, 0 << ISC00 and 1 << ISC01 = falling edge, 
                            // 1 << ISC00 and 0 << ISC01 = both rising and falling edges  
    */
                                                        
    PORTD &= B11101111;     // Set LED to LOW

    // Attach the ISR to the demodulation circuit input pin 
    attachInterrupt(digitalPinToInterrupt(EX_INT_PIN), calcPulseTime, RISING);
    
    Serial.println("Finished interrupt service routine initialization. Running...");
}


void loop() {
  
    if(PIND & (1 << PD4) && !lock)  // If output trigger pin goes to high 
    {
        lock = 1;
        PORTB |= B00100000;   // LED to HIGH to indicate output triggered
        delay(2000);          // Wait 2 seconds 

        timingValues = SD.open("test.txt", FILE_WRITE);   // re-open for writing 
        if (timingValues) {
          Serial.println("Writing to test.txt...");
            for (int i = 0; i < 144; i++)
            {
              
              // Decision function to decode timing values as 0's or 1's
              // 0's have period of about 64 us; 1's are about 80us (some small variation)
              if (periodLengths[i] < 72 && periodLengths[i] > 40)
              {
                periodLengths[i] = 0;
              }
              else if (periodLengths[i] >= 72 && periodLengths[i] < 100)
              {
                periodLengths[i] = 1;
              }
              else
              {
                // indicate an error
                periodLengths[i] = 2;
              }
              
              timingValues.println(periodLengths[i]);
              
            }
            
            timingValues.close();   // close the file
            Serial.println("Done write process.");
            
        } else {
          // if the file didn't open, print an error:
          Serial.println("Error opening test.txt during write");
        }
        /* --------WRITE COMPLETE-------- */
        
        delay(2000);    // wait for 2 seconds 

        
        // Re-open for reading 
        timingValues = SD.open("test.txt");
        
        if (timingValues) {
          Serial.println("Re-opening test.txt for demodulation circuit output read...");
          Serial.println("Opened, test.txt file contents:");

          // read from the file 
          while (timingValues.available()) {
              Serial.write(timingValues.read());
              // this returns the next byte/charactrer of the file
              // then writes to serial port 
          }
          timingValues.close();
          
        } else {
          Serial.println("Error opening test.txt");
        }
        delay(1000);    // wait for 1 second, whole process takes 5 seconds
        lock = 0;
    }
}
