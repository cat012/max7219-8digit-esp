// max7219.c
// max7219 8-digit display driver for ESP8266 (software SPI)
// (^^)~ beta 07 08 '23


#include "max7219-8digit-esp.h"


#define MAX7219_BUFF_SIZE  8
uint8_t max7219buff[MAX7219_BUFF_SIZE] = {0};


const uint8_t dfont[] = //habcdefg
{
  0b01111110,  //0
  0b00110000,  //1
  0b01101101,  //2
  0b01111001,  //3
  0b00110011,  //4
  0b01011011,  //5
  0b01011111,  //6
  0b01110000,  //7
  0b01111111,  //8
  0b01111011,  //9
  0b00000000,  //space //10
  0b00000001,   //-    //11
  0b01110111,  //A     //12
  0b01001110,  //C     //13
  0b01001111,  //E     //14
  0b01000111,  //F     //15
  0b01011110,  //G     //16
  0b00110111,  //H     //17
  0b00111100,  //J     //18
  0b00001110,  //L     //19
  0b01100111,  //P     //20
  0b00111110,  //U     //21
  0b00011111,  //b     //22
  0b00111101,  //d     //23
  0b00001101,  //c     //24
  0b00010111,  //h     //25
  0b00010101,  //n     //26
  0b00011101,  //o     //27
  0b01110011,  //q     //28
  0b00000101,  //r     //29
  0b00001111,  //t     //30
  0b00011100,  //u     //31
  0b00111011   //y     //32
/*  habcdefg*/
};


Max7219 :: Max7219(uint8_t dt, uint8_t cs, uint8_t ck)
{
  pinMode(cs, OUTPUT);
  digitalWrite(cs, HIGH);
  _cs = cs;
  
  pinMode(ck, OUTPUT);
  digitalWrite(ck, LOW);
  _ck = ck;
  
  pinMode(dt, OUTPUT);
  digitalWrite(dt, LOW);
  _dt = dt;
}


//================================================================================================
void Max7219 :: spi_byte(uint8_t data)  //send byte
    {
    for(uint8_t mask=0x80; mask; mask>>=1)
        {
        (data & mask) ? digitalWrite(_dt, HIGH) : digitalWrite(_dt, LOW); //data line
        digitalWrite(_ck, HIGH); //clock pulse
        digitalWrite(_ck, LOW);
        }
    }


//================================================================================================
void Max7219 :: ic_write(uint8_t high, uint8_t low)  //send two bytes to IC
    {
    digitalWrite(_cs, LOW);
    spi_byte(high);
    spi_byte(low);
    digitalWrite(_cs, HIGH);;
    }


//-------------------------------------------------------------------------------------------------
void Max7219 :: ic_init(void)
    {
    ic_write(0x0C, 0x00);  //Shutdown Register      //shutdown mode
    ic_write(0x09, 0x00);  //Decode-Mode Register   //no decode
    ic_write(0x0A, 0x00);  //Intensity Register     //minimum brightness
    ic_write(0x0B, 0x07);  //Scan-Limit Register    //display all digits
    ic_write(0x0F, 0x00);  //Display-Test Register  //normal operation

    for(uint8_t k=0; k<8; k++) ic_write(k+1, 0x00); //clear screen

    ic_write(0x0C, 0x01);  //Shutdown Register      //normal operation
    }


//-------------------------------------------------------------------------------------------------
// print a character  //xpos: 0-7  //character code: 0-255
//-------------------------------------------------------------------------------------------------
void Max7219 :: buff_char(uint8_t xpos, uint8_t code)
    {
    uint8_t c=11;                        //"-" //and for undefined characters

    if(code>=48 && code<=57) c=code-48;  //0-9
    else if(code==32) c=10;              //"space"
    else if(code==65 || code==97) c=12;  //A //a
    else if(code==67) c=13;              //C
    else if(code==69 || code==101) c=14; //E //e
    else if(code==70 || code==102) c=15; //F //f
    else if(code==71 || code==103) c=16; //G //g
    else if(code==72) c=17;              //H
    else if(code==74 || code==106) c=18; //J //j
    else if(code==76 || code==108) c=19; //L //l
    else if(code==80 || code==112) c=20; //P //p
    else if(code==85) c=21;              //U
    else if(code==98 || code==66) c=22;  //b //B
    else if(code==100 || code==68) c=23; //d //D
    else if(code==99) c=24;              //c
    else if(code==104) c=25;             //h
    else if(code==110 || code==78) c=26; //n //N
    else if(code==111 || code==79) c=27; //o //O
    else if(code==113 || code==81) c=28; //q //Q
    else if(code==114 || code==82) c=29; //r //R
    else if(code==116 || code==84) c=30; //t //T
    else if(code==117) c=31;             //u
    else if(code==121 || code==89) c=32; //y //Y

    max7219buff[xpos&0b00000111]=dfont[c]|(max7219buff[xpos&0b00000111]&0b10000000);
    }


//-------------------------------------------------------------------------------------------------
// print a string  //xpos: 0-7
//-------------------------------------------------------------------------------------------------
void Max7219 :: buff_print(uint8_t xpos, const char *str)
    {
    for(; *str; xpos++) buff_char(xpos,*str++);
    }


//-------------------------------------------------------------------------------------------------
void Max7219 :: buff_dots(uint8_t xpos, uint8_t state)
    {
    state ? max7219buff[xpos&0b00000111]|=0b10000000 : max7219buff[xpos&0b00000111]&=~0b10000000;
    }


//-------------------------------------------------------------------------------------------------
void Max7219 :: disp_update(void)
    {
    for(uint8_t i=1; i<9; i++) ic_write(9-i, max7219buff[i-1]);
    for(uint8_t k=0; k<8; k++) max7219buff[k]=0x00;
    }


//-------------------------------------------------------------------------------------------------
void Max7219 :: ic_shutdown(uint8_t mod)  //mode: 0-shutdown mode, 1-normal operation
    {
    ic_write(0x0C, mod&0b00000001);
    }


//-------------------------------------------------------------------------------------------------
void Max7219 :: set_bright(uint8_t value)  //brightness: 0-15
    {
    ic_write(0x0A, value&0b00001111);
    }
