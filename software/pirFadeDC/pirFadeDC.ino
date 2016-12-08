/* This code will fade Digital pin in&out */

#define DEBUG
#define OUTPUT_PIN  9   /*PWM only works on pins 3,5,6,9,10, and 11 */
#define OTHER_PIN   10
#define INPUT_PIN   3

/* Power supply is 12v. Let's limit max voltage to 9V so PWM = 9/12= 75% duty cycle.
  Analog Write resolution is 8-bit so values are 2^8 = 256. range is 0-255.
  75% of 255  (rounded down) is 191 ADU. */
#define DUTY_CYCLE_CAP   75

void setup() {
  #ifdef DEBUG
  Serial.begin(9600);  
  #endif
  pinMode(OUTPUT_PIN, OUTPUT);
    pinMode(OTHER_PIN, OUTPUT);
  pinMode(INPUT_PIN, INPUT);
}

/* --------------------------------------------------
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
  
  #ifdef DEBUG
  float voltage = mapfloat(analogDigitalUnit, 0,255, 0, 5);   /* assumes voltage out of DC regulator is 5v */
  Serial.print(" DutyCycle: "); Serial.print(dutyCycle);
  Serial.print(", ADU: ");      Serial.print(analogDigitalUnit);
  Serial.print(", voltage: ");  Serial.println(voltage);
  #endif    
}

/* --------------------------------------------------
* Start of main routine
* ------------------------------------------------------------*/

float setBrightness = 75;                   /* in percentage */
float riseTime      = 5;                        /* in seconds */
float calculatedSlope = (setBrightness/riseTime)/10.0;      /* y/x, lvl/time. brightness amount per 100ms */

/* for tracking PIR pin status */
int pinNow;
int pinPrevious;
float outPinLvl = 0;

/* for fading without delay */
long timeNow;

long oPinTimeElapsed;
long oPinPreviousTime =0;
long oPinUpdateInterval = 100; /* in ms */

/* for PIR timer */
long timeMotionElapsed;
long timeAtStopMotion = 0;
long timeNoMotion = 10*1000;

int timerStart =0;
int startFadingUp =0;
int startFadingDown = 0;
  
void loop() {
  timeNow = millis();
  pinNow = digitalRead(INPUT_PIN);
  oPinTimeElapsed = timeNow - oPinPreviousTime;
  timeMotionElapsed = timeNow -timeAtStopMotion;

  /* Triggers on rising edge */
  if (pinNow == HIGH && pinPrevious == LOW){
    Serial.print("pin enabled.");
    digitalWrite(13, HIGH); /* turn LED on */
    startFadingUp = 1;      
        
    if(timerStart){
      Serial.print("Restarting timer");
      timeAtStopMotion = timeNow;
    }
    Serial.println("");
  }

  if(startFadingUp && (oPinTimeElapsed >= oPinUpdateInterval)){
    oPinPreviousTime = timeNow;
    outPinLvl += calculatedSlope;
    outputPWM(OUTPUT_PIN, outPinLvl);

    if (outPinLvl >= setBrightness) startFadingUp = 0;
  }

  if(startFadingDown && (oPinTimeElapsed >= oPinUpdateInterval)){
    oPinPreviousTime = timeNow;
    outPinLvl -= calculatedSlope;
    outputPWM(OUTPUT_PIN, outPinLvl);

     if (outPinLvl <= 0) startFadingDown = 0;
  }

  if( timerStart && (timeMotionElapsed >= timeNoMotion)){
    digitalWrite(13, LOW);
    Serial.println("Motion timer elapsed");
    startFadingUp = 0;
    startFadingDown = 1;
    timerStart = 0;        
  }

  
/* if falling edge and timer not already on, start it */
  if(pinNow == LOW && pinPrevious == HIGH && !timerStart){
    Serial.println("Falling edge");
    timerStart = 1;
    timeAtStopMotion = timeNow;
  }
  
  pinPrevious = pinNow;  
}





