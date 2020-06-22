#include "buttons.h"

#define MASK_BUTTONS 	(1<<DOWN)|(1<<CANCEL)|(1<<UP)|(1<<ENTER)
#define ClearBit(reg, bit)       (reg) &= (~(1<<(bit)))
#define SetBit(reg, bit)          (reg) |= (1<<(bit))	
#define BitIsClear(reg, bit)    ((reg & (1<<(bit))) == 0)
#define BitIsSet(reg, bit)       ((reg & (1<<(bit))) != 0)

volatile unsigned char pressedKey = 0;
unsigned char comp = 0;

//_______________________________________
void BUT_Init(void)
{
  DDRX_BUTTON &= ~(MASK_BUTTONS); 
  PORT_BUTTON |= MASK_BUTTONS;
}

//_______________________________________
void BUT_Debrief(void)
{
unsigned char key;

  //последовательный опрос выводов мк
  if (BitIsClear(PIN_BUTTON, ENTER))     
    key = KEY_ENTER;
  else if (BitIsClear(PIN_BUTTON, CANCEL))    
    key = KEY_CANCEL;
  else if (BitIsClear(PIN_BUTTON, UP))        
    key = KEY_UP;        
  else if (BitIsClear(PIN_BUTTON, DOWN))      
    key = KEY_DOWN;
  else {
    key = KEY_NULL;
  }

  //если во временной переменной что-то есть
 if (key)
  {
    if (comp > THRESHOLD2)
    {
      comp = THRESHOLD2 - 8;   // отнимаемое число определяет как быстро будет перебираться переменная
      pressedKey = key;
      return;
    }
    else comp++;
    
    if (comp == THRESHOLD) 
    {
     pressedKey = key;
      return;
    }
  }
  else comp=0;
}

//__________________________
unsigned char BUT_GetKey(void)
{
  unsigned char key = pressedKey;
  pressedKey = KEY_NULL;
  return key;
}


//____________________________
void BUT_SetKey(unsigned char key)
{
    pressedKey = key;
}

