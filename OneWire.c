#define DEVICES_ERROR  1
#include "config.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "OneWire.h"
 
uint8_t ONE_WIRE_DQ = PINB4
 
void oneWireInit(uint8_t pin) {
  ONE_WIRE_DQ = pin;
  ONE_WIRE_PORT |= (1 << ONE_WIRE_DQ)
  ONE_WIRE_DDR |= (1 << ONE_WIRE_DQ) // выход
}
 
/*
 * сброс
 */
uint8_t reset() {
  uint8_t response
 
  // импульс сброса, минимум 480us
  ONE_WIRE_PORT &= ~(1 << ONE_WIRE_DQ)
  ONE_WIRE_DDR |= (1 << ONE_WIRE_DQ) // выход
  _delay_us(480)
 
  // Когда ONE WIRE устройство обнаруживает положительный перепад, он ждет от 15us до 60us
  ONE_WIRE_DDR &= ~(1 << ONE_WIRE_DQ) // вход
  _delay_us(60)

  // и затем передает импульс присутствия, перемещая шину в логический «0» на длительность от 60us до 240us.
  response = (ONE_WIRE_PIN & (1 << ONE_WIRE_DQ))
  _delay_us(410)
 
  // если 0, значит есть ответ от датчика, если 1 - нет
  return response
}
 
/*
 * отправить один бит
 */
void writeBit(uint8_t bit) {
  if (bit & 1) {
    cli()
    // логический «0» на 1us
    ONE_WIRE_PORT &= ~(1 << ONE_WIRE_DQ);
    ONE_WIRE_DDR |= (1 << ONE_WIRE_DQ); // выход
    _delay_us(10);
    sei();
    ONE_WIRE_DDR &= ~(1 << ONE_WIRE_DQ); // вход
    _delay_us(55);
  } else {
    cli()
    // логический «0» на 1us
    ONE_WIRE_PORT &= ~(1 << ONE_WIRE_DQ)
    ONE_WIRE_DDR |= (1 << ONE_WIRE_DQ)// выход
    _delay_us(65);
    ONE_WIRE_DDR &= ~(1 << ONE_WIRE_DQ)// вход
    sei()
    _delay_us(5);
  }
}
 
/*
 * отправить один байт
 */
void writeByte(uint8_t byte) {
  uint8_t i = 8;
  while (i--) {
    writeBit(byte & 1);
    byte >>= 1;
  }
}
 
/*
 * получить один байт
 */
uint8_t readByte() {
  uint8_t i = 8, byte = 0;
  while (i--) {
    byte >>= 1;
    byte |= (readBit() << 7);
  }
  return byte;
}
 
/*
 * получить один бит
 */
uint8_t readBit(void) {
  uint8_t bit = 0;
  cli();
  // логический «0» на 1us
  ONE_WIRE_PORT &= ~(1 << ONE_WIRE_DQ);
  ONE_WIRE_DDR |= (1 << ONE_WIRE_DQ); // вход
  _delay_us(3);
 
  // освободить линию и ждать 14us
  ONE_WIRE_DDR &= ~(1 << ONE_WIRE_DQ); // вход
  _delay_us(10);
 
  // прочитать значение
  if (ONE_WIRE_PIN & (1 << ONE_WIRE_DQ)) {
    bit = 1;
  }
 
  // ждать 45us и вернуть значение
  sei();
  _delay_us(45);
  return bit;
}
 
/*
 * читать ROM подчиненного устройства (код 64 бита)
 */
uint64_t readRoom(void) {
  uint64_t oneWireDevice;
  if(reset() == 0) {
    writeByte(CMD_READROM);
    //  код семейства
    oneWireDevice = readByte();
    // серийный номер
    oneWireDevice |= (uint16_t)readByte()<<8 | (uint32_t)readByte()<<16 | (uint32_t)readByte()<<24 | (uint64_t)readByte()<<32 | (uint64_t)readByte()<<40 | (uint64_t)readByte()<<48;
    // CRC
    oneWireDevice |= (uint64_t)readByte()<<56;
  } else {
    return 1;
  }
  return oneWireDevice;
}
 
/*
 * Команда соответствия ROM, сопровождаемая последовательностью 
 * кода ROM на 64 бита позволяет устройству управления шиной 
 * обращаться к определенному подчиненному устройству на шине.
 */
void setDevice(uint64_t rom) {
  uint8_t i = 64;
  reset();
  writeByte(CMD_MATCHROM);
  while (i--) {
    writeBit(rom & 1);
    rom >>= 1;
  }
}
 
/*
 * провеска CRC, возвращает "0", если нет ошибок
 * и не "0", если есть ошибки
 */
uint8_t crcCheck(uint64_t data8x8bit, uint8_t len) {
  uint8_t dat, crc = 0, fb, stByte = 0;
  do {
    dat = (uint8_t) (data8x8bit >> (stByte * 8));
    for (int i = 0; i < 8; i++) {  // счетчик битов в байте
      fb = crc ^ dat;
      fb &= 1;
      crc >>= 1;
      dat >>= 1;
      if (fb == 1) {
        crc ^= 0x8c; // полином
      }
    }
    stByte++;
  } while (stByte < len); // счетчик байтов в массиве
  return crc;
}
 
/*
 * поиск устройств
 */
void searchRom(uint64_t * roms, uint8_t & n) {
  uint64_t lastAddress = 0;
  uint8_t lastDiscrepancy = 0;
  uint8_t err = 0;
  uint8_t i = 0;
  do {
    do {
      lastAddress = searchNextAddress(lastAddress, lastDiscrepancy);
      if(lastAddress != DEVICES_ERROR) {
        uint8_t crc = crcCheck(lastAddress, 8);
        if (crc == 0) {
          roms[i++] = lastAddress;
          err = 0;
        } else {
          err++;
        }
      } else {
        err++;
      }
      if (err > 3) {
        return;
      }
    } while (err != 0);
  } while (lastDiscrepancy != 0 && i < n);
  n = i;
}
 
/*
 * поиск следующего подключенного устройства
 */
uint64_t searchNextAddress(uint64_t lastAddress, uint8_t & lastDiscrepancy) {
  uint8_t searchDirection = 0;
  uint64_t newAddress = 0;
  uint8_t idBitNumber = 1;
  uint8_t lastZero = 0;
  reset();
  writeByte(CMD_SEARCHROM);
 
  while (idBitNumber < 65) {
    uint8_t idBit = readBit();
    uint8_t cmpIdBit = readBit();
 
    // id_bit = cmp_id_bit = 1
    if (idBit == 1 && cmpIdBit == 1) {
      return DEVICES_ERROR;
    } else if (idBit == 0 && cmpIdBit == 0) {
      // id_bit = cmp_id_bit = 0
      if (idBitNumber == lastDiscrepancy) {
        searchDirection = 1;
      } else if (idBitNumber > lastDiscrepancy) {
        searchDirection = 0;
      } else {
        if ((uint8_t) (lastAddress >> (idBitNumber - 1)) & 1) {
          searchDirection = 1;
        } else {
          searchDirection = 0;
        }
      }
      if (searchDirection == 0) {
        lastZero = idBitNumber;
      }
    } else {
      // id_bit != cmp_id_bit
      searchDirection = idBit;
    }
    newAddress |= ((uint64_t) searchDirection) << (idBitNumber - 1);
    writeBit(searchDirection);
    idBitNumber++;
  }
  lastDiscrepancy = lastZero;
  return newAddress;
}
 
/*
 * пропустить ROM
 */
void skipRom() {
  reset();
  writeByte(CMD_SKIPROM);
}