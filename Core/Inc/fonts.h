/**
  ******************************************************************************
  * @file    fonts.h
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    18-February-2014
  * @brief   Header for fonts.c file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
  *
   ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FONTS_H
#define __FONTS_H

//#define MAX_HEIGHT_FONT         100	//41
#define MAX_WIDTH_FONT          100	//32
#define OFFSET_BITMAP           

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

//ASCII
typedef struct _tFont
{    
  const uint8_t *table;
  uint16_t Width;
  uint16_t Height;
  uint32_t Tabsize;
  
} sFONT;

/*
//GB2312
typedef struct                                          // ������ģ���ݽṹ
{
  unsigned char index[2];                               // ������������
  const char matrix[MAX_HEIGHT_FONT*MAX_WIDTH_FONT/8];  // ����������
}CH_CN;


typedef struct
{    
  const CH_CN *table;
  uint16_t size;
  uint16_t ASCII_Width;
  uint16_t Width;
  uint16_t Height;
  
}cFONT;
*/


//extern sFONT Arial_Narrow8x12;
//extern sFONT Arial_Narrow10x13;
//extern sFONT Arial_Narrow11x15;
//extern sFONT Arial_Narrow12x16;
//extern sFONT Arial_Narrow15x19;
//extern sFONT Arial_Narrow18x21;
//extern sFONT Arial_Narrow20x24;

//extern sFONT Arial_Narrow23x28;
//extern sFONT Arial_Narrow26x32;
//extern sFONT Arial_Narrow28x35;
//extern sFONT Arial_Narrow30x37;
//extern sFONT Arial_Narrow38x48;
//extern sFONT Arial_Narrow50x64;
//extern sFONT Arial_Narrow50x64_digits;

//extern sFONT Arial_Narrow_Bold9x11;
//extern sFONT Arial_Narrow_Bold10x12;
//extern sFONT Arial_Narrow_Bold11x13;
extern sFONT Arial_Narrow_Bold12x15;
//extern sFONT Arial_Narrow_Bold13x15;
extern sFONT Arial_Narrow_Bold15x19;
//extern sFONT Arial_Narrow_Bold17x20;
extern sFONT Arial_Narrow_Bold19x23;
//extern sFONT Arial_Narrow_Bold22x25;
//extern sFONT Arial_Narrow_Bold23x29;
//extern sFONT Arial_Narrow_Bold26x31;
//extern sFONT Arial_Narrow_Bold28x34;
//extern sFONT Arial_Narrow_Bold30x35;
//extern sFONT Arial_Narrow_Bold38x47;
extern sFONT Arial_Narrow_Bold38x47_digits;
//extern sFONT Arial_Narrow_Bold51x63;

//extern sFONT BigNeu42x35;
//extern sFONT Arial36x40;
//extern sFONT Arial37x36;
//extern sFONT Font24;
//extern sFONT Font20;
//extern sFONT Font16;
//extern sFONT Font12;
extern sFONT Font8;
extern sFONT Verdana11x11;
extern sFONT Verdana12x12;
extern sFONT Verdana13x11;
extern sFONT Verdana16x16;
//extern sFONT Verdana19x19;
//extern sFONT Verdana24x24;
//extern sFONT Verdana29x29;
//extern sFONT Verdana36x35;
//extern sFONT Verdana39x32;
//extern sFONT Verdana49x50;
//extern sFONT Verdana65x66;
//extern sFONT Verdana96x99;

//extern cFONT Font12CN;
//extern cFONT Font24CN;
#ifdef __cplusplus
}
#endif
  
#endif /* __FONTS_H */
 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
