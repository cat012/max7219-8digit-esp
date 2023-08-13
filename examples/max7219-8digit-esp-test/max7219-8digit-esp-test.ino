//test


#include <max7219-8digit-esp.h>


Max7219 max7219(14, 12, 5);  //GPIO numbers for DIN, CS, CLK


//---------------------------------------------------------------------------------------------
void setup(void)
    {
    delay(100);
    max7219.ic_init(); //required
    max7219.set_bright(3); //brightness: 0-15 (default 0)
    }


//---------------------------------------------------------------------------------------------
void loop(void) 
    {
    static uint8_t c=0;
    static char strbuff[16];

    max7219.buff_print(0,"hello");
    max7219.buff_dots(4,1);
    sprintf(strbuff, "%3u", c++);
    max7219.buff_print(5,strbuff);
    max7219.disp_update();
    delay(1000);
    }
