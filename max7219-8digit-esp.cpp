// max7219.c
// max7219 8-digit display driver for ESP8266 (software SPI)
// (^^)~  beta 2  13 08 '23


#include "max7219-8digit-esp.h"


static const uint8_t SHUTDOWN_REGISTER_ADDR = 0x0C;
static const uint8_t SHUTDOWN_REG_NORMAL_MODE = 0x01;
static const uint8_t SHUTDOWN_REG_SHUTDOWN_MODE = 0x00;

static const uint8_t DECMODE_REGISTER_ADDR = 0x09;
static const uint8_t DECMODE_REG_NO_DECODE = 0x00;

static const uint8_t INTESITY_REGISTER_ADDR = 0x0A;
static const uint8_t INTESITY_REG_MIN_BRIGHT = 0x00;

static const uint8_t SCANLIM_REGISTER_ADDR = 0x0B;
static const uint8_t SCANLIM_REG_DISP_ALL = 0x07;

static const uint8_t DISPTEST_REGISTER_ADDR = 0x0F;
static const uint8_t DISPTEST_REG_NORMAL_OP = 0x00;
static const uint8_t DISPTEST_REG_DISP_TETST = 0x01;


static const uint8_t MAX7219_BUFF_SIZE = 8;
static uint8_t max7219buff[MAX7219_BUFF_SIZE] = {0};


static const uint8_t dfont[] = //habcdefg
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


static const uint8_t FONT_MAP_SIZE = 37;
static const uint8_t FONT_MAP_CHAR_CODE = 0;
static const uint8_t FONT_MAP_DFONT_NUM = 1;
static const uint8_t fontmap[FONT_MAP_SIZE][2] =
{
  {' ',10},
  {'A',12},
  {'B',22},
  {'C',13},
  {'D',23},
  {'E',14},
  {'F',15},
  {'G',16},
  {'H',17},
  {'J',18},
  {'L',19},
  {'N',26},
  {'O',27},
  {'P',20},
  {'Q',28},
  {'R',29},
  {'T',30},
  {'U',21},
  {'Y',32},
  {'a',12},
  {'b',22},
  {'c',24},
  {'d',23},
  {'e',14},
  {'f',15},
  {'g',16},
  {'h',25},
  {'j',18},
  {'l',19},
  {'n',26},
  {'o',27},
  {'p',20},
  {'q',28},
  {'r',29},
  {'t',30},
  {'u',31},
  {'y',32}
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
    ic_write(SHUTDOWN_REGISTER_ADDR, SHUTDOWN_REG_SHUTDOWN_MODE);
    ic_write(DECMODE_REGISTER_ADDR,  DECMODE_REG_NO_DECODE);
    ic_write(INTESITY_REGISTER_ADDR, INTESITY_REG_MIN_BRIGHT);
    ic_write(SCANLIM_REGISTER_ADDR,  SCANLIM_REG_DISP_ALL);
    ic_write(DISPTEST_REGISTER_ADDR, DISPTEST_REG_NORMAL_OP);

    for(uint8_t k=0; k<8; k++) ic_write(k+1, 0x00); //clear screen

    ic_write(SHUTDOWN_REGISTER_ADDR, SHUTDOWN_REG_NORMAL_MODE);
    }


//-------------------------------------------------------------------------------------------------
// print a character  //xpos: 0-7  //character code: 0-255
//-------------------------------------------------------------------------------------------------
void Max7219 :: buff_char(uint8_t xpos, uint8_t code)
    {
    uint8_t c = '-';                     //for undefined characters

    if(code>=48 && code<=57) c=code-48;  //0-9
    else
        {
        for(uint8_t k=0; k<FONT_MAP_SIZE; k++) 
            {
            if(code==fontmap[k][FONT_MAP_CHAR_CODE])
                {
                c=fontmap[k][FONT_MAP_DFONT_NUM];
                break;
                }
            }
        }

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
    for(uint8_t i=1; i<((MAX7219_BUFF_SIZE)+1); i++) ic_write(((MAX7219_BUFF_SIZE)+1)-i, max7219buff[i-1]);
    for(uint8_t k=0; k<MAX7219_BUFF_SIZE; k++) max7219buff[k]=0x00;
    }


//-------------------------------------------------------------------------------------------------
void Max7219 :: ic_shutdown(uint8_t mod)  //mode: 0-shutdown mode, 1-normal operation
    {
    ic_write(SHUTDOWN_REGISTER_ADDR, mod&0b00000001);
    }


//-------------------------------------------------------------------------------------------------
void Max7219 :: set_bright(uint8_t value)  //brightness: 0-15
    {
    ic_write(INTESITY_REGISTER_ADDR, value&0b00001111);
    }
