/**
 *  @filename   :   main.cpp
 *  @brief      :   Quad VFO Generator
 *
 *  @author     :   Kevin Kessler
 *
 * Copyright (C) 2022 Kevin Kessler
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, 
 * publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, 
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE 
 * FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <Arduino.h>
#include <si5351.h>
#include <Wire.h>
#include <U8g2lib.h>
#include "cwtransceiver.h"
#define ENCODER_OPTIMIZE_INTERRUPTS
#include "Encoder.h"
#include "Bounce.h"


#define CORRECTION (13859)
#define PHASE_DELAY (57300)       // Delay is 62500 minus I2C and CPU time. Found by trial and error at 1.9MHz

volatile uint8_t rotB=0;
volatile uint8_t rotA=0;
volatile uint8_t rotState=0;

volatile uint32_t freq=14000000;
uint32_t oldfreq=0;
volatile int32_t radix=100000;
int32_t oldradix=radix;


U8G2_ST7565_ERC12864_F_4W_SW_SPI u8g2(U8G2_R0, 31, 32,/* cs=*/ 37, /* dc=*/38, /* reset=*/ 39);
Si5351 si5351;

volatile int Even_Divisor = 0;
volatile int oldEven_Divisor = 0;

Encoder knob(ROTA, ROTB);
Bounce button(ROTBUTTON,50);

uint32_t set_freq=2000000;

void EvenDivisor()
{
  if(freq <2100000) {
    Even_Divisor = 450;
    set_freq = 1800000;
  }

  if((freq>=2100000) && (freq < 2500000)) {
    Even_Divisor = 350;
    set_freq = 2200000;
  }

  if ((freq >= 2500000) && (freq < 3200000))
  {
    Even_Divisor = 274;
    set_freq = 3000000;
  }
  if ((freq >= 3200000) && (freq < 6850000))
  {
    Even_Divisor = 126;
  }
  if ((freq >= 6850000) && (freq < 9500000))
  {
    Even_Divisor = 88;
  }
  if ((freq >= 9500000) && (freq < 13600000))
  {
    Even_Divisor = 64;
  }
  if ((freq >= 13600000) && (freq < 17500000))
  {
    Even_Divisor = 44;
  }
  if ((freq >= 17500000) && (freq < 25000000))
  {
    Even_Divisor = 34;
  }
  if ((freq >= 25000000) && (freq < 36000000))
  {
    Even_Divisor = 24;
  }
  if ((freq >= 36000000) && (freq < 45000000)) {
    Even_Divisor = 18;
  }
  if ((freq >= 45000000) && (freq < 60000000)) {
    Even_Divisor = 14;
  }
  if ((freq >= 60000000) && (freq < 80000000)) {
    Even_Divisor = 10;
  }
  if ((freq >= 80000000) && (freq < 100000000)) {
    Even_Divisor = 8;
  }
  if ((freq >= 100000000) && (freq < 146600000)) {
    Even_Divisor = 6;
  }
  if ((freq >= 150000000) && (freq < 220000000)) {
    Even_Divisor = 4;
  }
}

void TimeDelayPhasing() 
{

  if(Even_Divisor != oldEven_Divisor) {

    Serial.println("Delay Phase");
    si5351.set_phase(SI5351_CLK0, 0);
    si5351.set_phase(SI5351_CLK1, 0);
    si5351.set_freq_manual((set_freq + 4) * SI5351_FREQ_MULT, set_freq * Even_Divisor * SI5351_FREQ_MULT,SI5351_CLK0);
    si5351.set_freq_manual((set_freq + 4)* SI5351_FREQ_MULT, set_freq * Even_Divisor * SI5351_FREQ_MULT,SI5351_CLK1);
    si5351.pll_reset(SI5351_PLLA);
    si5351.set_freq_manual(set_freq * SI5351_FREQ_MULT, set_freq * Even_Divisor * SI5351_FREQ_MULT,SI5351_CLK1);
    delayMicroseconds(PHASE_DELAY);
    si5351.set_freq_manual(set_freq * SI5351_FREQ_MULT, set_freq * Even_Divisor * SI5351_FREQ_MULT,SI5351_CLK0);
    oldEven_Divisor = Even_Divisor;
  }


  Serial.printf("Set Freq %ld\n",freq);
  si5351.set_freq_manual(freq * SI5351_FREQ_MULT, Even_Divisor * freq * SI5351_FREQ_MULT, SI5351_CLK0);
  si5351.set_freq_manual(freq * SI5351_FREQ_MULT, Even_Divisor * freq * SI5351_FREQ_MULT, SI5351_CLK1);
  

}

void SendFrequency()
{
  EvenDivisor();
  Serial.printf("EvenDivisor %d \n",Even_Divisor);
  if(Even_Divisor > 127)
    TimeDelayPhasing();
  else {
    si5351.set_freq_manual(freq * SI5351_FREQ_MULT, Even_Divisor * freq * SI5351_FREQ_MULT, SI5351_CLK0);
    si5351.set_freq_manual(freq * SI5351_FREQ_MULT, Even_Divisor * freq * SI5351_FREQ_MULT, SI5351_CLK1);
    si5351.set_phase(SI5351_CLK0, 0);
    si5351.set_phase(SI5351_CLK1, Even_Divisor);
  
    if(Even_Divisor != oldEven_Divisor)
    {
      si5351.pll_reset(SI5351_PLLA);
      oldEven_Divisor = Even_Divisor;
    }
  }
}

void readRot() {
  static unsigned long knobPos=0;

  if(button.update()) {
    if(button.fallingEdge()) {
      radix = radix / 10;
      if (radix < 1)
        radix = 100000;     
    }
  }

  unsigned long curKnobPos=knob.read();
  
  if(curKnobPos==knobPos)
    return;

  
  if(((curKnobPos/4)*4) != curKnobPos) //Every 4 counts is actually a rotary click
    return;


  Serial.printf("Knob %d %d \n",curKnobPos,knobPos);
  if(knobPos < curKnobPos) {


    freq = freq + radix;
    if (freq > BANDEND)
      freq = BANDEND;
  
    
  } else {

    freq = freq - radix;
    if (freq < BANDSTART)
      freq = BANDSTART;
    
  }
  knobPos=curKnobPos;
}

void UpdateDisplay() {
  u8g2.setColorIndex(1);
  u8g2.clearDisplay();
  u8g2.setFont(u8g2_font_9x18B_tf);
  
  u8g2.clearBuffer();
  u8g2.setContrast(0xf);

  char buffer[20];

  switch(radix) {
    case 1000000:
      u8g2.drawStr(14,18,"_");
      break;
    case 100000:
      u8g2.drawStr(32,18,"_");
      break;
    case 10000:
      u8g2.drawStr(41,18,"_");
      break;
    case 1000:
      u8g2.drawStr(50,18,"_");
      break;
    case 100:
      u8g2.drawStr(68,18,"_");
      break;
    case 10:
      u8g2.drawStr(77,18,"_");
      break;
    case 1:
      u8g2.drawStr(86,18,"_");
      break;
  }
  uint8_t meg=freq/1000000;
  uint16_t mills=(freq-meg*1000000)/1000;
  uint16_t mics=(freq-meg*1000000-mills*1000);
  sprintf(buffer,"%2d.%03d %03d",meg,mills,mics);
  u8g2.drawStr(4,16, buffer);
  u8g2.setFont(u8g2_font_9x15_tf);
  u8g2.drawStr(100,16,"MHz");
  u8g2.sendBuffer();
}



void setup() {
  Serial.begin(115200);
  while(!Serial) {}


  Wire2.begin();

  si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0,&Wire2);
  Wire2.setClock(400000);
  si5351.set_correction(CORRECTION,SI5351_PLL_INPUT_XO);


  delay(500);

  pinMode(ROTBUTTON, INPUT_PULLUP);


  Serial.println("Starting...");
  u8g2.begin();
  u8g2.setColorIndex(1);
  u8g2.clearDisplay();
  u8g2.setFont(u8g2_font_9x18B_tf);
  
  u8g2.clearBuffer();
  u8g2.setContrast(0xf);
  u8g2.drawStr(100,16,"MHz");
  u8g2.sendBuffer();
  
}


void loop() {

  readRot();
  if(oldfreq!=freq)
  {
    SendFrequency();
    oldfreq=freq;
    UpdateDisplay();
  } else if(oldradix != radix) {
    oldradix=radix;
    UpdateDisplay();
  }

}