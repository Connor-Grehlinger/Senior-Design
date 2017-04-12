/*  Team Breach Prototype Testing 
  Connor Grehlinger, Keith Doggett, Mathew Schlogel, John Oribioye, Matt Leung
  --------------------------------------------------------------
  
  This code will need to read a bitset and output
  that bitset as an encoded square wave signal 
  
  Encoding:
  
  0's --> 64 microsecond period
  1's --> 80 microsecond period 
  
  Output will be sqaure wave signal with 50% duty cycle 
  
  Captured bitsets will be X bits long (not sure yet)

 */
 
 
# define OUTPUT_PIN 5

int bitset [1000];          // dummy variable for encoded bitsets
//unsigned int i;

unsigned int dummyTimerValue;

void setup() {

    pinMode(OUTPUT_PIN, OUTPUT);                           // set pin mode
    pulseRiseTime = 0; 
    unsigned int i = 0;
    unsigned int j = 0;
    
    Serial.begin(115200);
    Serial.println("Finished initialization");
}

void loop() {

    for (i = 0; i < 1000; i++){
        for (j = 0; j < 32; j++){                           // using 32 for integers right now
            
            if ((bitset[i] >> j) &= 1){
                // it's a 1
                dummyTimerValue = 80;
                digitalWrite(OUTPUT_PIN, HIGH);
                delay(40);
                digitalWrite(OUTPUT_PIN, LOW);
                delay(40);
                
            }
            else{
                // it's a 0
                dummyTimerValue = 64;
                digitalWrite(OUTPUT_PIN, HIGH);
                delay(32);
                digitalWrite(OUTPUT_PIN, LOW);
                delay(32);
            }
        }
    }      
    
                    
}
 