//  G27 shifter pinout
//
//  DB9  Description        Pro Micro
//  1    Clock              5
//  2    Ourput Data        7
//  3    Inpt Data          6
//  4    Shifter X axis     A0
//  5    LED                10
//  6    GND                GND
//  7    +5V                VCC
//  8    Shifter Y axis     A1
//  9    -                  -

// include library that turns the Pro Mirco into a gamecontroller
#include "Joystick.h"

// Pro Micro pin definitions
#define LED_PIN            10
#define DATA_IN_PIN        7
#define MODE_PIN           6
#define CLOCK_PIN          5
#define X_AXIS_PIN         A0
#define Y_AXIS_PIN         A1

// H-shifter mode analog axis thresholds
#define HS_XAXIS_12        340
#define HS_XAXIS_56        580
#define HS_YAXIS_135       750
#define HS_YAXIS_246       200

// pin that controlls reverse gear
#define DI_REVERSE         1

void setup()
{
  Joystick.begin();
  
  // G27 shifter analog inputs configuration 
  pinMode(X_AXIS_PIN, INPUT_PULLUP);   // X axis
  pinMode(Y_AXIS_PIN, INPUT_PULLUP);   // Y axis
  
  // G27 shift register interface configuration 
  pinMode(DATA_IN_PIN, INPUT);         // Data in
  pinMode(MODE_PIN, OUTPUT);           // Parallel/serial mode
  pinMode(CLOCK_PIN, OUTPUT);          // Clock
  
  // LED output mode configuration 
  pinMode(LED_PIN, OUTPUT);            // LED
     
  // Digital outputs initialization
  digitalWrite(LED_PIN, HIGH);
  digitalWrite(MODE_PIN, HIGH);
  digitalWrite(CLOCK_PIN, HIGH); 
}

// Called in a loop after initialization
void loop() 
{
  
  // Reading of button states from G27 shift register
  int b[16];
  
  digitalWrite(MODE_PIN, LOW);         // Parallel mode: inputs are read into shift register
  delayMicroseconds(10);               // Wait for signal to settle
  digitalWrite(MODE_PIN, HIGH);        // Serial mode: data bits are output on clock falling edge
  
  for(int i=0; i<16; i++)              // Iteration over both 8 bit registers
  {
    digitalWrite(CLOCK_PIN, LOW);      // Generate clock falling edge
    delayMicroseconds(10);             // Wait for signal to settle
  
    b[i]=digitalRead(DATA_IN_PIN);     // Read data bit and store it into bit array
    
    digitalWrite(CLOCK_PIN, HIGH);     // Generate clock rising edge
    delayMicroseconds(10);             // Wait for signal to settle
  }

  // Reading of shifter position
  int x=analogRead(0);                 // X axis
  int y=analogRead(1);                 // Y axis
 
  // Current gear calculation
  int gear=0;                          // Default value is neutral  
  if(x<HS_XAXIS_12)                  // Shifter on the left?
  {
    if(y>HS_YAXIS_135) gear=1;       // 1st gear
    if(y<HS_YAXIS_246) gear=2;       // 2nd gear
  }
  else if(x>HS_XAXIS_56)             // Shifter on the right?
  {
    if(y>HS_YAXIS_135) gear=5;       // 5th gear
    if(y<HS_YAXIS_246) gear=6;       // 6th gear
  }
  else                               // Shifter is in the middle
  {
    if(y>HS_YAXIS_135) gear=3;       // 3rd gear
    if(y<HS_YAXIS_246) gear=4;       // 4th gear
  }

  // check if in reverse gear
  if(gear!=6) b[DI_REVERSE]=0;       // Reverse gear is allowed only on 6th gear position
  if(b[DI_REVERSE]==1)               // 6th gear is deactivated if reverse gear is engaged
  {
    gear=7;                          // reverse gear
    digitalWrite(LED_PIN, LOW);      // enable LED when reverse gear is selected
  }
  else
  {
    digitalWrite(LED_PIN, HIGH);     // disable LED if not in reverse gear
  }

// connect gear selection to G27 shifter button
// gear 1-6 = button 0-5
// reverse gear = button 6
for (int i = 0; i < 7; i++)
{
  if (gear == i+1)
  {
    Joystick.pressButton(i);  
  }
  else
  {
    Joystick.releaseButton(i);  
  }
}

// connect joystick button to G27 shifter buttons
for (int k = 0; k < 12; k++)
{
  if (b[k+4] == 1)
  {
    Joystick.pressButton(k+7);  
  }
  else
  {
    Joystick.releaseButton(k+7);  
  }
}

// send game controller state to PC
Joystick.sendState();  
}
