/*
 * ForeverLoop.cpp
 *
 *  Created on: Aug 9, 2025
 * 	Author	-	Jon Freeman  B Eng Hons MIET
 *
 *	//	This is the Bluetooth Hand Controller with 320x480 Graphic Touch Screen
 *
 */
#include 	"main.h"
#include	<cstring>
#include	<cstdio>
#include	<cmath>
#include	<new>	//	//	needed to handle std::nothrow

#include	"ProjectLocoHC.hpp"
#include	"Serial.hpp"

#define	CENTRE_ZERO_KNOB_MODE	false

extern	UART_HandleTypeDef	huart1;	//	uarts used in this project
extern	UART_HandleTypeDef	huart2;	//	uarts used in this project

//Serial				pc(PC_UART, 500);		//
//Serial				bt(BT_UART, 100);		//

Serial				pc(huart1, 1000, 500);		//
Serial				bt(huart2, 400, 400);		//

//	This is the Bluetooth Hand Controller with 320x480 Graphic Touch Screen

LSClass	Loco_State 			= LSClass::Power_On;		//	Power-on default
LSClass	Loco_Direction 		= LSClass::Park;			//	When driving is 'Forward' or 'Reverse'. Other states 'Drift' and 'Park'

//char	FileName[32];	//	Each time prog runs a new .csv file is created. This holds its name.
extern	char 	FileName[32];	//	in sd_card.cpp

extern	uint32_t	touch_time;
extern	uint32_t	forever_loop_timer;
extern	uint32_t	slow_loop_timer;
extern	bool		quarter_sec;
extern	bool		ms10;
extern	bool		exti_flag;
extern	bool		Bluetooth_connected_pres;	//	In main.c
extern	bool		Bluetooth_connected_prev;	//	In main.c
extern	uint32_t	exti_cnt;
extern	float		V_Loco_Batt;
extern	float		V_HC_Batt;

extern	bool	ticked	()	;
extern	void	motor_speed_bars	(float * norm_src)	;	//	normalised (0.0-1.0) values brought in array of floats
extern	void	check_commands	()	;	//	Called from ForeverLoop

extern	void	draw_driving_screen	(LSClass, bool CZ)	;	//	CZ true centre zero
extern	void	draw_setup_screen	()	;
extern	void	loco_run_screen_setup	()	;
extern	void	DrawMPH	(float speed)	;
extern	void	adjust_knob		(uint16_t X, uint16_t Y, bool touch)	;
extern	double	read_knob	()	;
extern	void	scan_buttons	(size_t X, size_t Y, bool touch)	;
extern	void	pippy	()	;		//	Draws advancing row of '.' while searching for bluetooth connection
extern	void	activate_fr_buttons	();
extern	void	update_run_screen	();
extern	bool	start_ADC	()	;	//	Inputs
extern	double	get_supply_voltage	()	;
extern	void	seeaddata	()	;
extern	bool	adc_updates	()	;	//	Call this often
extern	void	loco_batt_display_update	(float V)	;
extern	void	hh_batt_display_update	(float V)	;
extern	uint32_t	get_rx_ping_cnt	()	;	//	{	return	(received_pings);	}
extern	void		clr_rx_ping_cnt	()	;	//	{	received_pings = 0L;	}
extern	char *	createfilename	(char * dest)	;


extern	"C"	{
	bool	touch_rx	(uint8_t * rxbuff, size_t len)	;
	bool	DrawPixel	(uint32_t X, uint32_t Y, uint16_t Colour)	;
	bool	set_spi_prsc	(uint16_t	val)	;	//	New June 2026 use 2, 4, 8, ... 256
}

extern	void SD_Card_Test(void);
extern	int	log_a_block	(char * txt)	;

bool	set_Loco_Direction	(LSClass	new_state)	{
	switch	(new_state)	{
	case	LSClass::Forward:
	case	LSClass::Reverse:
		Loco_Direction = new_state;
		break;
	default:
		break;
	}
	return	(true);
}

#define	POWER_ON	(HAL_GPIO_WritePin(PWR_HOLD_GPIO_Port, PWR_HOLD_Pin, GPIO_PIN_SET))
#define	POWER_OFF	(HAL_GPIO_WritePin(PWR_HOLD_GPIO_Port, PWR_HOLD_Pin, GPIO_PIN_RESET))
/*
void	Loco_State_Machine	()	{	//	called twice per sec MOVE THIS INTO foreverloop
	if	(!(Bluetooth_connected_pres))
		Loco_State = LSClass::Setup_Menu;
	switch	(Loco_State)	{
	case	LSClass::Power_On:
//		draw_setup_screen	();
//		Bluetooth_connected = false;	//	probably is anyway
		Loco_State = LSClass::Connecting;
		break;
	case	LSClass::Connecting:
		//	bt.when returns time_ms_of_most_recent_rx, i,e, time of 'CR' in latest message received (see Serial.hpp)
		if	(bt.when() > (HAL_GetTick() - 500))	{	//	Have had verifiable comms over Bluetooth within the most recent half-second
			bt.write	("dr0\r\n", 5);			//	Command for brakes on, power off
			pc.write	("Trans from No_Connection to Setup_Menu\r\n", 40);
			activate_fr_buttons	();
//			Loco_State = LSClass::Setup_Menu;	//	stay here until driver does something like press a button
		}
		pippy	();	//	Draws advancing row of '.' while searching for bluetooth connection
		break;
	case	LSClass::Setup_Menu:	//	stay here until driver does something like press a button
		loco_batt_display_update	(V_Loco_Batt);
		if	((Loco_Direction == LSClass::Forward) || (Loco_Direction == LSClass::Reverse))	{
			Loco_State = Loco_Direction;	//	Forward or Reverse
			draw_driving_screen	(Loco_Direction, CENTRE_ZERO_KNOB_MODE);
		}
		break;
	case	LSClass::Forward:
	case	LSClass::Reverse:
		update_run_screen	();
		break;
	default:
		break;
	}
}
*/
//CircularBuffer<uint8_t>	newrxbuff	((size_t)100)	;

////uint8_t *	graph_buffer = new (std::nothrow) uint8_t[GRAFBUFFSIZE] { 0 };	//	Allocate and initialise buffer space if possible


//constexpr	char	const	version_str[] = "Info About Project Here," __DATE__;
constexpr	char	const	version_str[] = "Bluetooth_Hand_Controller_202607, Jon Freeman B Eng (Hons) MIET, " __DATE__;
const 	char * 	get_version	()	{	//	Makes above available throughout code.
	return	(version_str);
}

class	my_ID_class	{	//	Read unique STM32 die ident	SAME as in EMC26 code
	const	uint16_t ID8 = (READ_REG(*((uint32_t *)(UID_BASE + 0))) +	//	checksum method appears more random-like
			READ_REG(*((uint32_t *)(UID_BASE + 0))) 			+
			((READ_REG(*((uint32_t *)(UID_BASE + 0)))) >> 8) 	+
			((READ_REG(*((uint32_t *)(UID_BASE + 0)))) >> 16) 	+
			((READ_REG(*((uint32_t *)(UID_BASE + 0)))) >> 24) 	+
			READ_REG(*((uint32_t *)(UID_BASE + 1))) 			+
			((READ_REG(*((uint32_t *)(UID_BASE + 1)))) >> 8) 	+
			((READ_REG(*((uint32_t *)(UID_BASE + 1)))) >> 16) 	+
			((READ_REG(*((uint32_t *)(UID_BASE + 1)))) >> 24) 	+
((READ_REG(*((uint32_t *)(UID_BASE + 2)))) >> 8) 	+
			((READ_REG(*((uint32_t *)(UID_BASE + 2)))) >> 16) 	+
						READ_REG(*((uint32_t *)(UID_BASE + 2))) 			+
			((READ_REG(*((uint32_t *)(UID_BASE + 2)))) >> 24))  ;	//	8 bit sum of 12 bytes
public:

	uint16_t	get_ID8		()	const	{		return		(0x0ff & ID8);	}

}	My_ID	;




/**	class	touch_reader_class	{
 *
 */
class	touch_reader_class	{		//	Touch screen data through i2c, touch controller informs system using EXTI
	uint8_t		i2cbf	[16] { 0 };	//	need min 16, values to read from touch controller
public:
	touch_reader_class	()	{}	;	//	empty constructor
	bool	update_due_to_interrupt	()	//	Returns true if i2c functions return HAL_OK indicating good device read
	{
		return	(touch_rx	(i2cbf, 13))	;	//	Read regs 0x00 to 0x0e
	}	;

	uint16_t	P1X			()	{	return	(((i2cbf[0x03]  & 0x0f) << 8) | i2cbf[0x04]);	}	;
	uint16_t	P1Y			()	{	return	(((i2cbf[0x05]  & 0x0f) << 8) | i2cbf[0x06]);	}	;
	uint16_t	P2X			()	{	return	(((i2cbf[0x09]  & 0x0f) << 8) | i2cbf[0x0a]);	}	;
	uint16_t	P2Y			()	{	return	(((i2cbf[0x0b]	& 0x0f) << 8) | i2cbf[0x0c]);	}	;
//	uint16_t	P1Weight	()	{	return	(i2cbf[0x07]);	}	;	//	does not work as per spec
//	uint16_t	P1Area		()	{	return	(i2cbf[0x08] >> 4);	}	;	//	does not work as per spec
//	uint16_t	P2Weight	()	{	return	(i2cbf[0x0d]);	}	;	//	does not work as per spec
//	uint16_t	P2Area		()	{	return	(i2cbf[0x0e] >> 4);	}	;	//	does not work as per spec
}	touch	;




/*
const char * atptr[] = {
//const uint8_t * atptr[] = {
		"AT\r\n\0",
		"AT+reset\r\n\0",

		"AT+name=Controller_5914\r\n\0",
//		"AT+name=Loco_5914\r\n\0",
		"AT+role=1\r\n\0",				//	0 slave, 1 master
		"AT+cmode=0\r\n\0",				//	connect mode 0 fixed address, 1 any address
//		"AT+bind=1234, 56, abcdef\r\n\0",	//	address of remote to bind to
//		"AT+bind=98D3, 31, F710E7\r\n\0",	//	address of controller to bind to
		"AT+bind=98D3, 21, F83B02\r\n\0",	//	address of remote to bind to
		"AT+UART=115200, 0, 0\r\n",
		"AT+version?\r\n\0",
		"AT+addr?\r\n\0",
		"AT+name?\r\n\0",
		"AT+rname?\r\n\0",
		"AT+role?\r\n\0",
		"AT+class?\r\n\0",
		"AT+pswd?\r\n\0",
		"AT+uart?\r\n\0",
		"AT+cmode?\r\n\0",
		"AT+bind?\r\n\0",
		"AT+state?\r\n\0",
		nullptr
};
*/

/**
 * :98D3:31:F710E7	Controller_5914
 * 98D3:21:F83B02	Loco_5914
 *
 *	cmode 1 connect any address
 *
 *
 *
 */
/**Hand Controller Commands			->		Loco_Interface Response example

*	?vb		request battery volts	->		vb 23.4					//
*	?ib		request battery current	->		ib -19.7				//
*	?s		request speed			->		s07.3					//
*	?m		request motor speed		->		m 1234 4567 2468 1967	//	list of rpm for number of motors
*	fw		command forward	nn		->		fw						//	fw forward, optional nn 0.0 to 1.0
*	re		command reverse	nn		->		re						//	re reverse, optional nn 0.0 to 1.0
*	dr		command power motors nn	->		dr						//	dr drive, nn 0.0 to 1.0
*	rb		command regen brake	nn	->		rb						//	regen brake nn = degree
*	pk		command park	nn		->		pk						//	park
*	h		command horn nn			->		h						//	n=0 horns off, n=1 horn1 on, n=2  horn2 on
*	p		command 'ping'			->		p						//	acknowledges connection
*/

/*void	bt_send_knob	()	{	//	only do this when needs be
	char	t[32];
	size_t	len = sprintf	(t, "dr%.3f\r\n", (read_knob() / 4.0) + 0.5);
	bt.write	(t, len);
}*/

//	This is the Bluetooth Hand Controller with 320x480 Graphic Touch Screen
//#ifdef	PROPER_PCB
void	power_off	()	{
	HAL_GPIO_WritePin(PWR_HOLD_GPIO_Port, PWR_HOLD_Pin, GPIO_PIN_RESET);	//	power off
	__disable_irq();
	HAL_Delay	(10000);
}
//#endif

void	bril	(uint16_t b)	{	//	Useful range 99 - 990
	TIM1->CCR1 = b;
}


#define	BRIL_MIN	(TIM1->CCR1 = 99)
#define	BRIL_MAX	(TIM1->CCR1 = 990)
#define	BRIGHT_TIME_SECS	120

extern	void	draw_clock	()	;
extern	int	new_csv_file	()	;	//	Picks up global FileName
extern	int	transmit_file	(char * fname)	;
extern	void	get_file_n	(int n)	;

//	This is the Bluetooth Hand Controller with 320x480 Graphic Touch Screen and SD card

extern "C" void	ForeverLoop	()	{	// Jumps to here from 'main.c'
	char	t[164];
	size_t	len;
	uint32_t	qtrseccnt	{ 0L }	;
	uint32_t	seconds		{ 0L }	;
	uint32_t	dimtime_sec	{ BRIGHT_TIME_SECS }	;	//	two minutes
	bool		bright_display	{ true }	;
	LSClass	state = LSClass::Power_On;
	bool	local_connected = false;

	POWER_ON;
	pc.start_rx();
	bt.start_rx();
	pc.write	("\r\n\n\n", 4);
	pc.write	(get_version(), strlen(get_version()));
	len = sprintf	(t, ", Chip ID 0x%2x\r\n", My_ID.get_ID8());
	pc.write	(t, len);

	createfilename	(FileName);	//	get e.g. "20260804-124516" - file created date and time is the filename.csv
	pc.write	(FileName, strlen(FileName));
	pc.write	(" is the new filename\r\n", 22);

	new_csv_file	();
//	transmit_file	((char*)"Log.txt");

	start_ADC	();		//	Continuous ping pong buffering. Reads battery voltage
	draw_setup_screen	()	;	//	assumes no bluetooth connection yet

	SD_Card_Test();

	get_file_n	(47);

	HAL_GPIO_WritePin(TP_RST_GPIO_Port, TP_RST_Pin, GPIO_PIN_SET);	//	Release touch screen reset

	while	(true)	{
		if	(ticked())	{
			adc_updates	();		//	check this once per millisec

			if	(quarter_sec)	{	//	four times per sec or so, check connection not dropped
				quarter_sec = false;
				qtrseccnt++;

				if	(qtrseccnt == 4)	{	//	Once per second stuff
					qtrseccnt = 0L;
					seconds = uwTick / 1000;
					draw_clock();
					if	(bright_display && (dimtime_sec < seconds))	{
						bright_display = false;
						BRIL_MIN;
					}
					if	((seconds % 3) == 0)	{	//	every three secs stuff
						V_HC_Batt = get_supply_voltage();				//	Read voltage of local hand controller battery
						hh_batt_display_update	(V_HC_Batt);
					}
				}

				if	((qtrseccnt & 1) == 0)	{	//	half sec stuff
					if	(local_connected)	{	//	if bluetooth IS connected
						bt.write	("?vis\r\n", 6);	//	ask for volts, amps, speed info
					}
				}

				//	quarter second stuff
				if	(local_connected)	{		//	quarter second stuff
					if	((bt.when() + 2500) < uwTick)	{	//	All gone quiet for 2500ms, suspect connection failed
						clr_rx_ping_cnt	();
						local_connected = false;
						draw_setup_screen	();			//	Connection not made, show Connecting Bluetooth Search  . . .
						len = sprintf	(t, "Bluetooth Connection Dropped\r\n");
						pc.write	(t, len);
						state = LSClass::BT_Dropped;
					}
				}
			}

			switch	(state)	{

			case	LSClass::Power_On:	//	When unit first switched on
				state = LSClass::Startup;
				break;

			case	LSClass::BT_Dropped:
				len = sprintf	(t, "Bluetooth Dropped, next is Startup\r\n");
				pc.write	(t, len);
				Loco_Direction = LSClass::Park;
				state = LSClass::Startup;
				break;

			case	LSClass::Startup:
				clr_rx_ping_cnt	();
				local_connected = false;
				draw_setup_screen	();
				pc.write	("Starting Up\r\n", 13);
				state = LSClass::Start_Search;
				break;

			case	LSClass::Start_Search:
				len = sprintf	(t, "Searching for Bluetooth Connection\r\n");
				pc.write	(t, len);
				state = LSClass::Searching;
				break;

			case	LSClass::Searching:	//	trying to establish bluetooth connection
				if	(get_rx_ping_cnt() > 3)	{	//	rx_ping_cnt gets incremented when "p\r" received over Bluetooth
					len = sprintf	(t, "Bluetooth Connection Established\r\n");
					pc.write	(t, len);
					local_connected = true;
					clr_rx_ping_cnt	();
					state = LSClass::BT_Connected;
				}
				else	{
					if	((uwTick % 50) == 0)
						bt.write	("p\r", 2);	//	Not connected, so issue speculative 'ping' every 50ms or so
				}
				break;

			case	LSClass::BT_Connected:
				activate_fr_buttons	();
				state = LSClass::In_Setup_Menu;
				break;

			case	LSClass::In_Setup_Menu:
				if	(Loco_Direction != LSClass::Park)	{
					state = LSClass::Run_Loco;
					draw_driving_screen(Loco_Direction, CENTRE_ZERO_KNOB_MODE);
				}
				break;

			case	LSClass::Run_Loco:
				break;

			default:
				break;
			}

			if	((uwTick % 50) == 2)	{	//	do at 20 Hz or thereabouts

				if	(exti_flag)	{				//	Got interrupt due to screen finger touch
					touch.update_due_to_interrupt();	//	returns bool
					BRIL_MAX;
					dimtime_sec = seconds + BRIGHT_TIME_SECS;
					bright_display = true;
					pc.write	("touch\r\n", 7);
				}
				adjust_knob		(touch.P1X(), touch.P1Y(), exti_flag)	;	//	update with latest touch positions if touched, auto move if not touched
				scan_buttons	(touch.P1X(), touch.P1Y(), exti_flag)	;
				exti_flag = false;
			}


			check_commands	()	;

			pc.tx_any_buffered();
			bt.tx_any_buffered();

		}	//	Endof if (ticked())
	}		//	Endof while (true)
}

