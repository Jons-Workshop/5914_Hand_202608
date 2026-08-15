/*
 * colours.h
 *
 *  Created on: Aug 8, 2025
 *      Author: jon34
 */

#ifndef INC_COLOURS_H_
#define INC_COLOURS_H_
#include	"main.h"
// Some ready-made 16-bit ('565') color settings:
#define	BLACK	0x0000
#define	WHITE	0xFFFF
#define	RED		0xF800
#define	GREEN	0x07E0
#define	BLUE	0x001F
#define	CYAN	0x07FF
#define	MAGENTA	0xF81F
#define	YELLOW	0xFFE0
#define	ORANGE	0xFC00

//	Definitions of colours used in battery state indicators
#define	GREEN75		(((GREEN * 3) / 4) & GREEN)	//	These are about changing display colours in
#define	GREEN66 	(((GREEN * 2) / 3) & GREEN)	//	battery condition indicators to reflect
#define	GREEN50		(((GREEN * 2) / 4) & GREEN)	//	state of charge
#define	GREEN33		(((GREEN * 1) / 3) & GREEN)
#define	GREEN25		(((GREEN * 1) / 4) & GREEN)

#define	RED75 		(((RED * 3) / 4) & RED)
#define	RED66 		(((RED * 2) / 3) & RED)
#define	RED50 		(((RED * 2) / 4) & RED)
#define	RED33 		(((RED * 1) / 3) & RED)
#define	RED25 		(((RED * 1) / 4) & RED)




#endif /* INC_COLOURS_H_ */
