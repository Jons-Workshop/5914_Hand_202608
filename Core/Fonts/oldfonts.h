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

/*�������΢���ź�24 (32x41) */
#define MAX_HEIGHT_FONT         41
#define MAX_WIDTH_FONT          32
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
  uint16_t Tabsize;
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
extern sFONT Arial_Narrow23x28;
extern sFONT Arial_Narrow26x32;
extern sFONT Arial_Narrow28x35;
extern sFONT Arial_Narrow30x37;
extern sFONT Arial_Narrow38x48;
extern sFONT Arial_Narrow50x64;
extern sFONT BigNeu42x35;
extern sFONT Ariel36x40;
extern sFONT Ariel37x36;
extern sFONT Font24;
extern sFONT Font20;
extern sFONT Font16;
extern sFONT Font12;
extern sFONT Font8;

//extern cFONT Font12CN;
//extern cFONT Font24CN;
#ifdef __cplusplus
}
#endif
  
#endif /* __FONTS_H */
 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
