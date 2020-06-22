


#define F_CPU 8000000L


#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <math.h>
#include <avr/eeprom.h>
#include <stdlib.h>
#include "n5110.h"
#include "i2c.h"
#include "buttons.h"


unsigned char i=0;                  // переменная кнопок 
unsigned int f=1079;                  // частота
unsigned char f1;
unsigned char f2;
unsigned char f3;
unsigned char f4;

unsigned char volume=8; // Start volume

//unsigned char station=1;
//uint8_t station_ee EEMEM ;


// =========================================================================================================================
// =========================================================================================================================
// RDA5807


#define FM_R 0b00100001
#define FM_W 0b00100000

static uint8_t writeBuf[9];
//static uint8_t read_Buf[5];








// Регистры для записи данных (настроек)

/* 0 register (02H) */
#define RDA5807_DHIZ				(1<<7) /* Audio out enable (1) / High impedance (0)*/
#define RDA5807_DMUTE				(1<<6) /* Audio unmuted (1) / muted (0) */
#define RDA5807_MONO				(0<<5) /* Mono mode (1) / stereo mode (0) */
#define RDA5807_BASS				(1<<4) /* Bass boost (1) */
#define RDA5807_RCLK_NON_CAL_MODE	(1<<3) /* RCLK always on (0) */
#define RDA5807_RCLK_DIR_IN_MODE	(1<<2) /* RCLK direct input mode (1) */
#define RDA5807_SEEKUP				(1<<1) /* Seek up (0) / down (1) */
#define RDA5807_SEEK				(1<<0) /* Stop seek (0) / start seek in SEEKUP direction (1) */

/* 1 register (02L) */
#define RDA5807_SKMODE				(1<<7) /* Stop seeking on band limit (1) */
#define RDA5807_CLK_MODE_32768		(0<<4) /* Select quartz frequency */
	#define RDA5807_CLK_MODE_12M	(1<<4)
	#define RDA5807_CLK_MODE_24M	(5<<4)
	#define RDA5807_CLK_MODE_13M	(2<<4)
	#define RDA5807_CLK_MODE_26M	(6<<4)
	#define RDA5807_CLK_MODE_19M2	(3<<4)
	#define RDA5807_CLK_MODE_38M4	(7<<4)
#define RDA5807_RDS_EN				(1<<3) /* RDS/RBDS enable (1) */
#define RDA5807_NEW_METHOD			(1<<2) /* New demodulation method (1) */
#define RDA5807_SOFT_RESET			(1<<1) /* Reset settings (1) */
#define RDA5807_ENABLE				(1<<0) /* Power on radio (1) */

/* 2 register (03H) */
/* CHAN 9..2 bits */
/*
 * BAND = 0    => Freq = Channel spacing (kHz) * CHAN + 87.0MHz
 * BAND = 1,2  => Freq = Channel spacing (kHz) * CHAN + 76.0MHz
 * BAND = 3    => Freq = Channel spacing (kHz) * CHAN + 65.0MHz
*/

/* 3 register (03L) */
/* CHAN 1.. 0 bits */
#define RDA5807_DIRECT_MODE			(1<<5) /* Direct mode (1), only used when test */
#define RDA5807_TUNE				(1<<4) /* Tune enable (1) */
#define RDA5807_BAND_US_EUROPE		(0<<2) /* 87..108 MHz */
	#define RDA5807_BAND_JAPAN		(1<<2) /* 76..97 MHz */
	#define RDA5807_BAND_WORLDWIDE	(2<<2) /* 76..108 MHz */
	#define RDA5807_BAND_EASTEUROPE	(3<<2) /* 65..76 MHz */
	#define RDA5807_SPACE_100		(0<<0) /* 100kHz step */
	#define RDA5807_SPACE_200		(1<<0) /* 200kHz step */
	#define RDA5807_SPACE_50		(2<<0) /* 50kHz step */
	#define RDA5807_SPACE_25		(3<<0) /* 25kHz step */

/* 4 register (04H) */
#define RDA5807_DE					(1<<3) /* De-emphasis 75us (0) / 50us (1) */
#define RDA5807_SOFTMUTE_EN			(1<<1) /* Softmute enable (1) 0 - disable*/
#define RDA5807_AFCD				(1<<0) /* AFC disable (1) */

/* 5 register (04L) */
/* Unused on RDA5807M, used on RDA5807FP for GPIO/I2S control */

/* 6 register (05H) */
#define RDA5807_INT_MODE			(1<<7) /* 5ms interrupt for RDSIEN on RDS ready (0) */
#define RDA5807_SEEKTH				0x0F   /* Seek SNR threshold, 4bits, default 1000=32dB */

/* 7 register (05L) */
#define RDA5807_LNA_PORT_SEL		(2<<6) /* Only for RDA5807FP 2 bit (10) to select FMIN input */
#define RDA5807_VOLUME				0x0F   /* 4 bits volume (0000 - muted, 1111 - max) */

/* 8 register (06H) */
#define RDA5807_OPEN_MODE			0x60   /* 2 bits (11) to open read-only regs for writing */
/* Other bits for RDA5807FP, for I2S control */

/* 9 register (06L) */
/* It's for RDA5807FP, for I2S control */

/* 10 register (07H) */
#define RDA5807_TH_SOFRBLEND		0x7C   /* 5 bits for noise soft blend, default 10000 */
#define RDA5807_65M_50M_MODE		(1<<1) /* For BAND=11, 50..76MHz (0) */

/* 11 register (07L) */
#define RDA5807_TH_OLD				0xFC   /* 6 bits seek treshold, valid for SKMODE=1 */
#define RDA5807_SOFTBLEND_EN		(1<<1) /* Softblend enable (1) */
#define RDA5807_FREQ_MODE			(1<<0) /* Manual freq setup (1) for 12,13 regs */

/* 12,13 registers (08H,L) */
/* Manual frequency value. Freq = 76000(87000)kHz + value of 08 word */





// Регистры для чтения

/* 0 register (0AH) */
#define RDA5807_RDSR				(1<<7) /* RDS ready (1) */
#define RDA5807_STC					(1<<6) /* Seek/tune complete (1) */
#define RDA5807_SF					(1<<5) /* Seek failure (1) */
#define RDA5807_RDSS				(1<<4) /* RDS decoder synchronized (1) */
#define RDA5807_BLK_E				(1<<3) /* (When RDS enabled) block E has been found (1)*/
#define RDA5807_ST					(1<<2) /* Stereo indicator (1) */
/* READCHAN 9,8 bits */

/* 1 register (0AL) */
/* READCHAN 7-0 bits */

/* 2 register (0BH) */
#define RDA5807_RSSI				0xFE   /* 7 bits of RSSI signal level*/
#define RDA5807_FM_TRUE				(1<<0) /* Current channel is a station (1) */

/* 3 register (0BL) */
#define RDA5807_FM_READY			(1<<7) /* Ready */
#define RDA5807_ABCD_E				(1<<4) /* Data block E (1) or blocks A-D (0) */
#define RDA5807_BLERA				0x0C   /* 2 bits error level in block A(RDS) or E(RBDS) */
#define RDA5807_BLERB				0x03   /* 2 bits error level in block B(RDS) or E(RBDS) */

/* 4-11 registers */
/* RDS data registers:
 * 4-5   => A,
 * 6-7   => B,
 * 8-9   => C,
 * 10-11 => D,
 * or 4-11 => E when ABCD_E = 1
 */





//  Ternopil stations in ultra short wave range
#define TernopilStation1					917  
#define TernopilStation2					1006 
#define TernopilStation3					1011 
#define TernopilStation4					1015 
#define TernopilStation5					1023  
#define TernopilStation6					1032  
#define TernopilStation7					1035 
#define TernopilStation8					1041 
#define TernopilStation9					1045 
#define TernopilStation10					1049 
#define TernopilStation11					1056 
#define TernopilStation12					1061 
#define TernopilStation13					1068 
#define TernopilStation14					1074 
#define TernopilStation15					1079 





// запись I2C
void I2C_write (void) {

	i2c_start_cond();
	i2c_send_byte(0x20);
	i2c_send_byte(writeBuf[0]);
	i2c_send_byte(writeBuf[1]);
	i2c_send_byte(writeBuf[2]);
	i2c_send_byte(writeBuf[3]);
	i2c_send_byte(writeBuf[4]);
	i2c_send_byte(writeBuf[5]);
	i2c_send_byte(writeBuf[6]);
	i2c_send_byte(writeBuf[7]);
	i2c_send_byte(writeBuf[8]);
	i2c_stop_cond(); 

}


// инициализация, конфигурация
void FM__init (void) {

	writeBuf[0] = 0b11010010;
	writeBuf[1] = 0b10001101;
	writeBuf[2] = 0;
	writeBuf[3] = 0;
	writeBuf[4] = 0b00000010;
	writeBuf[5] = 0b00000000;
	writeBuf[6] = 0b00001000;
	writeBuf[7] = 0b10001111;
	//writeBuf[8] = 0b00000000;

	I2C_write();
return;
}


// задать частоту радиостанции
// частота в формате 101.2 FM пишем как 1012 (например)
// шаг 0.1 МГц
void SetFreq (uint16_t freq) {

	uint16_t chan = (freq - 870);
	//uint16_t chan = (freq - RDA5807_FREQ_MIN) / RDA5807_CHAN_SPACING;

	writeBuf[2]  = chan >> 2;					// смаргшие 8 бит
	writeBuf[3] |= (chan << 6) | RDA5807_TUNE;

	I2C_write();

	Lcd_prints(0,5,FONT_1X,(unsigned char *)PSTR("              "));

return;
}




// Задать уровень громкости (значение от 0 до 15)
void Set_volume (uint8_t vol)
{
	writeBuf[7] &= 0b11110000;
	writeBuf[7] |= vol;
	I2C_write();
}



// запустить автопоиск (вверх, вниз = 1 или 0 соответственно)
void Auto_seek (uint8_t d) {

	writeBuf[0] |= (d<<1)|(1<<0);
	I2C_write();

}




void set_station (uint8_t st) {

if (st==1) {SetFreq(TernopilStation1);         f=TernopilStation1;}
if (st==2) {SetFreq(TernopilStation2);         f=TernopilStation2;}
if (st==3) {SetFreq(TernopilStation3);         f=TernopilStation3;}
if (st==4) {SetFreq(TernopilStation4);         f=TernopilStation4;}
if (st==5) {SetFreq(TernopilStation5);		   f=TernopilStation5;}
if (st==6) {SetFreq(TernopilStation6);         f=TernopilStation6;}
if (st==7) {SetFreq(TernopilStation7);         f=TernopilStation7;}
if (st==8) {SetFreq(TernopilStation8);         f=TernopilStation8;}
if (st==9) {SetFreq(TernopilStation9);         f=TernopilStation9;}
if (st==10) {SetFreq(TernopilStation10);       f=TernopilStation10;}
if (st==11) {SetFreq(TernopilStation11);       f=TernopilStation11;}
if (st==12) {SetFreq(TernopilStation12);	   f=TernopilStation12;}
if (st==13) {SetFreq(TernopilStation13);       f=TernopilStation13;}
if (st==14) {SetFreq(TernopilStation14);       f=TernopilStation14;}
if (st==15) {SetFreq(TernopilStation15);       f=TernopilStation15;}

//eprom_write_byte(&station_ee, st);

}





// =========================================================================================================================
// =========================================================================================================================


ISR (TIMER0_OVF_vect)               // вектор прерывания по переполнеинию таймера 0
{

BUT_Debrief ();                     // опрос 4х кнопок
Lcd_update();

}



int main (void)
{

Lcd_init();
i2c_init();
BUT_Init();
FM__init();


//инициализация таймера 0
TCCR0 |= 0b00000101;            // Включаем предделитель 1/1024
TIMSK |= 0b00000001;            // разрешаем прерывания по переполнению таймера T0.
TCNT0=0;                        // Обнуляем счётный регистр таймера T0 

//station=eeprom_read_byte(&station_ee);
//set_station(station);                
Set_volume(volume);

Lcd_clear();


sei();                          // глобально разрешить прерывания

while(1)
{


i=BUT_GetKey();
if (i==1) {f++; if (f>1080) {f=870;} SetFreq(f);}
if (i==2) {f--; if (f<870) {f=1080;} SetFreq(f);}

if (i==3){volume++; if (volume>15) {volume=15;} Set_volume(volume);}
if (i==4){volume--; if (volume<1) {volume=1;} Set_volume(volume);}
//if (i==3) {station++; if (station>15) {station=1;} set_station(station);}
//if (i==4) {station--; if (station<1) {station=15;} set_station(station);}

 
f1=f/1000;
f2=f/100-f1*10;
f3=f/10-f1*100-f2*10;
f4=f-f1*1000-f2*100-f3*10;

//Для вывода числа преобразуем его в строку при помощи встроенной 
//функции itoa(число, массив,система счисления)

char buff[20];
utoa(f1, buff, 10);
Lcd_print(1, 3, FONT_2X,(unsigned char *)buff);
utoa(f2, buff, 10);
Lcd_print(3, 3, FONT_2X,(unsigned char *)buff);
utoa(f3, buff, 10);
Lcd_print(5, 3, FONT_2X,(unsigned char *)buff);

Lcd_pixel( 43,  32,  PIXEL_ON );   // точка
Lcd_pixel( 43,  31,  PIXEL_ON );
Lcd_pixel( 43,  30,  PIXEL_ON );
Lcd_pixel( 42,  31,  PIXEL_ON );
Lcd_pixel( 44,  30,  PIXEL_ON );
Lcd_pixel( 44,  31,  PIXEL_ON );
Lcd_pixel( 44,  32,  PIXEL_ON );
Lcd_pixel( 45,  31,  PIXEL_ON );

utoa(f4, buff, 10);
Lcd_print(8, 3, FONT_2X,(unsigned char *)buff);

Lcd_prints(11, 3, FONT_1X,(unsigned char *)PSTR("FM"));



_delay_ms(40);

} // end while(1)
} // end main





















