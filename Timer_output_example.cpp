const int ocr1aPin = 9;

// Set the frequency that we will get on pin OCR1A
void setFrequency(uint16_t freq)
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

void setup()
{
  digitalWrite(ocr1aPin, LOW);
  pinMode(ocr1aPin, OUTPUT);
}