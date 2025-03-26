#include <string.h>

#include "lcd.h"
#include "pins.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"

#define ___ 0
#define __X 1
#define _X_ 2
#define _XX 3
#define X__ 4
#define X_X 5
#define XX_ 6
#define XXX 7
#define PACK_5_TO_16(A,B,C,D,E) ((A) | (B<<3) | (C<<6) | (D<<9) | (E<<12))
 // 48

#define LCD_FONT_3X5_CHARS 95
const unsigned short LCD_FONT_3X5[] = { // from 33 up to 127
    PACK_5_TO_16( XXX , _X_ , XXX , XX_ , X_X ), // 01234
    PACK_5_TO_16( X_X , XX_ , __X , __X , X_X ),
    PACK_5_TO_16( X_X , _X_ , _X_ , XX_ , XXX ),
    PACK_5_TO_16( X_X , _X_ , X__ , __X , __X ),
    PACK_5_TO_16( XXX , XXX , XXX , XX_ , __X ),

    PACK_5_TO_16( XXX , XXX , XXX , XXX , XXX ), // 56789
    PACK_5_TO_16( X__ , X__ , __X , X_X , X_X ),
    PACK_5_TO_16( XXX , XXX , _X_ , XXX , XXX ),
    PACK_5_TO_16( __X , X_X , _X_ , X_X , __X ),
    PACK_5_TO_16( XXX , XXX , _X_ , XXX , XXX ),

    PACK_5_TO_16( ___ , ___ , __X , ___ , X__ ), // :;<=>
    PACK_5_TO_16( _X_ , _X_ , _X_ , XXX , _X_ ),
    PACK_5_TO_16( ___ , ___ , X__ , ___ , __X ),
    PACK_5_TO_16( _X_ , _X_ , _X_ , XXX , _X_ ),
    PACK_5_TO_16( ___ , X__ , __X , ___ , X__ ),

    PACK_5_TO_16( _X_ , ___ , _X_ , XX_ , _XX ), // ?@ABC
    PACK_5_TO_16( X_X , _X_ , X_X , X_X , X__ ), // @ is used as +
    PACK_5_TO_16( __X , XXX , XXX , XX_ , X__ ),
    PACK_5_TO_16( ___ , _X_ , X_X , X_X , X__ ),
    PACK_5_TO_16( _X_ , ___ , X_X , XX_ , _XX ),

    PACK_5_TO_16( XX_ , XXX , XXX , _XX , X_X ), // DEFGH
    PACK_5_TO_16( X_X , X__ , X__ , X__ , X_X ),
    PACK_5_TO_16( X_X , XX_ , XXX , X_X , XXX ),
    PACK_5_TO_16( X_X , X__ , X__ , X_X , X_X ),
    PACK_5_TO_16( XX_ , XXX , X__ , _XX , X_X ),

    PACK_5_TO_16( XXX , XXX , X_X , X__ , X_X ), // IJKLM
    PACK_5_TO_16( _X_ , __X , X_X , X__ , XXX ),
    PACK_5_TO_16( _X_ , __X , XX_ , X__ , XXX ),
    PACK_5_TO_16( _X_ , __X , X_X , X__ , X_X ),
    PACK_5_TO_16( XXX , XX_ , X_X , XXX , X_X ),

    PACK_5_TO_16( XX_ , _XX , XX_ , _X_ , XX_ ), // NOPQR
    PACK_5_TO_16( X_X , X_X , X_X , X_X , X_X ),
    PACK_5_TO_16( X_X , X_X , XX_ , X_X , XX_ ),
    PACK_5_TO_16( X_X , X_X , X__ , X_X , X_X ),
    PACK_5_TO_16( X_X , _X_ , X__ , _XX , X_X ),

    PACK_5_TO_16( _XX , XXX , X_X , X_X , X_X ), // STUVW
    PACK_5_TO_16( X__ , _X_ , X_X , X_X , X_X ),
    PACK_5_TO_16( _X_ , _X_ , X_X , X_X , XXX ),
    PACK_5_TO_16( __X , _X_ , X_X , _X_ , XXX ),
    PACK_5_TO_16( XX_ , _X_ , XXX , _X_ , X_X ),

    PACK_5_TO_16( X_X , X_X , XXX , _XX , ___ ), // XYZ[\ end
    PACK_5_TO_16( X_X , X_X , __X , _X_ , ___ ), // \ is used as .
    PACK_5_TO_16( _X_ , _X_ , _X_ , _X_ , ___ ),
    PACK_5_TO_16( X_X , _X_ , X__ , _X_ , ___ ),
    PACK_5_TO_16( X_X , _X_ , XXX , _XX , _X_ ),
};


// LCD data buffer
unsigned char lcdBuffer[(LCD_STRIDE*LCD_HEIGHT) + 2/* final 2 bytes of 0 needed */];
uint8_t cursor_col = 0xF;

#define LCD_CURSOR_X_START 4
#define LCD_CURSOR_Y_START 4
int cursor_x = LCD_CURSOR_X_START, cursor_y = LCD_CURSOR_Y_START;

// utils
int reverse_byte(int v) {
  unsigned int b = v&0xFF;
  // http://graphics.stanford.edu/~seander/bithacks.html#ReverseByteWith64Bits
  return (((b * 0x0802LU & 0x22110LU) | (b * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16) & 0xFF;
}

uint8_t lcd_get_pixel(int x, int y) {
  if (x<0 || y<0 || x>=LCD_WIDTH || y>=LCD_HEIGHT) return 0;
  int addr = LCD_ROWHEADER + (x>>1) + (y*LCD_STRIDE);
  unsigned char b = lcdBuffer[addr];
  unsigned int c = (x&1) ? ((b>>1)&7) : (b>>5);
  return c;
}

void lcd_set_pixel(int x, int y, uint8_t col) {
  if (x<0 || y<0 || x>=LCD_WIDTH || y>=LCD_HEIGHT) return;
  col = col&7;
  int addr = LCD_ROWHEADER + (x>>1) + (y*LCD_STRIDE);
  if (x&1) lcdBuffer[addr] = (lcdBuffer[addr] & 0x0F) | (col << 4);
  else lcdBuffer[addr] = (lcdBuffer[addr] & 0xF0) | col;
}


void lcd_flip() {
  nrf_gpio_pin_set(LCD_SPI_CS);
  asm("nop");asm("nop");asm("nop");asm("nop");
  for (int i=0;i<sizeof(lcdBuffer);i++) {
    int data = lcdBuffer[i];
    for (int bit=0;bit<8;bit++) {
      nrf_gpio_pin_clear(LCD_SPI_SCK);
      asm("nop");asm("nop");asm("nop");asm("nop");
      nrf_gpio_pin_write(LCD_SPI_MOSI, (data>>bit)&1);
      asm("nop");asm("nop");asm("nop");asm("nop");
      nrf_gpio_pin_set(LCD_SPI_SCK);
      asm("nop");asm("nop");asm("nop");asm("nop");
    }
  }
  nrf_gpio_pin_clear(LCD_SPI_MOSI);
  nrf_gpio_pin_clear(LCD_SPI_SCK);
  nrf_gpio_pin_clear(LCD_SPI_CS);
}

// call this once a second to avoid (temporary) burn-in on the LCD
void lcd_toggle() {
  nrf_gpio_pin_set(LCD_EXTCOMIN);
  nrf_delay_us(2);
  nrf_gpio_pin_clear(LCD_EXTCOMIN);
}

void lcd_set_backlight(bool on) {
  nrf_gpio_pin_write(LCD_BL,on);
}


void lcd_init() {
  memset(lcdBuffer,0,sizeof(lcdBuffer));
  for (int y=0;y<LCD_HEIGHT;y++) {
    lcdBuffer[y*LCD_STRIDE]=reverse_byte(0b10010000); // 4 bit mode
    lcdBuffer[(y*LCD_STRIDE)+1]=reverse_byte(y+1);
  }

  nrf_gpio_pin_write(LCD_SPI_CS,0);
  nrf_gpio_cfg_output(LCD_SPI_CS);
  nrf_gpio_pin_write(LCD_SPI_SCK,1);
  nrf_gpio_cfg_output(LCD_SPI_SCK);
  nrf_gpio_pin_write(LCD_SPI_MOSI,1);
  nrf_gpio_cfg_output(LCD_SPI_MOSI);
  nrf_gpio_pin_write(LCD_DISP,1);
  nrf_gpio_cfg_output(LCD_DISP);
  nrf_gpio_pin_write(LCD_EXTCOMIN,1);
  nrf_gpio_cfg_output(LCD_EXTCOMIN);

  nrf_gpio_pin_write(LCD_BL,1);
  nrf_gpio_cfg_output(LCD_BL);
}




void lcd_char(int x1, int y1, char ch) {
  // char replacements so we don't waste font space
  if (ch=='.') ch='\\';
  if (ch=='+') ch='@';
  if (ch>='a') ch-='a'-'A';
  int idx = ch - '0';
  if (idx<0 || idx>=LCD_FONT_3X5_CHARS) return; // no char for this - just return
  int cidx = idx % 5; // character index
  idx -= cidx;
  int y;
  for (y=0;y<5;y++) {
    unsigned short line = LCD_FONT_3X5[idx + y] >> (cidx*3);
    if (line&4) lcd_set_pixel(x1+0, y+y1, cursor_col);
    if (line&2) lcd_set_pixel(x1+1, y+y1, cursor_col);
    if (line&1) lcd_set_pixel(x1+2, y+y1, cursor_col);
  }
}

void lcd_print_char(char ch) {
  lcd_char(cursor_x,cursor_y,ch);
  if ('\n'==ch) {
    if (cursor_y <= LCD_HEIGHT-12) {
      cursor_y += 8;
    } else {
      lcd_clear(0); // if at end, just clear screen and restart
      // TODO: could scroll
      cursor_y = LCD_CURSOR_Y_START;
    }
  } else if ('\r'==ch) {
    cursor_x = LCD_CURSOR_X_START;
  } else cursor_x += 4;
}

void lcd_print(char *ch) {
  while (*ch)
    lcd_print_char(*(ch++));
}
void lcd_println(char *ch) {
  lcd_print(ch);
  lcd_print("\r\n");
  lcd_flip();
}
void lcd_clear(uint8_t col) {
  col = col&7;
  for (int y=0;y<LCD_HEIGHT;y++)
    memset(&lcdBuffer[(y*LCD_STRIDE) + LCD_ROWHEADER], col | (col<<4), LCD_STRIDE-LCD_ROWHEADER);
  lcd_flip();
}
void lcd_fill_rect(int x1, int y1, int x2, int y2, uint8_t col) {
  for (int y=y1;y<y2;y++)
    for (int x=x1;x<x2;x++)
      lcd_set_pixel(x,y,col);
}