#ifndef	BUTTONS_h
#define	BUTTONS_h

#include <avr/io.h>

//����, � �������� ���������� ������
#define PORT_BUTTON 	PORTC
#define PIN_BUTTON 	    PINC
#define DDRX_BUTTON 	DDRC

//������ �������, � ������� ���������� ������
#define DOWN 		3
#define CANCEL 		2
#define UP		    1
#define ENTER 		0

//����, ������� ����� ������������ � �����
#define KEY_NULL      0
#define KEY_ENTER     1
#define KEY_CANCEL    3
#define KEY_UP        2
#define KEY_DOWN      4

//������� ������ ������ ������ ������ ������������ - �������� �������
#define THRESHOLD 3
//������� ������ ������ ������ ������ ������������ - ������������ ������� ������� (� � ����� ������������ �������� - ����� ���� � ������� ���������� �����)
#define THRESHOLD2 12
/**************************************************************************
*   Function name : BUT_Init
*   Returns :       ���
*   Parameters :    ���
*   Purpose :       ������������� ������ �����/������
*                   ���������� ������ � ������ main`a
****************************************************************************/
void BUT_Init(void);

/**************************************************************************
*   Function name : BUT_Debrief
*   Returns :       ���
*   Parameters :    ���
*   Purpose :       ���������� ������. ���������� ������ �� ����������
*                   ���� ������ ������ � ������� 20 ����������,
*                   �� ����� ������������ � �����
****************************************************************************/
void BUT_Debrief(void);

/**************************************************************************
*   Function name : BUT_GetKey
*   Returns :       ����� ������� ������
*   Parameters :    ���
*   Purpose :       ���������� ���������� ���������� ������
*                   ��� ���� ����� ���������
*                   ���������� ������ � main`e � ����� while
*
****************************************************************************/
unsigned char BUT_GetKey(void);


/**************************************************************************
*   Function name : BUT_SetKey
*   Returns :       ���
*   Parameters :    ����� ������
*   Purpose :       ���������� � ��������� ����� ��������
*                   ��������� ������ ��� �������� ������� ������
****************************************************************************/
void BUT_SetKey(unsigned char key);

#endif //BUTTONS_H
