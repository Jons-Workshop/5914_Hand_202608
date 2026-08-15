/*
 * mph_bars_202507.cpp
 *
 *  Created on: Jul 14, 2025
 *      Author: Jon Freeman  B Eng Hons MIET
 *
 *
 */

#include 	"main.h"
#include	"colours.h"
#include	"fonts.h"
#include	"ProjectLocoHC.hpp"

extern	"C"	{
#include	"st7796s.h"
}

#include	<cstdio>
#include	<cstring>
#include	<cmath>		//	sin, cos sqrt
#include	"Serial.hpp"

extern	float	V_HC_Batt;
extern	float	V_Loco_Batt;
extern	float	I_Loco_Batt;
extern	float	Loco_Speed;

extern	Serial	pc;		//	Comms to pc via USB
extern	Serial	bt;		//	HC05 Bluetooth module

extern	"C"	{	bool	st7796s_set_window	(int32_t Xstart, int32_t Ystart, int32_t Xend, int32_t Yend)	;	}
extern	"C" {	void	spi_tx	(uint8_t * pData, uint16_t Size)	;}	//	DMA pixel data to lcd
extern	"C" {	uint32_t	spi_callback_cnt	()	;	}				//	for proof spi dma interrupts are working
extern	"C" {	bool	DrawBar		(uint32_t Xstart, uint32_t Ystart, uint32_t Xend, uint32_t Yend, uint16_t Colour)	;	}
extern	"C" {	bool	DrawString	(size_t const X, size_t const Y, const char * const text_string, sFONT* Font, size_t const Background_Col, size_t const Foreground_Col)	;	}
extern	"C"	{	void	LCD_Clear	(uint16_t)	;	}
extern	"C"	{	bool	fill_circle	(size_t X, size_t Y, size_t radius, size_t colour)	;	}

extern	bool		spi_tx_cplt ;
//extern	bool	set_Loco_State		(LSClass	new_state)	;
//extern	bool	set_Loco_Direction	(LSClass	new_state)	;

//	fn prototype forward declarations
void	draw_screen_commons	()	;
extern	void	destroy_all_buttons	()	;


//	Table of colours used in battery state indicators
constexpr	uint16_t 	bar_colours[11]= {RED, RED, RED + GREEN33, RED + GREEN66
		, RED + GREEN, RED + GREEN, RED66 + GREEN, RED33 + GREEN
		, GREEN, GREEN, GREEN}	;


void	draw_loco_name	()	{
	constexpr	char*	LOCO_NAME_STR = (char*)"Baby Deltic - 5914";
	//constexpr	char*	LOCO_NAME_STR = (char*)"5914 for Julie";
	//	Co-ordinates for Loco Name banner
	constexpr	uint32_t	LOCO_NAME_X		=	1;	//	Assumed always 0
	constexpr	uint32_t	LOCO_NAME_Y		=	0;	//
	constexpr	uint32_t	LOCO_NAME_W		=	LCD_WIDTH - (LOCO_NAME_X * 2);	//	Don't write past right side of display
	constexpr	uint32_t	LOCO_NAME_H		=	30;	//
	constexpr	uint16_t	LOCO_NAME_BG	=	BLACK;	//
	constexpr	uint16_t	LOCO_NAME_TXT	=	WHITE;	//
	constexpr	sFONT *		LOCO_NAME_FONT	=	&Arial_Narrow_Bold15x19;
	DrawBar	(LOCO_NAME_X, LOCO_NAME_Y, LOCO_NAME_X + LOCO_NAME_W, LOCO_NAME_Y + LOCO_NAME_H, LOCO_NAME_BG);
	DrawString	(
			  LOCO_NAME_X + (LOCO_NAME_W - (strlen(LOCO_NAME_STR) * LOCO_NAME_FONT->Width)) / 2	//	centre text
			, LOCO_NAME_Y + LOCO_NAME_H - LOCO_NAME_FONT->Height								//	text Y position
			, LOCO_NAME_STR, LOCO_NAME_FONT, LOCO_NAME_BG, LOCO_NAME_TXT)	;
}

//	speedo parameters
constexpr	uint32_t	MPH_X =	0;	//	assumed 0
constexpr	uint32_t	MPH_Y =	32;
constexpr	uint32_t	MPH_W =	240;
constexpr	uint32_t	MPH_H =	86;
constexpr	uint16_t	MPH_TEXT_COLOUR	=	WHITE;	//RED
constexpr	uint16_t	MPH_BACK_COLOUR	=	BLACK;
constexpr	uint32_t	MPH_BORDER = 4;
constexpr	uint32_t	MPH_MARGIN = 8;
constexpr	uint32_t	MPH_PADING = 18;

//	various status bars - assumes x = 0
constexpr	uint32_t	BAT_BAR_Y =	122;
constexpr	uint32_t	MOT_BAR_Y =	142;
constexpr	uint32_t	STATUS_Y =	162;

constexpr	double	PI	= 4.0 * atan(1.0);

//	Co-ordinates for hand held unit battery indicator
//#define	HH_BATT_X		125	//	coordinates for hand held unit battery state indicator
//#define	HH_BATT_Y		190
#define	HH_BATT_X		215	//	coordinates for hand held unit battery state indicator
#define	HH_BATT_Y		48
//#define	HH_BATT_W		100	//	width
#define	HH_BATT_W		90	//	width
#define	HH_BATT_H		18	//	height
#define	HH_BATT_VMIN	3.0	//	July 2026 - Using single 18650 3.7V lithium
#define	HH_BATT_VMAX	3.8

//	Co-ordinates for Locomotive battery indicator
//	July 2026 - Loco Battery is 25.6V 100AH Lithium
#define	LB_BATT_X		10	//	coordinates for loco battery state indicator
#define	LB_BATT_Y		190
#define	LB_BATT_W		100
#define	LB_BATT_H		47
#define	LB_BATT_VMIN	25.0	//	Lithium
#define	LB_BATT_VMAX	26.9	//	Lithium

#define	LCD_MARGIN_LR	5
#define	LCD_MARGIN_TB	5


//	motor speed bars
constexpr	size_t	MSB_X			= 0;	//	this will be 0 as occupies full display width
constexpr	size_t	MSB_Y			= 242;	//	top edge this many pixels down from top of display
constexpr	size_t	NUMOF_MSBARS	= 4	;	//	typically number of brushless motors, each reporting back speed
constexpr	size_t	MSBAR_HEIGHT_PX = 12;	//	height or thickness of bar display
constexpr	size_t	MSBAR_GAP_PX	= 2	;	//	gap between bar displays
constexpr	size_t	MSBAR_SPACING_PX= MSBAR_HEIGHT_PX + MSBAR_GAP_PX	;	//
constexpr	size_t	MSBAR_WIDTH 	= 240;//LCD_WIDTH - (2 * LCD_MARGIN_LR);	//	width on display, allow few pixels for margin
constexpr	size_t	MSBAR_LEN_PX 	= MSBAR_WIDTH - (2 * LCD_MARGIN_LR);	//	width on display, allow few pixels for margin
constexpr	size_t	MSBAR_YMAX		= (MSB_Y + MSBAR_GAP_PX + LCD_MARGIN_TB + (MSBAR_SPACING_PX * 5))	;






class	virtual_control_knob_class	{

#define	KNOB_X				(LCD_WIDTH / 2)
#define	KNOB_Y				380
#define	KNOB_RAD			67
#define	KNOB_DIMPLE_SIZE	6	//8
#define	KNOB_DIMPLE_TRACK_RAD		(58.0)
#define	KNOB_COLOUR			RED	//BLACK
#define	KNOB_DIMPLE_COLOUR	WHITE
#define	KNOB_SCALE_COLOUR	BLACK
#define	KNOB_SCALE_RAD		(KNOB_RAD + 10)
#define	KNOB_MAX_FINGER_TRACK_RADIUS	(KNOB_RAD * 2.5)
#define	KNOB_DIGITS_ANGLE_STEP	(0.4)		//	radian

	int32_t	dimple_x = - 1;
	int32_t	dimple_y = - 1;
	int32_t	new_x;
	int32_t	new_y;
	double	knob_radian = -2.0;	//	knob fully CCW
	bool	centre_zero	{ false }	;
	bool	in_use	{ false }	;

	void	destroy	()	{	in_use = false;	}

	void	draw_knob	()	{	//	draw in background colour when !in_use
		int32_t	digit_x;
		int32_t	digit_y;
		fill_circle	(KNOB_X, KNOB_Y, KNOB_RAD, in_use ? KNOB_COLOUR : WHITE);			//	Big black filled circle for knob, white if not in use
		const char * 	scale_text[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "Z"}	;
		const char * 	scale_text_CZ[] = {"-5", "-4", "-3", "-2", "-1", "0", "1", "2", "3", "4", "5", "Z"}	;
		const char ** 	scale = (centre_zero ? scale_text_CZ : scale_text);
		sFONT * fontptr = &Verdana13x11;
		size_t	text_index = 0;
		double	digit_angle = - 5.0 * KNOB_DIGITS_ANGLE_STEP;
		for	(int i = 1; i < 12; i++)	{
			digit_x	= KNOB_X + (int32_t)(KNOB_SCALE_RAD * sin(digit_angle));		//	calculate coordinates for scale digit
			digit_y	= KNOB_Y - (int32_t)(KNOB_SCALE_RAD * cos(digit_angle));
			if	(scale[text_index][0] == '-')
				digit_x -= fontptr->Width;
			DrawString	(digit_x - (fontptr->Width / 3), digit_y - 4, scale[text_index++], fontptr, WHITE, in_use ? KNOB_SCALE_COLOUR : WHITE);
			digit_angle += KNOB_DIGITS_ANGLE_STEP;
		}
	}

public:
	virtual_control_knob_class	()	{}	;	//	empty constructor

	void	create	(bool	czmode)	{	//	create with centre zero true, 1 to 11 continuous scale false
		in_use = true;
		centre_zero = czmode;
		draw_knob	();		//	colours determined by 'in_use' flag
	}

	bool	erase	()	{
		if	(!in_use)
			return	(false);
		destroy	();
		draw_knob	();
		return	(true);
	}

	bool	read	(double & rv)	{	//	returns false with rv not affected if knob not in use
		if	(!in_use)
			return	(false);
		if	(centre_zero)	{		//	-1.0 <= rv <= +1.0
			rv = knob_radian / 2.0;
		}
		else	{					//	0.0 <= rv <= 1.0
			rv = (knob_radian + 2.0) / 4.0;
		}
		return	(true);
	}

	bool	adjust	(uint16_t X, uint16_t Y, bool touch)	{	//	virtual_control_knob_class
		if	(!in_use)
			return	(false);
		if	(touch)	{		//	if finger is touching screen
			int	dx = (X - KNOB_X);
			int	dy = (KNOB_Y - Y);	//	strange order to get quadrants where we need them for -2.0<=theta<=2.0 radian
			int	vector_mag	= (int)sqrt((dx * dx) + (dy * dy));
			double newangle = atan2	(dx, dy);				//	calculate angle for new dimple
			if	((vector_mag < KNOB_RAD) || (vector_mag > (KNOB_MAX_FINGER_TRACK_RADIUS)) || (newangle < -2.0) || (newangle > 2.0))	//	check finger in sensible range
				return	(true);
			if	(newangle < knob_radian)		//	reducing, user demands less power or more brake. Fast response
				knob_radian = newangle;
			else	{					//	increasing. User demanding more power or less brake. Slow response
				knob_radian += (newangle - knob_radian) / 25.0;
			}
			if	(knob_radian < -2.0)	knob_radian = -2.0;						//	limit angle to +/- 2.0 radian
			if	(knob_radian > +2.0)	knob_radian = +2.0;						//	positive driving, negative braking
		}	//	endof if touch
		else	{		//	finger is not touching screen
			if	(knob_radian > (centre_zero ? 0.01 : -2.0))	knob_radian -= 0.05;	//	glide back to fail-safe knob position
		}
		//	methods above may alter knob position angle because knob was either touched or not touched
		new_x	= KNOB_X + (int32_t)(KNOB_DIMPLE_TRACK_RAD * sin(knob_radian));		//	calculate coordinates for new dimple
		new_y	= KNOB_Y - (int32_t)(KNOB_DIMPLE_TRACK_RAD * cos(knob_radian));
		if	((dimple_x != new_x) || (dimple_y != new_y))	{					//	redraw dimple and transmit new data only if moving
			fill_circle	(dimple_x, dimple_y, KNOB_DIMPLE_SIZE, KNOB_COLOUR);	//	undraw old dimple
			fill_circle	(new_x, new_y, KNOB_DIMPLE_SIZE, KNOB_DIMPLE_COLOUR);	//	draw new dimple
			dimple_x = new_x;													//	save co-ords for next time around
			dimple_y = new_y;
			char	t[20];
			size_t 	len = sprintf	(t, "dr%.3f\r\n", (knob_radian / 4.0) + 0.5);	//	convert +/-2.0 range to 0.0 to 1.0
			bt.write	(t, len);												//	Bluetooth out new knob related stuff
		}
		return	(true);
	}
}
	control_knob	;


void	adjust_knob	(uint16_t X, uint16_t Y, bool touch)	{	//	Wrapper fn isolates caller from need to know code structure detail
	control_knob.adjust	(X, Y, touch);
}


/*
void	numconvert	(float f, char * dest)	{
	*dest = '+';
	if	(f < 0.0)	{
		f = -f;
		*dest = '-';
	}
	sprintf	(dest + 1, "%3d", (int)(f * 10.0));
}


void	draw_bar_meter	(size_t Y, char * item, float V, float A)	{	//	To draw e.g. "Bat 25.6V 13.2A" across width of display.
char	t[64];
char	v[16];
char	i[16];
int		px, py;
sFONT *	Font = &Arial_Narrow_Bold15x19;
	numconvert	(V, v);
	numconvert	(A, i);
	sprintf	(t, " %s %sV %sA", item, v+1, i);	//	v + 1 skips over the '+' or '-'
	DrawString	(0, Y, t, Font, WHITE, BLACK);
	px = (8 * Font->Width) - 5;					//	decimal point x
	py = Y + (Font->Height * 2) / 3;			//	decimal point y
	DrawBar	(px, py, px + 2, py + 2, BLACK);	//	draw decimal point in voltage
	px += 6 * Font->Width;
	DrawBar	(px, py, px + 2, py + 2, BLACK);	//	draw decimal point in current
}
*/
/*
void	draw_status	(uint32_t status)	{
	sFONT *	Font = &Arial_Narrow_Bold15x19;
	const	char	* statustab [] = {
			"Driving Fwd    "
		,	"Driving Rev    "
		,	"Drift - Fwd    "
		,	"Drift - Rev    "
		,	"Regen Braking  "
		,	"STOPPED        "
		,	"Parking Brake  "
		,	"STATUS OOR!!   "
	}	;
	constexpr	uint32_t	tablen = sizeof(statustab) / sizeof(char*);
	if	(status >= tablen)
		status = tablen - 1;
	char	t[64];
	sprintf(t, "%s", statustab[status]);
	DrawString	(15, STATUS_Y, t, Font, WHITE, BLACK);
}
*/


/**
 * class	generic_graphic_indicator	{
 *
 */
/*class	generic_graphic_indicator	{
	const	size_t	x;
	const	size_t	y;			//	x and y define top left
	const	size_t	width;
	const	size_t	height;
	const	size_t	line_weight	{ 1 }	;
	const	char *	txt;
public:
	generic_graphic_indicator	(
			const size_t topx
		, 	const size_t topy
		, 	const size_t wid
		, 	const size_t hgt
		, 	const char * text	//	e.g. "Battery    V" leaving space to overwrite value
	)
	: 		x 		{topx}
		, 	y 		{topy}
		, 	width 	{wid}
		, 	height 	{hgt}
		,	txt		{text}
	{}	;

	void	setup	()	{	//	draw the generic_graphic_indicator object with default value (0?)
		DrawBar	(x, y, x + width, y + height, MAGENTA);
		DrawString (x + 4, y + 4, txt, &Arial_Narrow_Bold12x15,    WHITE,  RED);
	}	;

	void	update	(char *	t)	{	//	write new text value, e.g. "Fwd ", "Drive", "Drift", "Regen", "Brake", "Park "

	}

	void	update	(float	v)	{	//	write new numeric value, e.g. "25.7"
		char	t[24];
		sprintf	(t, "%.1f", v);
		update	(t);
	}
}	;
*/


class	graphic_Loco_battery_indicator	{
	bool	in_use	{ false }	;
	const	size_t		line_weight	{ 1 }	;
	const	uint32_t	Colour	{ BLACK }	;
/*	void	szetup	()	{
		DrawBar	(LB_BATT_X, LB_BATT_Y, LB_BATT_X + LB_BATT_W, LB_BATT_Y + line_weight, Colour);										//	Battery box top long horizontal
		DrawBar	(LB_BATT_X, LB_BATT_Y + LB_BATT_H, LB_BATT_X + LB_BATT_W, LB_BATT_Y + LB_BATT_H + line_weight, Colour);				//	bottom long horizontal
		DrawBar	(LB_BATT_X, LB_BATT_Y, LB_BATT_X + line_weight, LB_BATT_Y + LB_BATT_H, Colour);										//	left vertical
		DrawBar	(LB_BATT_X + LB_BATT_W, LB_BATT_Y, LB_BATT_X + line_weight + LB_BATT_W, LB_BATT_Y + LB_BATT_H, Colour);				//	right vertical
		DrawBar	(LB_BATT_X + (LB_BATT_W / 8) - 4, LB_BATT_Y - 8, LB_BATT_X + line_weight + (LB_BATT_W / 8) + 4, LB_BATT_Y, Colour);	//	draw left battery terminal
		DrawBar	(LB_BATT_X + ((LB_BATT_W * 7) / 8) - 4, LB_BATT_Y - 8, LB_BATT_X + line_weight + ((LB_BATT_W * 7) / 8) + 4, LB_BATT_Y, Colour);	//	draw right battery terminal
		DrawString (LB_BATT_X + 4, LB_BATT_Y + 6, "Unknown", &Arial_Narrow_Bold12x15,    WHITE,  BLACK);							//	Probably don't know voltage at this time
	}
*/
public:
	graphic_Loco_battery_indicator	()	{}	//	Constructor

	void	update	(float V)	{					//	Loco battery state indicator. V to contain real voltage e.g. 24.73
		if	(!in_use)	{
			in_use = true;
//			szetup	();
			DrawBar	(LB_BATT_X, LB_BATT_Y, LB_BATT_X + LB_BATT_W, LB_BATT_Y + line_weight, Colour);										//	Battery box top long horizontal
			DrawBar	(LB_BATT_X, LB_BATT_Y + LB_BATT_H, LB_BATT_X + LB_BATT_W, LB_BATT_Y + LB_BATT_H + line_weight, Colour);				//	bottom long horizontal
			DrawBar	(LB_BATT_X, LB_BATT_Y, LB_BATT_X + line_weight, LB_BATT_Y + LB_BATT_H, Colour);										//	left vertical
			DrawBar	(LB_BATT_X + LB_BATT_W, LB_BATT_Y, LB_BATT_X + line_weight + LB_BATT_W, LB_BATT_Y + LB_BATT_H, Colour);				//	right vertical
			DrawBar	(LB_BATT_X + (LB_BATT_W / 8) - 4, LB_BATT_Y - 8, LB_BATT_X + line_weight + (LB_BATT_W / 8) + 4, LB_BATT_Y, Colour);	//	draw left battery terminal
			DrawBar	(LB_BATT_X + ((LB_BATT_W * 7) / 8) - 4, LB_BATT_Y - 8, LB_BATT_X + line_weight + ((LB_BATT_W * 7) / 8) + 4, LB_BATT_Y, Colour);	//	draw right battery terminal
			DrawString (LB_BATT_X + 4, LB_BATT_Y + 6, "Unknown", &Arial_Narrow_Bold12x15,    WHITE,  BLACK);							//	Probably don't know voltage at this time
		}
		uint16_t	barbuff[LB_BATT_W + 2];				//	store for one lines worth of colour data for writing via spi
		uint16_t	bar_colour;					//	to be determined by battery voltage
		uint32_t	line_count = LB_BATT_H - 6;	//	numof horiz lines used to draw charge state brick
		uint32_t	active_lines = (line_count * 4) / 5;	//	leave visible solid red bar when bat too low to use
		size_t		switch_at = 0;				//	case for fully charged
		char	t[32];
		float	norm_v = (V - LB_BATT_VMIN) / (LB_BATT_VMAX - LB_BATT_VMIN);
		if	(norm_v < 0.0)	norm_v = 0.0;
		if	(norm_v > 1.0)	norm_v = 1.0;		//	normalised 0.0 <= X <= 1.0
		bar_colour = bar_colours[(int)(norm_v * 10.0)];
		switch_at = (size_t)((1.0 - norm_v) * active_lines);
		for	(uint32_t i = 0; i < LB_BATT_W; i++)	{
			barbuff[i] = (uint16_t)((WHITE >> 8) | (WHITE << 8));	//	space available to take battery charge shown a white space
		}
		st7796s_set_window	(LB_BATT_X + 4, LB_BATT_Y + 4, LB_BATT_X + LB_BATT_W - 3, LB_BATT_Y + LB_BATT_H - 2);
		uint32_t	line_len = LB_BATT_W - 8;
		for	(uint32_t i = 0; i < line_count; i++)	{
			if	(i == switch_at)	{					//	change colour
				for	(uint32_t i = 0; i < LB_BATT_W; i++){	//	space representing remaining battery charge shown in a colour to suit state
					barbuff[i] = (uint16_t)((bar_colour >> 8) | (bar_colour << 8));
				}
			}
			spi_tx	((uint8_t*)barbuff, (2 + line_len) * 2);	//	draw 1 pixel high line across battery box
		}
		sprintf	(t, "%.1fV", V);
		DrawString (LB_BATT_X + 24, LB_BATT_Y - 15, t, &Arial_Narrow_Bold12x15,    WHITE,  BLACK);
		if	(norm_v < 0.20)
//			DrawString (LB_BATT_X + 4, LB_BATT_Y + 6, "STOP!", &Arial_Narrow_Bold19x23,    WHITE,  RED);
			DrawString (LB_BATT_X + 4, LB_BATT_Y + 6, "STOP!", &Arial_Narrow_Bold12x15,    WHITE,  RED);
		else	if	(norm_v < 0.30)
			DrawString (LB_BATT_X + 4, LB_BATT_Y + 6, "LowBatt", &Arial_Narrow_Bold12x15,    WHITE,  RED);
	}		//	endof void bar (float V)


	void	destroy	()	{
		in_use = false;
	}

}	;


//		graphic_AA_battery_indicator	bi	(X,  Y,   Wid, Ht, min, max);
class	graphic_AA_battery_indicator	{
	bool	in_use	{ false }	;
	const	size_t	line_weight	{ 1 }	;
	const	uint32_t	Colour	{ BLACK }	;
	const	uint32_t	pole_x	{ (HH_BATT_W * 105) / 100 }	;
	const	uint16_t 	bar_colours[12]= {RED, RED, RED + GREEN33, RED + GREEN66
			, RED + GREEN, RED + GREEN, RED66 + GREEN, RED33 + GREEN
			, GREEN, GREEN, GREEN, GREEN}	;
/*	void	setup	()	{
		DrawBar	(HH_BATT_X, HH_BATT_Y, HH_BATT_X + HH_BATT_W, HH_BATT_Y + line_weight, Colour);														//	top long h
		DrawBar	(HH_BATT_X, HH_BATT_Y + HH_BATT_H, HH_BATT_X + HH_BATT_W, HH_BATT_Y + HH_BATT_H + line_weight, Colour);								//	bottom long h
		DrawBar	(HH_BATT_X, HH_BATT_Y, HH_BATT_X + line_weight, HH_BATT_Y + HH_BATT_H, Colour);														//	left v
		DrawBar	(HH_BATT_X + HH_BATT_W, HH_BATT_Y, HH_BATT_X + line_weight + HH_BATT_W, HH_BATT_Y + (HH_BATT_H / 3), Colour);						//	tit top v
		DrawBar	(HH_BATT_X + HH_BATT_W, HH_BATT_Y + (HH_BATT_H *2 / 3), HH_BATT_X + line_weight + HH_BATT_W, HH_BATT_Y + HH_BATT_H, Colour);		//	tit bottom v
		DrawBar	(HH_BATT_X + pole_x, HH_BATT_Y + (HH_BATT_H *1 / 3), HH_BATT_X + line_weight + pole_x, HH_BATT_Y + (HH_BATT_H *2 / 3), Colour);		//	tit surface v
		DrawBar	(HH_BATT_X + HH_BATT_W, HH_BATT_Y + (HH_BATT_H *1 / 3), HH_BATT_X + pole_x, HH_BATT_Y + line_weight + (HH_BATT_H *1 / 3), Colour);	//	tit top h
		DrawBar	(HH_BATT_X + HH_BATT_W, HH_BATT_Y + (HH_BATT_H *2 / 3), HH_BATT_X + pole_x, HH_BATT_Y + line_weight + (HH_BATT_H *2 / 3), Colour);	//	tit bottom h
	}
*/
public:
//	CircularBuffer	(const size_t s) : buffsize { s }	{
	graphic_AA_battery_indicator	()	{}	;	//	Constructor

	void	display_update	(float V)	{	//	bars 0 to 9. 0 always, 9 when V > V_Max
		char	t[32];
		float	norm_v = (V - HH_BATT_VMIN) / (HH_BATT_VMAX - HH_BATT_VMIN);
		if	(!in_use)	{
			in_use = true;
//			setup	();
			DrawBar	(HH_BATT_X, HH_BATT_Y, HH_BATT_X + HH_BATT_W, HH_BATT_Y + line_weight, Colour);														//	top long h
			DrawBar	(HH_BATT_X, HH_BATT_Y + HH_BATT_H, HH_BATT_X + HH_BATT_W, HH_BATT_Y + HH_BATT_H + line_weight, Colour);								//	bottom long h
			DrawBar	(HH_BATT_X, HH_BATT_Y, HH_BATT_X + line_weight, HH_BATT_Y + HH_BATT_H, Colour);														//	left v
			DrawBar	(HH_BATT_X + HH_BATT_W, HH_BATT_Y, HH_BATT_X + line_weight + HH_BATT_W, HH_BATT_Y + (HH_BATT_H / 3), Colour);						//	tit top v
			DrawBar	(HH_BATT_X + HH_BATT_W, HH_BATT_Y + (HH_BATT_H *2 / 3), HH_BATT_X + line_weight + HH_BATT_W, HH_BATT_Y + HH_BATT_H, Colour);		//	tit bottom v
			DrawBar	(HH_BATT_X + pole_x, HH_BATT_Y + (HH_BATT_H *1 / 3), HH_BATT_X + line_weight + pole_x, HH_BATT_Y + (HH_BATT_H *2 / 3), Colour);		//	tit surface v
			DrawBar	(HH_BATT_X + HH_BATT_W, HH_BATT_Y + (HH_BATT_H *1 / 3), HH_BATT_X + pole_x, HH_BATT_Y + line_weight + (HH_BATT_H *1 / 3), Colour);	//	tit top h
			DrawBar	(HH_BATT_X + HH_BATT_W, HH_BATT_Y + (HH_BATT_H *2 / 3), HH_BATT_X + pole_x, HH_BATT_Y + line_weight + (HH_BATT_H *2 / 3), Colour);	//	tit bottom h
		}
		if	(norm_v < 0.0)	norm_v = 0.0;
		if	(norm_v > 1.0)	norm_v = 1.0;
		int	numofbars = (int)(norm_v * 9.0);	//	BARS 0 TO 9
		uint16_t	box_colr;// = RED >> 8 | RED << 8;		//	display boxes 6px wide at 10px intervals
		uint16_t	gbuff[HH_BATT_W+4] ;
		for	(size_t q = 0; q < HH_BATT_W - 2; q++)	//	1 px high white line across width
			gbuff[q] = WHITE;
		uint16_t *	gbptr = gbuff;					//	Write 1 px high bar colour short lines over this
		for	(int i = 0; i <= numofbars; i++)	{	//	bar chart elements
			box_colr = (uint16_t)((bar_colours[i] >> 8) | (bar_colours[i] << 8));
			for	(int j = 0; j < 6; j++)			//
				*gbptr++ = box_colr;			//	streak of 6 px wide box colours written to output line
//			gbptr += 4;	//	hop over white space
			gbptr += 3;	//	hop over white space
		}	//	1 px high pattern of bar colours.
			//	Written one line of bar chart colours to gbuff
		//		Now write this into display window 'num of lines high' times
		st7796s_set_window	(HH_BATT_X+2 , HH_BATT_Y + 5, HH_BATT_X+2 + HH_BATT_W - 1, HH_BATT_Y + HH_BATT_H - 4);
		for	(int i = 0; i < 19; i++)
			spi_tx	((uint8_t*)gbuff, HH_BATT_W * 2);
		//	Done drawing battery health bar graphic
		sprintf	(t, "%.2fV", V);
		DrawString (HH_BATT_X + 24, HH_BATT_Y - 15, t, &Arial_Narrow_Bold12x15,    WHITE,  BLACK);
//		HAL_Delay(0);	//	Is beneficial here, prevents last line getting corrupted due to early demise of gbuff before dma completion
	}	//	endof update

	void	destroy	()	{
		in_use = false;
	}

}	;	//		endof class graphic_AA_battery_indicator	bi	(X,  Y,   Wid, Ht, min, max);



class	motors_speeds_bars_box	{
	sFONT *	Font;
	const	size_t	bar_colour_on_sw;
	const	size_t	bar_colour_off_sw;
public:
	motors_speeds_bars_box	(	//	Constructor
				sFONT*	BarsFont
			,	const size_t on_colour
			,	const size_t off_colour
			)
		:
				Font	{BarsFont}
			,	bar_colour_on_sw	{(on_colour >> 8) | (on_colour << 8)}
			,	bar_colour_off_sw	{(off_colour >> 8) | (off_colour << 8)}
			{	//	Constructor - nothing useful to do

	}	;	//	endof constructor

//	motors_speeds_bars_box			mot_speed_bars	(&Arial_Narrow_Bold12x15, GREEN,  BLACK);

	void	setup	()	{
		DrawBar	(MSB_X, MSB_Y, MSBAR_WIDTH, MSB_Y + 78, YELLOW);
		DrawString	(MSB_X + 30, MSB_Y + 2, "  Motor Speeds  ", Font, YELLOW, BLACK);
	}

	void	update	(float * speeds)	{	//	bars 0 to 9. 0 always, 9 when V > V_Max
		constexpr	size_t		pixels_margin	= 2	;	//	allow tiny space for 'end stop pins'
		float	f_temp;
		static	uint16_t	barbuff[MSBAR_LEN_PX + 2];		//	buffer to get filled and passed to dma. static prevents loss on return from fn
		size_t	ypos = MSB_Y + MSBAR_SPACING_PX + LCD_MARGIN_TB;			//	Y coord of top edge of top bar
		for	(size_t barnum = 0; barnum < NUMOF_MSBARS; barnum++)	{	//	do for number of motors / bars
			f_temp = speeds[barnum];			//	fetch value for this bar
			if	(f_temp < 0.0)	f_temp = 0.0;	//	Check 0.0 <= input <= 1.0
			if	(f_temp > 1.0)	f_temp = 1.0;	//	clip to limits if need be
			size_t	bar_change_at_x = pixels_margin + (size_t)(f_temp * (MSBAR_WIDTH - 10));	//
			size_t	xpos = 0;
			for	(	; xpos < bar_change_at_x; xpos++)
				barbuff[xpos] = bar_colour_on_sw;		//	write colour in buff for first (left) part of bar
			for	(	; xpos < MSBAR_LEN_PX - 2; xpos++)
				barbuff[xpos] = bar_colour_off_sw;		//	write colour in  buff for second (right) part of bar

			st7796s_set_window	(LCD_MARGIN_LR, ypos, LCD_MARGIN_LR + MSBAR_LEN_PX - 1, ypos + MSBAR_HEIGHT_PX);
			for	(size_t i = 0; i < MSBAR_HEIGHT_PX; i++)	{						//	draw same info for number of vertical pixels of bar height
				spi_tx	((uint8_t*)barbuff, MSBAR_LEN_PX * 2);
				ypos++;
			}
			ypos += MSBAR_GAP_PX;
		}		//	endof for numof motors / bars
	}			//	endof update

}	;


class	draw_mph	{
public:
	draw_mph	()	{}	;
	void	setup	()	{	//	green, cyan, green, black
//		bool	DrawBar	(uint32_t Xstart, uint32_t Ystart, uint32_t Xend, uint32_t Yend, uint16_t Colour)
		DrawBar	(MPH_X, MPH_Y, MPH_W, MPH_Y + MPH_H, GREEN);		//	redraw decimal point
		DrawBar	(MPH_X + MPH_BORDER, MPH_Y + MPH_BORDER, MPH_W - (1 * MPH_BORDER), MPH_Y + MPH_H - (1 * MPH_BORDER), CYAN);		//	redraw decimal point
		DrawBar	(MPH_X + MPH_MARGIN, MPH_Y + MPH_MARGIN, MPH_W - (1 * MPH_MARGIN), MPH_Y + MPH_H - (1 * MPH_MARGIN), 0x4e0);		//	redraw decimal point
		DrawBar	(MPH_X + MPH_PADING, MPH_Y + MPH_PADING, MPH_W - (1 * MPH_PADING), MPH_Y + MPH_H - (1 * MPH_PADING), BLACK);		//	redraw decimal point
		DrawString (MPH_X + MPH_PADING + 4, MPH_Y + MPH_PADING + 4, "Speed",	&Arial_Narrow_Bold15x19,    MPH_BACK_COLOUR,  MPH_TEXT_COLOUR);
		DrawString (MPH_X + MPH_PADING + 4, MPH_Y + MPH_PADING + 26, "M.P.H",	&Arial_Narrow_Bold15x19,    MPH_BACK_COLOUR,  MPH_TEXT_COLOUR);
	}
	void	update	(float V)	;

}	;

graphic_AA_battery_indicator	HH_Bat	;	//	For battery in hand held unit
graphic_Loco_battery_indicator	Loco_Bat	;	//	For loco traction batteries
motors_speeds_bars_box			mot_speed_bars	(&Arial_Narrow_Bold12x15, GREEN,  BLACK);
//generic_graphic_indicator		mycrap	(114, 212, 124, 25, "Fwd-Driven");
draw_mph						My_MPH	;


void	update_run_screen	()	{
	HH_Bat.display_update	(V_HC_Batt);
	Loco_Bat.update	(V_Loco_Batt);
//	draw_bar_meter	(BAT_BAR_Y, (char*)"Loco", V_Loco_Batt, -0.5);
//	draw_bar_meter	(MOT_BAR_Y, (char*)"Mot ", 13.2, speed - 10.5);
}


void	draw_mph::update	(float speed)	{
#define	MPH_DP_X	(MPH_X + 172)
#define	MPH_DP_Y	(MPH_Y + 50)
//	sFONT * Font = &Arial_Narrow50x64;
	char	t[16];
	if	(speed < 0.0)		speed = 0.0;
	if	(speed > 99.0)		speed = 99.0;
	HH_Bat.display_update	(V_HC_Batt);
	Loco_Bat.update	(V_Loco_Batt);
//	draw_bar_meter	(BAT_BAR_Y, (char*)"Bat ", V_Loco_Batt, -0.5);
//	draw_bar_meter	(MOT_BAR_Y, (char*)"Mot ", 13.2, speed - 10.5);
	uint16_t	uintspeed = (uint16_t)(Loco_Speed * 10.0);
	sprintf	(t, "%3d", uintspeed);
	DrawString (MPH_X + MPH_PADING + 90, MPH_Y + MPH_PADING + 4, t, &Arial_Narrow_Bold38x47_digits,    MPH_BACK_COLOUR,  MPH_TEXT_COLOUR);
	DrawBar	(MPH_DP_X, MPH_DP_Y, MPH_DP_X + 6, MPH_DP_Y + 6, MPH_TEXT_COLOUR);		//	redraw decimal point
//	draw_status	(statustest++);
//	if	(statustest > 8)
//		statustest = 0;
}


void	DrawMPH	(float speed)	{
	My_MPH.update(speed);
}


void	loco_run_screen_setup	()	{	//	called once from startup
	draw_screen_commons	();
	mot_speed_bars.setup();
	HH_Bat.display_update	(0.0);		//	First call initiates setup
	Loco_Bat.update	(0.0);		//	First call initiates setup
	My_MPH.setup();
}


void	motor_speed_bars	(float * norm_src)	{
	mot_speed_bars.update	(norm_src);
}



class	bluetooth_connecting_class	{
	int	pips = 0;
	bool	connected_flag	{ false }	;
public:
	bluetooth_connecting_class	()	{}		//	empty constructor
	void	update_pips	()	{
		char	t[32] = { ".       \0" }	;
		if	(++pips > 7)
			pips = 1;
		for	(int i = 0; i < pips; i++)
			t[i] = '.';
//		DrawString (134, 100, t,	&Arial_Narrow_Bold19x23,    WHITE, BLUE);
		DrawString (134, 100, t,	&Arial_Narrow_Bold15x19,    WHITE, BLUE);
//		bt.write	("p\r\n", 3);	//	Issue a 'p' for ping
	}

	void	set_connected	(bool con)	{
		connected_flag = con;
	}

	bool	get_connected	()	{
		return	(connected_flag);
	}
}
	bt_connecting	;


	void	pippy	()	{	//	Call to this wrapper function from ForeverLoop twice/sec while not connected
	bt_connecting.update_pips();	//	repeatedly draws line of '.' to give impression something is happening
}

extern	char * get_time	(char * dest)	;	//	get e.g. "16:50:11"
extern	int	log_a_block	(const char * local_filename, const char * txt)	;
extern	int	bollocks	(const char * local_filename, const char * txt, bool)	;
extern	char	TodaysLogFileName[] ;	//	Each time prog runs a new .csv file is created. This holds its name.

void	draw_clock	()	{
	char	t[16];
//	char	u[16];
	get_time	(t);
//	log_a_block	(t);
	LCD_CS_ACTIVE;
	DrawString	(10, 40, t,	&Arial_Narrow_Bold15x19,    WHITE, BLUE);
	//	DrawString (10, 40, "Connecting", 	&Arial_Narrow_Bold19x23,    WHITE, BLUE); put clock readout here
//	while	(!spi_tx_cplt)	{
//		HAL_Delay	(1);
//	}
//	strcpy	(u, t);
	strcat	(t, "\r\n");
	LCD_CS_INACTIVE;
//	log_a_block	(nullptr, t);
//	bollocks	("Bollocks.txt", t, false);
	bollocks	(TodaysLogFileName, t, false);
	LCD_CS_ACTIVE;
}


void	draw_setup_screen	()	{	//	power-on screen
	draw_screen_commons	();			//	clears screen, writes name, destroys all buttons
	draw_clock();
//	DrawString (10, 40, "Connecting", 	&Arial_Narrow_Bold19x23,    WHITE, BLUE); put clock readout here
	DrawString (10, 70, "Bluetooth Search", 	&Arial_Narrow_Bold15x19,    WHITE, BLUE);
	DrawString (10, 100, "Search ..  ",	&Arial_Narrow_Bold15x19,    WHITE, BLUE);
}

extern	bool	new_button	(size_t X, size_t Y, size_t Width, size_t Height, const char * but_name, bool(*Press)(size_t), bool(*Release)(size_t))	;
extern	bool	button_select_reverse	(size_t i)	;
extern	bool	button_select_park		(size_t i)	;
extern	bool	button_select_forward	(size_t i)	;
extern	bool	button_released			(size_t i)	;

void	activate_fr_buttons	()	{
	draw_screen_commons	();			//	clears screen, writes name, destroys all buttons
	Loco_Bat.update	(V_Loco_Batt);
//	DrawString (20, 40, "*Connected*", 	&Arial_Narrow_Bold19x23,    WHITE, BLUE);
	new_button	(15, 300, 90, 80, 	"Reverse", 	button_select_reverse, 	button_released);
	new_button	(115, 300, 90, 80, 	"Park", 	button_select_park, 	button_released);
	new_button	(215, 300, 90, 80, 	"Forward", 	button_select_forward, 	button_released);
}


void	draw_driving_screen	(LSClass direction, bool knob_mode)	{
	draw_screen_commons	();			//	clears screen, writes name, destroys all buttons
//	DrawString (20, 40, direction == LSClass::Forward ? "Forward" : "Reverse", 	&Arial_Narrow_Bold19x23,    WHITE, BLACK);
	DrawString (20, 40, direction == LSClass::Forward ? "Forward" : "Reverse", 	&Arial_Narrow_Bold15x19,    WHITE, BLACK);
	HH_Bat.display_update	(V_HC_Batt);
	Loco_Bat.update	(V_Loco_Batt);
	control_knob.create	(knob_mode);
}


void	hh_batt_display_update	(float V)	{
	HH_Bat.display_update	(V);
}


void	loco_batt_display_update	(float V)	{
	Loco_Bat.update	(V);
}

void	draw_screen_commons	()	{
	HH_Bat.destroy	();
	Loco_Bat.destroy	();
	HH_Bat.destroy	();
	destroy_all_buttons	();
	control_knob.erase();	//	precaution
	LCD_Clear	(WHITE);
	draw_loco_name	();
}

