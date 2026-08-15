/*
 * ProjectLocoHC.hpp
 *
 *  Created on: Aug 17, 2025
 *      Author: Jon Freeman  B Eng (Hons) MIET
 *
 *	****	June 2026	****
 *	Changed SPI from Transmit Only Master to Full Duplex Master. CubeMX added PA11 as SPI1_MISO
 *	This is for future use of SD card holder on display.
 *	No problem observed. Will need to look at DMA for reading, also attn to card speed.
 *	Added DMA for card read, seems alright
 *	Added FATS middleware as advised by "https://controllerstech.com/interface-sd-card-with-stm32-via-spi-dma/", no probs observed
 *
 *	Best SD Card advice from - https://deepbluembedded.com/stm32-sd-card-spi-fatfs-tutorial-examples/
 */

#ifndef INC_PROJECTLOCOHC_HPP_
#define INC_PROJECTLOCOHC_HPP_

/*
 * Loco will be in one of the following Loco_State(s)
 *
 * Revisit July 2026	-	Connection status needs to be apart from these. New bool 'Bluetooth_connected' defined in main.c
 */
enum	class	LSClass	:	const int	{Power_On
	, Startup
	, Start_Search
	, Searching
	, Connecting
	, In_Setup_Menu
	, Run_Loco
	, Reverse
	, Forward
	, Drift
	, Park
	, BT_Connected
	, BT_Dropped
}	;
//LSClass	Loco_State = LSClass::No_Connection;	//	Power-on default

#define	SPI_PRSC_LCD	2
#define	SPI_PRSC_SD		2



#endif /* INC_PROJECTLOCOHC_HPP_ */
