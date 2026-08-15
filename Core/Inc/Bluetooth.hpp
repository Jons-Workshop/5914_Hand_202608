/*
 * Bluetooth.hpp
 *
 *  Created on: 13 Jun 2026
 *      Author: jon34
 */

#ifndef INC_BLUETOOTH_HPP_
#define INC_BLUETOOTH_HPP_

/**	Two HC05 Bluetooth modules provide wireless serial link between LocoController and HandRemote
 *
 * One is 'Master', other is 'Slave'
 *
 *	To set module into 'AT' mode, press button before powerup.
 *	Default baud rate in AT mode is 38k4
 *
 *	Comms link baud rate is 9k6 default, may be other, and maybe does not have to be set same both ends
 *
 *	14th June 2026
 *	Two new Bluetooth HC-05 from PiHut, connected to serial converters - Red VCC, Black GND, White TXD, Green RXD.
 *
 *	Trying to connect @ baud 38k4 using 'coolterm' (becauase this sends string only after 'CR' hit to avoid timeout between key presses
 *	( in following using [ ] to contain responses, allows " in response more clearly
 *	First	: COM9
 *	Second	: COM10	-	opened both in coolterm windows.
 *	"AT+ROLE?"	->	[+ROLE:0]	-	both 0 = Slave.	(1 = Master, 2 = Slave-Loop)
 *	"AT+CLASS?"	->	[+CLASS:1f00]	-	what does this mean?
 *	"AT+PSWD?"	->	[+PIN:"1234"]
 *	"AT+UART?"	->	[+UART:9600,0,0]
 *		Try changing baud to 115200. Note this is not applicable to 'AT+' mode.
 *	"AT+UART:115200,1,0"
 *	has the effect
 *	"AT+UART?"	->	[+UART:115200,1,0]	-	Believe we have set baud for radio link to 115200 (baud to set on STM32 uart)
 *	"AT+CMODE?"	->	[+CMODE:1]	-	connect mode : connect any address. (+CMODE:0 connect fixed address)
 *
 *	Refer to "https://blog.zakkemble.net/getting-bluetooth-modules-talking-to-each-other/"
 *
 *	Module to be used as 'Master' requires more setup, Slave hardly any.
 *	Can be kept in 'AT mode' by tying 'EN' pin to Vcc
 *
 *	********
 *	Perhaps most useful found online - "https://pcbsync.com/hc-05-bluetooth-module-arduino/"
 *	********
 *
 *
 */



#endif /* INC_BLUETOOTH_HPP_ */
