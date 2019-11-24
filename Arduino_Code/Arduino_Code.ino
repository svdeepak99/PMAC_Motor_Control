#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>
#include<math.h>

/*
 * Circuit Connections:
 * 
 * Motor Driver PWM pins:
 * R Phase - 6 (PORTD6)
 * Y Phase - 5 (PORTD5)
 * B Phase - 11(PORTB3)
 * 
 * Motor Driver Dir (direction) pins:
 * R Phase - 8 (PORTB0)
 * Y Phase - 9 (PORTB1)
 * B Phase - 10(PORTB2)
 * 
 * Important Note: The Brk (brake) pins of the Motor Drivers must be connected to the Ground.
 */

float freq=0.0;
#define tp (1000000/freq)
#define offset ((2*pi)/3)
#define pi 3.14159
#define A 150
unsigned long long int tm=0;

float angle,r,y,b;

void compute()
{
    angle=0.002*0.001*pi*freq*tm;
    r=sin(angle)*A;
    y=sin(angle-offset)*A;
    b=sin(angle+offset)*A;
    
    if(r<0){
      PORTB|=1; //Setting PORTB0 as HIGH
      r=-r;
    }
    else
      PORTB&=~1; //Setting PORTB0 as LOW
      
    if(y<0){
      PORTB|=2; //Setting PORTB1 as HIGH
      y=-y;
    }
    else
      PORTB&=~2; //Setting PORTB1 as LOW
      
    if(b<0){
      PORTB|=4; //Setting PORTB2 as HIGH
      b=-b;
    }
    else
      PORTB&=~4; //Setting PORTB2 as LOW
      
    OCR0A=r;
    OCR0B=y;
    OCR2A=b;

}

void accelerate(int ft)
{
    //Function that gradually raised speed of rotating magnetic field to target Frequency (RPM/60)
    freq=0;
    for(float i=0;i<=ft;i+=0.5)
    {
      freq=i;
      Serial.println(freq*60);
      _delay_ms(200);
    }
}
  
int main()
{
  int tar_RPM=0;
  DDRB|=0b00001111; //PWM Pin 11 - OCR2A & Dir pins
  DDRD|=0b01100000; //PWM Pins 5 & 6

  TCCR0A|=(1<<WGM01)|(1<<WGM00)|(1<<COM0A1)|(1<<COM0B1);  //non-inverting Fast PWM - 5 & 6
  TCCR0B|=(1<<CS00); // Prescalar 1

  TCCR2A|=(1<<WGM21)|(1<<WGM20)|(1<<COM2A1);  //non-inverting Fast PWM - 11
  TCCR2B|=(1<<CS20);  // Prescalar 1
  
  TCCR1B=(1<<CS11)|(1<<CS10)|(1<<WGM13)|(1<<WGM12); //Prescalar 64 & CTC Mode
  TIMSK1|=(1<<ICIE1);   //ICR1 Interrupt Enable
  ICR1=255; //Setting TOP value

  Serial.begin(9600);
  
  sei();

  while(1)
  {
    if(Serial.available()>0)
    {
      _delay_ms(10);
      tar_RPM=Serial.parseInt();  //Get Target RPM from User
      Serial.println(tar_RPM);    //Print Target RPM back to the Serial Monitor for verification
      accelerate(tar_RPM/60);     //Sending Target frequncy to accelerate function
    }
    _delay_ms(10);
  }

  return 0;
}

ISR(TIMER1_CAPT_vect)
{
  //Timer Interrupt function that regularly updates PWM dutycycles
  tm+=16*64;
  if(tm>=tp)
    tm=0;
  compute();
}
