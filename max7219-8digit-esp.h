// max7219.h
// max7219 8-digit display driver for ESP8266 (software SPI)
// (^^)~ beta 07 08 '23


#ifndef MAX7219_H_
#define MAX7219_H_


#include <Arduino.h>


class Max7219
{
public:
  Max7219(uint8_t dt, uint8_t cs, uint8_t ck);
  
  void ic_init(void);
  void set_bright(uint8_t value);
  void ic_shutdown(uint8_t mod);

  void buff_char(uint8_t xpos, uint8_t code);
  void buff_print(uint8_t xpos, const char *str);
  void buff_dots(uint8_t xpos, uint8_t state);
  void disp_update(void);

private:
  uint8_t _dt;
  uint8_t _cs;
  uint8_t _ck;

  void spi_byte(uint8_t data);
  void ic_write(uint8_t high, uint8_t low);
};



#endif /* MAX7219_H_ */
