/*
 * LCD_Buttons.cpp
 *
 *  Created on: Aug 22, 2025
 *      Author: Jon Freeman  B Eng (Hons) MIET
 */
#include	"main.h"
#include	"Serial.hpp"
#include	<cstdio>
#include	"colours.h"
#include	"fonts.h"
#include	"ProjectLocoHC.hpp"

extern	Serial	pc;		//	Serial port for pc comms
extern	"C" {	bool	DrawBar		(uint32_t Xstart, uint32_t Ystart, uint32_t Xend, uint32_t Yend, uint16_t Colour)	;	}
extern	"C" {	bool	DrawString	(size_t const x, size_t const y, const char * const str, sFONT* Font, size_t const BG_Col, size_t const FG_Col)	;	}
//extern	bool	set_Loco_State		(LSClass	new_state)	;
extern	bool	set_Loco_Direction	(LSClass	new_state)	;


#define	MAX_BUTTONS	13

class	button_info	{
	bool	in_use_flag;
	bool	press_status_new;
	bool	press_status_old;
	size_t	X1;
	size_t	Y1;
	size_t	X2;
	size_t	Y2;
	bool	(*pressed)(size_t);	//	Function to be executed when button is pressed
	bool	(*released)(size_t);	//	Function to be executed when button is released
public:
	int		button_number	;
	const char *	name;

	void	draw_button_outline	(uint16_t colour)	{
		DrawBar	(X1, Y1, X2, Y1 + 2, colour);	//	draw top horizontal
		DrawBar	(X1, Y2, X2, Y2 + 2, colour);	//	draw bottom horizontal
		DrawBar	(X1, Y1, X1 + 2, Y2, colour);	//	draw left vertical
		DrawBar	(X2, Y1, X2 + 2, Y2, colour);	//	draw right vertical
	}

	void	draw_button_body	(uint16_t colour)	{
		DrawBar	(X1, Y1, X2, Y2, colour);	//	draw solid box
		DrawString (X1 + 4, Y1 + ((Y2 - Y1) / 3), name, &Arial_Narrow_Bold12x15,    colour,  WHITE);
	}

	void	draw_button	(uint16_t colour)	{
		draw_button_body	(colour);
		draw_button_outline	(BLACK);
	}

	void	erase_button	()	{
		DrawBar	(X1, Y1, X2 + 2, Y2 + 2, WHITE);
		in_use_flag = false;
	}

#define	BUTTON_NEWLY_PRESSED	(press_status_new && !press_status_old)	//	is being pressed this time, not last time
#define	BUTTON_NEWLY_RELEASED	(!press_status_new && press_status_old)

	bool	button_action	(int  button, size_t X, size_t Y, bool touch)	{	//true on new press or new release, false otherwise
		bool	rv { false }	;
		if	(in_use_flag)	{
			press_status_new = (touch && (X > X1) && (X < X2) && (Y > Y1) && (Y < Y2));	//	Up to the moment button is being touched or not
			if	(BUTTON_NEWLY_PRESSED)	{
				draw_button_outline	(YELLOW);
				pressed	(button);
				rv = true;
			}
			if	(BUTTON_NEWLY_RELEASED)	{
				draw_button_outline	(BLACK);
				released	(button);
				rv = true;
			}
			press_status_old = press_status_new;
		}
//		return	(false)	;	//	Always returns false
		return	(rv);		//	Returns true when button newly pressed or newly released, i.e. a button action has been performed
	}

	void	button_setup	(size_t X, size_t Y, size_t Width, size_t Height, const char * but_name, bool(*Press)(size_t), bool(*Release)(size_t))	{
		in_use_flag			= true;
		press_status_new	= false;
		press_status_old	= false;
		X1		= X;
		Y1		= Y;
		X2		= X + Width;
		Y2		= Y + Height;
		name 	= but_name;
		pressed = Press;
		released = Release;
		draw_button(MAGENTA);
	}

	bool	in_use	()	{
		return	(in_use_flag);
	}

	void	destroy_button	()	{
		in_use_flag = false;				//	Is sufficient
	}
}
	button_table[MAX_BUTTONS]	;	//	Table of buttons


void	scan_buttons	(size_t X, size_t Y, bool touch)	{
	for	(size_t i = 0; i < MAX_BUTTONS; i++)	{
		button_table[i].button_action(i, X, Y, touch);
	}
}


void	destroy_all_buttons	()	{
	for	(size_t i = 0; i < MAX_BUTTONS; i++)	{
		button_table[i].destroy_button();
	}
}


bool	new_button	(size_t X, size_t Y, size_t Width, size_t Height, const char * but_name, bool(*Press)(size_t), bool(*Release)(size_t))	{
	int i { 0 }	;
	while	(i < MAX_BUTTONS)	{
		if	(!(button_table[i].in_use()))	{	//	found empty button slot
			button_table[i].button_setup(X, Y, Width, Height, but_name, Press, Release);
			button_table[i].button_number = i;
			return	(true);
		}
		i++;
	}
	return	(false);
}


bool	button_select_reverse	(size_t i)	{
	set_Loco_Direction	(LSClass::Reverse)	;
	char	t[64];
	size_t	len;
	len = sprintf	(t, "Fn %d, [%s], Set Reverse\r\n", i, button_table[i].name);
	pc.write	(t, len);
	return	(true);
}


bool	button_select_park		(size_t i)	{
	set_Loco_Direction	(LSClass::Park)	;
	char	t[64];
	size_t	len;
	len = sprintf	(t, "Fn %d, [%s], Set Park\r\n", i, button_table[i].name);
	pc.write	(t, len);
	return	(true);
}


bool	button_select_forward	(size_t i)	{
	set_Loco_Direction	(LSClass::Forward)	;
	char	t[64];
	size_t	len;
	len = sprintf	(t, "Fn %d, [%s], Set Forward\r\n", i, button_table[i].name);
	pc.write	(t, len);
	return	(true);
}


bool	button_released	(size_t i)	{
//	char	t[64];
//	size_t	len;
//	len = sprintf	(t, "Fn %d, [%s], Button Released\r\n", i, button_table[i].name);
//	pc.write	(t, len);
	return	(true);
}




