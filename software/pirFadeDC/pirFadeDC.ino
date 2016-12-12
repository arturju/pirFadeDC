#define PIR_PIN          3
#define SWITCH_PIN       9   /*PWM only works on pins 3,5,6,9,10, and 11 */
#define TEST_PIN         10
#define DUTY_CYCLE_CAP   100
/* Power supply is 12v. Let's limit max voltage to 9V so PWM = 9/12= 75% duty cycle.
  Analog Write resolution is 8-bit so values are 2^8 = 256. range is 0-255.
  75% of 255  (rounded down) is 191 ADU. */

/* -----------------------------------------------------------
* Function for mapping floats 
* ------------------------------------------------------------*/
boolean IsTime(unsigned long *timeMark, unsigned long timeInterval) {
    if (millis() - *timeMark >= timeInterval) {
        *timeMark = millis();
        return true;
    }    
    return false;
}

/* -----------------------------------------------------------
* Function for mapping floats 
* ------------------------------------------------------------*/
float mapfloat(long x, long in_min, long in_max, long out_min, long out_max)
{
 return (float)(x - in_min) * (out_max - out_min) / (float)(in_max - in_min) + out_min;
}

/* --------------------------------------------------
* Function to limit maximum duty cycle based on macro and to convert percentage to ADU.
* Works only on pins 3,5,6,7,10, and 11.
* ------------------------------------------------------------*/
void outputPWM(int pin, float dutyCycle){
  
  if((int)dutyCycle >= DUTY_CYCLE_CAP) dutyCycle = DUTY_CYCLE_CAP;
  float analogDigitalUnit = floor(mapfloat(dutyCycle, 0, 100, 0, 255));
  analogWrite(pin,(int) analogDigitalUnit);
  
  float voltage = mapfloat(analogDigitalUnit, 0,255, 0, 5);   /* assumes voltage out of DC regulator is 5v */
//  Serial.print(" DutyCycle: "); Serial.print(dutyCycle);
//  Serial.print(", ADU: ");      Serial.print(analogDigitalUnit);
//  Serial.print(", voltage: ");  Serial.println(voltage);

}

/* -----------------------------------------------------------
* Main routine and variables
* ------------------------------------------------------------*/
void setup() {
  Serial.begin(9600);  
  pinMode(SWITCH_PIN, OUTPUT);
  pinMode(TEST_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);
}

/* --------------------------------------------------
* Start of main routine
* ------------------------------------------------------------*/
unsigned long int pirTimeOutInterval    = 30*1000;    /* PIR takes ~ 6 seconds to respond after rising edge */
unsigned long int switchUpdateInterval  = 50;         /* update fade value every 10 ms */
float             riseTime              = 15 *1000;   /* in ms */

float             calculatedSlope   = DUTY_CYCLE_CAP/(riseTime/switchUpdateInterval); 
unsigned long     switchTimeMark    = 0 ;
float             switchPWMval      = 0;
unsigned long     pirTimeMark       = 0 ;
int               pirPrevious;
int               state;

#define FADING_UP       3
#define FADING_DOWN     2
#define FADE_DONE       1 

void loop() {
  int pirNow = digitalRead(PIR_PIN);

/* Keeps track of how long it has been since movement happened */
   if( IsTime(&pirTimeMark,pirTimeOutInterval) )
   {
    digitalWrite(TEST_PIN, HIGH);
    state = FADING_DOWN;    
   }
  
  /* PIR Rising edge */
  if (pirNow == HIGH && pirPrevious == LOW)
  {
    digitalWrite(13, HIGH);         /* turn LED on */
    digitalWrite(TEST_PIN, LOW);
    pirTimeMark = millis();         /* resets timer for PIR */
    state = FADING_UP;
  }

  /* PIR Falling edge */
  if(pirNow == LOW && pirPrevious == HIGH)    digitalWrite(13, LOW);          

/*-------- State machine part ------------*/  
  switch(state){
    case FADING_UP:       
       if (switchPWMval >= 99.9)  {state = FADE_DONE;   break;} 
       if( IsTime(&switchTimeMark,switchUpdateInterval) )
       {
        switchPWMval += calculatedSlope;
        outputPWM(SWITCH_PIN, switchPWMval);
       }      
      break;
  
    case FADING_DOWN:    
      if (switchPWMval <= 0.01)  {state = FADE_DONE; digitalWrite(TEST_PIN, LOW); break;}
      if( IsTime(&switchTimeMark,switchUpdateInterval) )
      {
      switchPWMval -= calculatedSlope;
      outputPWM(SWITCH_PIN, switchPWMval);
      }      
      break;    
  
    case FADE_DONE:      
      break;
  }

  pirPrevious = pirNow;
} /* End of main loop() */

