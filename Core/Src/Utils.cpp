/*
 * Utils.cpp	INTENDED TO BE PROJECT SPECIFIC
 *
 *  Created on: Feb 11, 2024
 *      Author: Jon Freeman  B. Eng. Hons
 *
 *	For menus, and functions executed through menus.
 *	Put such clutter here,
 *
 *	Keep 'Project.cpp' for the main logical flow of your project
 */
#define	USING_RTC

#include	<cstdio>			//	for sprintf
#include	<cstring>			//	for strlen

#include	"parameters.hpp"
#include	"Serial.hpp"
#include	"CmdLine.hpp"

extern	Serial	pc	;
extern	Serial	bt	;

//extern	double	read_knob	()	;
//extern	bool	can_node_command	(int node, int command, int data)	;
//extern	bool	send_can_msg	(int node11bit, uint8_t * TxData, int len)	;
extern	char * get_time	(char * dest)	;	//	get e.g. "16:50:11"


//	Prototypes for functions included in 'settings_data' menu structure
bool	null_cmd	(parameters &);
bool	set_defaults_cmd	(parameters &);
bool	set_one_wrapper_cmd	(parameters &);

//enum	class	MenuType	{MENU, SETTINGS}	;	//	in 'parameters.hpp'
/*bool	ver_cmd	(parameters & par)	{
	char * p = get_version();
	pc.write	(p, strlen(p));
	pc.write	("\r\n", 2);
	return	(true);
}*/

struct cli_menu_entry_set	const  settings_data[]
{    // Can not form pointers to member functions.
	{"?",     	"Lists all user settings, alters none", null_cmd, static_cast<int32_t>(MenuType::SETTINGS)}, //	Examples of use follow
	{"defaults","Reset settings to factory defaults", set_defaults_cmd},     //	restore factory defaults to all settings
	{"mca",		"My CAN Address", 		set_one_wrapper_cmd, 	0, 0x7ff, 6, 1.0},   //
	{nullptr},	//	June 2023 new end of list delimiter. No need to pass sizeof
}	;

//	Prototypes for functions included in 'pc_command_list' menu structure
bool	menucmd	(parameters &);
bool	ping_cmd	(parameters &);
bool	getvb_cmd	(parameters &);
bool	rtc_cmd	(parameters &);
//bool	cnc_cmd	(parameters &);
bool	adc_cmd	(parameters &);
bool	st_cmd	(parameters &);
bool	sd_cmd	(parameters &);
bool	bril_cmd	(parameters &);
bool	edit_settings_cmd	(parameters &);
//bool	grow_file_cmd	(parameters &);
bool	get_file_cmd	(parameters &);
bool	del_file_cmd	(parameters &);
bool	dir_cmd			(parameters &);
bool	make_dir_cmd	(parameters & par)	;
bool	wav_cmd	(parameters & par)	;
bool	odo_cmd	(parameters & par)	;


//#if 0
/**
struct  cli_menu_entry_set      const loco_command_list[] = {
List of commands accepted from external pc through non-opto isolated com port 115200, 8,n,1
*/
struct  cli_menu_entry_set	const pc_command_list[] = {
    {"?", "Lists available commands", 	menucmd, static_cast<int32_t>(MenuType::MENU)},
	{"ping", "Received a 'ping'. Update local ping count", ping_cmd},
	{"getvb", "Received a 'ping'. Update local ping count", getvb_cmd},
	{"rtc", "Read the real time clock", 	rtc_cmd},
	{"adc", "check adc dma working", 	adc_cmd},
	{"st", "Set real time clock Time", 		st_cmd},
	{"sd", "Set real time clock Date", 		sd_cmd},
	{"md", "Make directory", 		make_dir_cmd},
	{"dir", "File Directory", 			dir_cmd},
	{"bril", "Display brightness 0-99", bril_cmd},		//	New July 2026
	{"us", "user settings", 			edit_settings_cmd},
	{"file", "get text file", 			get_file_cmd},
	{"del", "Delete file", 		del_file_cmd},
	{"wav", "look into .wav file", 		wav_cmd},
	{"odo", "testing odo code", 		odo_cmd},
    {"nu", "do nothing", null_cmd},
    {nullptr},	//	June 2023 new end of list delimiter. No need to pass sizeof
}   ;
//#endif


bool	got_vis_response	(parameters &);
bool	got_vb_response	(parameters &);
bool	got_speed_response	(parameters & par)	;
bool	got_motorspeeds_response	(parameters & par)	;
/*
 * THESE ARE WHAT THE LOCO INTERFACE BLACK BOX EXPECTS
 //	*	Bluetooth_HC_Loco_Interface - A black box, NOT the hand-held box with the display
struct  cli_menu_entry_set	const bluetooth_loco_interface_commands[] = {
    {"?", "Lists available commands", 	menucmd, static_cast<int32_t>(MenuType::MENU)},
	{"?vis", "Report battery volts, current, loco speed", 	bt_get_vis_cmd},
	{"?vb", "Report battery volts", 	bt_get_vb_cmd},
	{"?ib", "Report battery current", 	bt_get_ib_cmd},
	{"?s", "Report speed", 				bt_get_rail_speed_cmd},
	{"?m", "Report motor speeds", 		bt_get_motor_speeds_cmd},
	{"fw", "Set Forward switch", 		bt_set_forward_cmd},
	{"re", "Set Reverse switch", 		bt_set_reverse_cmd},
//	{"dr", "Drive nn", 					bt_set_drive_cmd},		NO NO, Rethought this, use 'fw' or 're' instead as apt. Change between fw and re accompanied by control effort -> 0.0
	{"rb", "Regen Brake nn", 			bt_set_regen_brake_cmd},
	{"pk", "Park nn", 					bt_set_park_cmd},
	{"h", "Horns 0, 1, 2", 				bt_set_horns_cmd},
	{"p", "Received a 'ping'. Update local ping count", bt_return_ping_cmd},
//	{"us", "user settings", 			edit_settings_cmd},
    {"nu", "do nothing", null_cmd},
    {nullptr},	//	June 2023 new end of list delimiter. No need to pass sizeof
}   ;

 */
struct  cli_menu_entry_set	const bluetooth_commands[] = {
    {"?", "Lists available commands", 	menucmd, static_cast<int32_t>(MenuType::MENU)},
	{"p", "Received a 'ping'. Update local ping count", ping_cmd},
//	{"adc", "check adc dma working", 	adc_cmd},
	{"us", "user settings", 			edit_settings_cmd},
	{"vis", "got response to '?vis'", got_vis_response},
	{"vb", "got response to '?vb'", got_vb_response},
	{"s", "got response to '?s'", got_speed_response},
	{"m", "got response to '?m'", got_motorspeeds_response},
    {"nu", "do nothing", null_cmd},
    {nullptr},	//	June 2023 new end of list delimiter. No need to pass sizeof
}   ;
//#endif


bool    AT_cmd (struct parameters & par)	;	//
bool    atb_cmd (struct parameters & par)	;	//
bool    vi_cmd (struct parameters & par)	;	//
bool    seton_cmd (struct parameters & par)	;	//
bool    clroff_cmd (struct parameters & par)	;	//
bool    i_cmd (struct parameters & par)	;	//
bool    ce_cmd (struct parameters & par)	;	//
bool    sb_cmd (struct parameters & par)	;	//
bool    pl_cmd (struct parameters & par)	;	//
bool	can_report_cmd	(parameters & par)	;
bool    x_cmd (struct parameters & par)	;	//

/**
struct  cli_menu_entry_set      const loco_command_list[] = {
List of commands accepted from external pc through non-opto isolated com port 115200, 8,n,1
*/

/*struct  cli_menu_entry_set       const pc_command_list[] {
    {"?", "Lists available commands", menucmd},
	{"AT", "AT commands for setting up Bluetooth modules", AT_cmd},
	{"atb", "Calc counter for new baud rate", atb_cmd},
	{"canrep", "Test for CAN bus errors", 	can_report_cmd},
	{"cnc", "Can Node1-127, Command0-255, Param0-255", 	cnc_cmd},
    {"vi", "Fifth, do nothing very much at all really", vi_cmd},
    {"set", "set one or  more output on", seton_cmd},
    {"clr", "clr one or more output off", clroff_cmd},
	  {"x", "test 0x hexadecimal input", x_cmd},
	  {"i", "read an input", i_cmd},
	  {"ce", "can errors", ce_cmd},
//	  {"fwistest", "user settings", fwistest},
	  {"sb", "signals buggery", sb_cmd},
	  {"rtc", "real time clock buggery", rtc_cmd},
	  {"st", "real time clock Time", st_cmd},
	  {"sd", "real time clock Date", sd_cmd},
	  {"pl", "pins lister", pl_cmd},
	  {"us", "user settings", edit_settings_cmd},
    {"nu", "do nothing", null_cmd},
    {nullptr},	//	June 2023 new end of list delimiter. No need to pass sizeof
}   ;
*/
//CommandLineHandler	command_line_handler	(pc_command_list, & pc);	//	Nice and clean

//	************* Create Utilities *****************
//extern	UART_HandleTypeDef	huart1;	//	uarts used in this project
//extern	UART_HandleTypeDef	huart2;	//	uarts used in this project
extern	I2C_HandleTypeDef 	hi2c1;	//	I2C
//extern	bool	set_output_bit	(OUTPIN which_output, bool hiorlo)	;

//Serial				ctrl(huart1);
//Serial				pc(huart2);		//, * Com_ptrs[];
CommandLineHandler	pc_command_line_handler	(pc_command_list, &pc);	//	Nice and clean
CommandLineHandler	rx_from_bluetooth_handler	(bluetooth_commands, &bt);	//	Nice and clean

extern	int	log_a_block	(char * txt)	;
extern	int files_dir (char * selected_file_name, const int * selected_file_number);

bool	dir_cmd	(parameters &)	{
	int	i = 0;
	files_dir	(nullptr, &i);
//	files_dir	(nullptr, nullptr);
	return	(true);
}


/*bool	grow_file_cmd	(parameters &)	{
	char	t[32];
	log_a_block	(get_time(t));
	log_a_block((char*)"\r\n");
	return	(true);
}*/

extern	void	get_file_n	(int n)	;
extern	int	make_dir	(char * dirname)	;
extern	int	open_wav	(char * wavname)	;
extern	int	odo_bugger	(uint32_t dist);

bool	odo_cmd	(parameters & par)	{
	uint32_t	time = uwTick;
	char	t[32];
	int		len;
	odo_bugger	((uint32_t)par.flt[0]);
	len = sprintf	(t, "Time taken in odo = %ldms\r\n", uwTick - time);
	pc.write	(t, len);
	return	(true);
}


bool	wav_cmd	(parameters & par)	{
	char	t[52] { 0 }	;
	int		len { 0 }	;
	if	(strlen(par.command_line) > 4)	{
		while	((len < 40) && (par.command_line[len + 4] >= ' '))	{
			t[len] = par.command_line[len + 4];
			len++;
		}
		pc.write	("Got wav name [", 14);
		pc.write	(t, strlen(t));
		pc.write	("]\r\n", 3);
		pc.tx_any_buffered();
		open_wav	(t);
		pc.write	("Back from open_wav\r\n", 20);
	}
	return	(true);
}


bool	make_dir_cmd	(parameters & par)	{
	int	len = strlen(par.command_line + 3);
	pc.write	("Creating dir [", 14);
	pc.write	(par.command_line + 3, len);
	pc.write	("]\r\n", 3);
	if	(len)
		make_dir((char *)par.command_line + 3);
	return	(true);
}


extern	int	file_erase	(char * filename)	;

bool	del_file_cmd	(parameters & par)	{	//	also works to delete directories
	char	t[64];
	int	rv;
	int	len = strlen(par.command_line + 4);		//	not good way of finding filename to delete
	pc.write	("Attempting to delete [", 22);
	pc.write	(par.command_line + 4, len);
	pc.write	("]\r\n", 3);
	if	(len)	{
		rv = file_erase((char *)par.command_line + 4);
		len = sprintf	(t, "file_erase returned %d\r\n", rv);
		pc.write	(t, len);
	}
	return	(true);
}

	bool	get_file_cmd	(parameters & par)	{
//	char	t[132];
//	char	fn[32];
//	int		len;
	int	i = (int)par.flt[0];
	get_file_n	(i);
//	files_dir	(fn, &i);
//	len = sprintf	(t, "Got file name [%s]\r\n", fn);
//	pc.write	(t, len);
	return	(true);
}


extern	float	V_Loco_Batt;
extern	float	I_Loco_Batt;
extern	float	Loco_Speed;

bool	got_vis_response	(parameters & par)	{
//	char	t[64];
	V_Loco_Batt 		= par.flt[0];
	I_Loco_Batt 		= par.flt[1];
	Loco_Speed 	= par.flt[2];
//	size_t	len = sprintf	(t, "Got ?vis response [%.1f]V, [%.1f]A, [%.1f] RPM\r\n", V_Batt, I_Batt, Loco_Speed);
//	pc.write	(t, len);
	return	(true);
}


bool	got_vb_response	(parameters & par)	{
//	char	t[64];
	V_Loco_Batt = par.flt[0];
//	size_t	len = sprintf	(t, "Got ?vb response [%.1f]V\r\n", V_Batt);
//	pc.write	(t, len);
	return	(true);
}


bool	got_motorspeeds_response	(parameters & par)	{

	return	(true);
}


bool	got_speed_response	(parameters & par)	{
//	char	t[64];
	Loco_Speed = par.flt[0];
//	size_t	len = sprintf	(t, "Got ?s response [%.1f]V\r\n", Loco_Speed);
//	pc.write	(t, len);
	return	(true);
}



#if 0


extern	void	CAN_status_report	()	;	//	6th March 2024
bool	can_report_cmd	(parameters & par)	{
	CAN_status_report	()	;	//	6th March 2024
	return	(true);
}


bool    AT_cmd 	(struct parameters & par)	{	//
	pc.write	(par.command_line, strlen(par.command_line));
	return	(true);
}


bool	atb_cmd 	(struct parameters & par)	{	//	baud setting
	/*
AT+BAUD AT+BAUD1 – sets the baud rate to 1200 and returns OK1200
AT+BAUD2 – sets the baud rate to 2400 and returns OK2400

Other possible baud rates are
1——— 1200	AT+BAUD8,0,0
2——— 2400
3——— 4800
4——— 9600
5——— 19200
6——— 38400
7——— 57600
8——— 115200
9——— 230400
A——— 460800
B——— 921600
C——— 1382400
*/
	const	uint32_t	baud_list[] = {1200, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200
			, 230400, 460800, 921600, 1382400};
	char	t[100] {0};
	uint32_t	el = (int)par.flt[0];
	int	len;
	if	((el < 1) || (el >= (sizeof(baud_list) / sizeof(uint32_t))))	{
		len = sprintf	(t, "Wrong baud [%ld], range is 1 to %d\r\n", el, (sizeof(baud_list) / sizeof(uint32_t)) - 1);
		pc.write	(t, len);
		return	(false);
	}
//	int x = UART_DIV_SAMPLING16(HAL_RCC_GetPCLK2Freq(), newbaud);
	uint32_t x = UART_DIV_SAMPLING16(HAL_RCC_GetPCLK2Freq(), baud_list[el]);
	len = sprintf	(t, "Cntr for baud %ld is %ld\r\n", baud_list[el], x);

	USART1->BRR = x;

	pc.write	(t, len);
	len = sprintf	(t, "BRR=%ld\r\n", USART1->BRR);
	pc.write	(t, len);
	return	(true);
}


bool	cnc_cmd	(parameters & par)	{	//	Now must use 0x addressing
	//							CAN Address		Command			Parameter
//	return	(can_node_command((int)par.flt[0], (int)par.flt[1], (int)par.flt[2]));
	uint8_t	data[4];
	data[0] = (uint8_t)par.flt[1];
	data[1] = (uint8_t)par.flt[2];
	return	(send_can_msg	((int)par.flt[0], data, 2)	)	;
}


bool    x_cmd (struct parameters & par)	{	//	Test using 0x prefix - it works!
	char	t[180];
	int	len = sprintf	(t, "%s, decimal %ld",par.command_line , (int32_t)par.flt[0]);
	pc.write	(t, len);
	return	(true);
}


bool    i_cmd (struct parameters & a)	{	//	read an input
	char	t[55];
	INPIN b = (INPIN)a.flt[0];
	int	len = sprintf(t, "In%d = %c\r\n", static_cast<int>(b), get_input_bit_debounced(b,1)	? 'T' : 'F');
	pc.write(t, len);
    return	(true);
}


bool    seton_cmd (struct parameters & a)	{	//	set one or more outputs ON
	OUTPIN	j;
	int	k = a.numof_floats;
	while	(k > 0)	{
		j = (OUTPIN)a.flt[--k];
		set_output_bit	(j, true);
	}
    return	(true);
}

bool    clroff_cmd (struct parameters & a)	{	//	set one or more outputs OFF
	OUTPIN	j;
	int	k = a.numof_floats;
	while	(k > 0)	{
		j = (OUTPIN)a.flt[--k];
		set_output_bit	(j, false);
	}
    return	(true);
}

#endif

/**
*   void    menucmd 		(struct parameters & a)
*	void	list_settings	(const menu_entry_set * list)
*   List available terminal commands to pc terminal. No sense in touch screen using this
*/
void	list_settings	(const cli_menu_entry_set * list)	{
	int i = 0;
	int len;
//	int32_t	ival;
//	float	fval;
	char	t[200];
	char	ins_tab[2] {0,0};
	extern 	char * 	get_version	();//	{	return	(version_str);	}
	pc.write	(get_version(), strlen(get_version()));
	pc.write	("\r\n", 2);
	len = sprintf	(t, "Listing %s Functions and Values :\r\n", list[0].min ? "SETTINGS" : "MENU");
	pc.write	(t, len);
	while	(list[i].cmd_word)	{
		(6 > strlen(list[i].cmd_word)) ? ins_tab[0] = '\t' : ins_tab[0] = 0;
		len = sprintf	(t, "[%s]\t%s%s"
			, list[i].cmd_word
			, ins_tab
			, list[i].description	);
		pc.write	(t, len);	//	This much common to MENU and SETTINGS
		if	(list[0].min)	{	//	is SETTINGS, not MENU
/*			if	(my_settings.read1(list[i].cmd_word, ival, fval))	{
				(6 > strlen(list[i].cmd_word)) ? ins_tab[0] = '\t' : ins_tab[0] = 0;
				len = sprintf	(t, "\tmin%ld, max%ld, def%ld\t[is %ld]\t(float mpr %.2f)"
					, list[i].min
					, list[i].max
					, list[i].de_fault
					, ival
					, fval	);
				pc.write	(t, len);
			}
			else	pc.write	("Settings Read Error\r\n", 21);

			*/

		}	//	Endof is SETTINGS, not MENU
		pc.write	("\r\n", 2);
		i++;
	}	//	Endof 	while	(list[i].cmd_word)
	pc.write("End of List of Commands\r\n", 25);
}


bool    menucmd (struct parameters & par)     {
	list_settings	(par.command_list)	;
    return	(true);
}


bool	bril_cmd	(parameters & a)	{	//	New July 2026 - big current savings to be had
	uint16_t	bril = (uint16_t) (a.flt[0] * 10.0);
	if	(bril > 990)	bril = 990;
	if	(bril < 99)		bril = 99;
	TIM1->CCR1 = bril;
	return	(true);
}


uint32_t	rx_ping_cnt { 0 }	;	//	Incremented in ping_cmd in response to receiving "p"

uint32_t	get_rx_ping_cnt	()	{	return	(rx_ping_cnt);	}
void		clr_rx_ping_cnt	()	{	rx_ping_cnt = 0L;	}

bool	ping_cmd	(parameters &)	{	//	here when received "p\r" from loco controller
	rx_ping_cnt++;
	char	t[64];
	int	len	= sprintf	(t, "Hand Unit Received ping. Count = %ld\r\n", rx_ping_cnt);
	pc.write	(t, len);
	return	(true);
}


bool	getvb_cmd	(parameters &)	{
//	ping++;
	char	t[64];
	int	len	= sprintf	(t, "Sending ?vb\r\n");
	pc.write	(t, len);
	bt.write	("?vb\r", 4);
	return	(true);
}


bool	edit_settings_cmd (struct parameters & par)     {	//	Here from CLI having found "us "
//	bool	rv =	(my_settings.edit	(par));
	list_settings	(settings_data)	;
//	return	(rv)	;
	return	(false)	;	//	temp jf
}


bool    set_one_wrapper_cmd (struct parameters & par)     {	//	Called via edit, a.second_word found in edit
//	return	(my_settings.set_one	(par));
	return	(false)	;	//	temp jf
}


bool    null_cmd (struct parameters & par)     {
	const char t[] = "null command - does nothing useful!\r\n";
	pc.write(t, strlen(t));
    return	(true);
}


bool    set_defaults_cmd (struct parameters & par)     {
//	return	(my_settings.set_defaults());
	return	(false)	;	//	temp jf
}


extern	int32_t	run_mode;
bool    set_runmode_cmd (struct parameters & par)     {
	run_mode = (int32_t)par.flt[0];
	return	(true);
}

//	This is the Bluetooth Hand Controller with 320x480 Graphic Touch Screen

void	check_commands	()	{	//	Called from ForeverLoop
/**
 * bool	Serial::test_for_message	()	{
 *
 * Called from ForeverLoop at repetition rate
 * Returns true when "\r\n" terminated command line has been assembled in lin_inbuff
 */
	char * buff_start = pc.test_for_rx_message();
	if	(buff_start != nullptr)	{
		pc.write	("[PC]", 4);
		pc_command_line_handler.CommandExec(buff_start);
	}
	buff_start = bt.test_for_rx_message();
	if	(buff_start != nullptr)	{
		pc.write	("[bT]", 4);
		pc.write	(buff_start, strlen(buff_start));
		rx_from_bluetooth_handler.CommandExec(buff_start);
	}
}


extern	uint32_t	can_errors;
extern	void	rtc_buggery	()	;
extern	void	adc_cnt_report	()	;

bool	adc_cmd	 (struct parameters & par)     {
//#ifdef	USING_ANALOG
	adc_cnt_report();
//#endif
	return	(true);
}


#ifdef	USING_RTC
extern	bool	set_time	(struct parameters & par)	;
extern	bool	set_date	(struct parameters & par)	;

bool	st_cmd	 (struct parameters & par)     {
	return	(set_time	(par));
}


bool	sd_cmd	 (struct parameters & par)     {
	return	(set_date	(par));
}

bool	rtc_cmd	 (struct parameters & par)     {
	rtc_buggery();
	return	(true);
}


#else
//bool	st_cmd	 (struct parameters & par)     {
//	return	(false);
//}


//bool	sd_cmd	 (struct parameters & par)     {
//	return	(false);
//}


//bool	rtc_cmd	 (struct parameters & par)     {
//	return	(false);
//}


#endif





