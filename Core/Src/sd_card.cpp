/*
 * sd_card.cpp
 *
 *  Created on: 17 Jun 2026
 *      Author: jon34
 */
#include "main.h"
#include "fatfs.h"
#include "Serial.hpp"
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include	"ProjectLocoHC.hpp"

extern	SPI_HandleTypeDef hspi1;
extern	Serial	pc;

//char SDTxBuffer[250];
char	TodaysLogFileName[32] { 0 }	;	//	Each time prog runs a new .csv file is created. This holds its name.
int	len;

//extern	"C"	{
//	bool	set_spi_prsc	(uint16_t	val)	;	//	New June 2026 in main.c use 2, 4, 8, ... 256
//}


/*	int files_dir (char * selected_file_name, int selected_file_number)
 * selected_file_name may be nullptr, or may be address to copy file name to
 * selected_file_number is the position in directory list.
 *
 * Idea is to use twice, first with selected_file_name nullptr to just see visual list on PuTTY
 * Second, to choose file number 'n' to upload nth file to be saved on laptop
 */

FATFS 	FatFs;
FRESULT FR_Status;	//	Integer 0 to 19

#define	FILE_BUFF_SIZE	2500
#define	WRITE_TRIG_SIZE	2000	//	trigger file write when buffer contains this many or more
FRESULT	log_a_block	(const char * local_file_name, const char * txt)	;

	//	Assumes all file content is text and only usable on one file

void	bollocks	(const char * file_name, const char * to_add, bool immediate)	{	//	immediate flag used to force real file write
	static	int	onptr	{ 0 }	;
	char	t[128];
	int	tmp;
	static uint32_t	time;
	static	char	bigbuff[FILE_BUFF_SIZE+4];	//	consolidate many small write chunks into one larger blob for actual writes to file
	//	Assume here with fresh text to put on file queue
	tmp = strlen	(to_add);
	if	((onptr + tmp) < FILE_BUFF_SIZE)	{	//	will fit on buffer
		strcpy	(bigbuff + onptr, to_add);
		onptr += tmp;
//		tmp = sprintf	(t, "Bollocks now %d\r\n", onptr);
//		pc.write	(t, tmp);
	}
	else	{	//	disaster, trying to put too much on buffer

	}
	if	((immediate) || (onptr >= WRITE_TRIG_SIZE))	{	//	write block to file
		time = uwTick;
		tmp = log_a_block	(file_name, bigbuff);	//	Does log_a_block terminate on '\n' ?
		if	(tmp != FR_OK)
			pc.write	("bollocks fail\r\n", 15);
		else	{
//			tmp = sprintf	(t, "bollocks saving %d to file[%s], %ld ms\r\n", onptr, TodaysLogFileName, uwTick - time);
			tmp = sprintf	(t, "bollocks saving %d to file[%s], %ld ms\r\n", onptr, file_name, uwTick - time);
			pc.write	(t, tmp);
		}
		onptr = 0;
	}
}


int files_dir (char * selected_file_name, const int * selected_file_number)	//
{
//    global FATFS fs;
//    global FRESULT res;
    char buff[300];
    int	len;
    int	FileNumber = 0;
    DIR	MyDir;
	static	FILINFO	MyFileinfo;
//	bool	delete_file = ((selected_file_name == nullptr) && (selected_file_number == nullptr));

//	if	(delete_file)
	LCD_CS_INACTIVE;

    FR_Status = f_mount(&FatFs, "", 1);
    len = sprintf	(buff, "f_mount returned %d\r\n", FR_Status);
    pc.write	(buff, len);

    buff[0] = 0;

    FR_Status = f_opendir	(&MyDir, buff);
    len = sprintf	(buff, "f_opendir returned %d\r\n", FR_Status);
    pc.write	(buff, len);

    do	{
    	FR_Status = f_readdir	(&MyDir, &MyFileinfo);
		if	(MyFileinfo.fname[0] == 0)	//	Have read to end of directory list
			break;
		if	((nullptr != selected_file_name) && (*selected_file_number == FileNumber))
			strcpy	(selected_file_name, MyFileinfo.fname);
		len = sprintf	(buff, "%d\t%s\tsize %ld\tattrib %d\r\n", FileNumber++, MyFileinfo.fname, MyFileinfo.fsize, MyFileinfo.fattrib);
		while	(!pc.write	(buff, len))	//	use txbuff to max capacity if need be
			pc.tx_any_buffered();
//		MyFileinfo.fdate = 1234;
//		HAL_Delay	(2);	//	chance for com port to catch up
//    }	while	(MyFileinfo.fname[0] != 0)	;
    }	while	(1)	;
    //	only get here from break; once MyFileinfo.name[0] == 0
    FR_Status = f_mount	(NULL, "", 0);
    return FR_Status;
}



int	transmit_file	(const char * fname)	{	//	Quite reasonably blocks other ops during file send
	char	t[96];
	int		len;
//now global	FATFS	FatFs;			//	File system object structure
	UINT	RRC;		//	Record of numof bytes written to file
	FIL 	Fil;		//	File object structure
//	FRESULT FR_Status;	//	Integer 0 to 19
//	set_spi_prsc	(SPI_PRSC_SD);	//	slower spi for SD card ** Found will run at full speed **
	LCD_CS_INACTIVE;
	do	{
		FR_Status = f_mount(&FatFs, "", 1);
		if (FR_Status != FR_OK)
		{
			len = sprintf(t, "Error! While Mounting SD Card, Error Code: (%i)\r\n", FR_Status);
			pc.write	(t, len);
			break;
		}
		FR_Status = f_open(&Fil, fname, FA_OPEN_EXISTING | FA_READ);	//
		if	(FR_Status != FR_OK)	{
			len = sprintf	(t, "Couldn't open file (%d)\r\n", FR_Status);
			pc.write	(t, len);
			break;
		}
		//	read stuff from file here
		RRC = 1;	//	not 0
		while	(RRC)	{
			f_read(&Fil, t, 90, &RRC);
				while	(RRC && !(pc.write	(t, RRC)))	{	//	Tests pc.write for fail on insufficient buffer space
					pc.tx_any_buffered();
				}
		}
		FR_Status = f_close(&Fil);
		f_mount	(NULL, "", 0);
		pc.write	("Done sending log file [", 23);
		pc.write	(fname, strlen(fname));
		pc.write	("]\r\n", 3);
	}	while	(0)	;
	f_mount	(NULL, "", 0);
	return	(FR_Status);
}


int	make_dir	(char * dirname)	{
	char	t[64];
//now global	FATFS	FatFs;			//	File system object structure
//now global	FRESULT FR_Status;	//	Integer 0 to 19
	LCD_CS_INACTIVE;
	do	{
		FR_Status = f_mount(&FatFs, "", 1);
		if (FR_Status != FR_OK)
		{
			len = sprintf(t, "Error! While Mounting SD Card, Error Code: (%i)\r\n", FR_Status);
			pc.write	(t, len);
			break;
		}
		FR_Status = f_mkdir	(dirname);
		if (FR_Status != FR_OK)
		{
			pc.write	("f_mkdir failed\r\n", 16);
		}
	}	while	(0);
	f_mount	(NULL, "", 0);
	return	(FR_Status);
}


void	get_file_n	(int const n)	{	//	get nth file in directory listing
	char	fname[64];
	files_dir	(fname, &n);	//	Includes 	LCD_CS_INACTIVE;
	pc.write	("In get_file_n, got [", 20);
	pc.write	(fname, strlen(fname));
	pc.write	("]\r\n", 3);
	transmit_file	(fname);
}



FRESULT	log_a_block	(const char * local_file_name, const char * txt)	{
//	const char * p = nullptr;
	char	t[84];
	int		len = sprintf	(t, "At log_a_block with [%s], len %d\r\n", local_file_name, strlen(txt));
	pc.write	(t, len);
	if	(!local_file_name)	{
		local_file_name = TodaysLogFileName;	//	globally known name of todays log file
	}
	UINT	WWC;		//	Record of numof bytes written to file
	FIL 	Fil;		//	File object structure
//	set_spi_prsc	(SPI_PRSC_SD);	//	slower spi for SD card ** Found will run at full speed **
	LCD_CS_INACTIVE;

	FR_Status = f_mount(&FatFs, "", 1);
	if (FR_Status != FR_OK)
	{
		len = sprintf(t, "Error! While Mounting SD Card, Error Code: (%i)\r\n", FR_Status);
		pc.write	(t, len);
		return	(FR_Status);
	}

	FR_Status = f_open(&Fil, local_file_name, FA_OPEN_EXISTING | FA_WRITE);	//	CHECK Creates file if missing
	if	(FR_Status != FR_OK)	{
		len = sprintf	(t, "Couldn't open file, Creating new [%s] file\r\n", local_file_name);
		pc.write	(t, len);
		FR_Status = f_open(&Fil, local_file_name, FA_WRITE | FA_CREATE_ALWAYS);		//	CHECK Creates file if missing
	}
	if	(FR_Status != FR_OK)	{
		len = sprintf	(t, "Couldn't open [%s] file (%d)\r\n", local_file_name, FR_Status);
		pc.write	(t, len);
		return	(FR_Status);
	}
	FR_Status = f_lseek(&Fil, f_size(&Fil)); // Move The File Pointer To The EOF (End-Of-File)
	if(FR_Status != FR_OK)	{
		len = sprintf	(t, "log_a_block file fail (%d)\r\n", FR_Status);
		pc.write	(t, len);
		return	(FR_Status);
	}
	//	write stuff to file here

	FR_Status = f_write(&Fil, txt, strlen(txt), &WWC);
	if	(FR_Status != FR_OK)	{
		len = sprintf	(t, "Error (%d) writing file in log_a_block\r\n", FR_Status);
		pc.write	(t, len);
	}
//		len = sprintf	(t, "Adding (%d) to %s of size %ld\r\n", WWC, TodaysLogFileName, f_size(&Fil));
//		pc.write	(t, len);
	FR_Status = f_close(&Fil);
	f_mount	(NULL, "", 0);
//    set_spi_prsc	(SPI_PRSC_LCD);	//	max speed for lcd
	return	(FR_Status);
}


//static void SD_Card_Test(void)
void SD_Card_Test(void)
{
//now global	  FATFS 	FatFs;			//	File system object structure
  FIL 		Fil;			//	File object structure
//now global	  FRESULT 	FR_Status;		//	Integer 0 to 19
  FATFS *	FS_Ptr;
  UINT RWC, WWC; 	// Read/Write Word Counter
  DWORD FreeClusters;
  uint32_t TotalSize, FreeSpace;
  char SDTxBuffer[250];
  char RW_Buffer[200];
//  set_spi_prsc	(SPI_PRSC_SD);	//	16
  do	//	Wheze to allow use of break;
  {
    //------------------[ Mount The SD Card ]--------------------
    FR_Status = f_mount(&FatFs, "", 1);
    if (FR_Status != FR_OK)
    {
      len = sprintf(SDTxBuffer, "Error! While Mounting SD Card, Error Code: (%i)\r\n", FR_Status);
      pc.write	(SDTxBuffer, len);
      break;
    }
    len = sprintf(SDTxBuffer, "SD Card Mounted Successfully! \r\n\n");
    pc.write	(SDTxBuffer, len);
    //------------------[ Get & Print The SD Card Size & Free Space ]--------------------
    f_getfree("", &FreeClusters, &FS_Ptr);
    TotalSize = (uint32_t)((FS_Ptr->n_fatent - 2) * FS_Ptr->csize * 0.5);
    FreeSpace = (uint32_t)(FreeClusters * FS_Ptr->csize * 0.5);
    len = sprintf(SDTxBuffer, "Total SD Card Size: %lu Bytes\r\n", TotalSize);
    pc.write	(SDTxBuffer, len);
    len = sprintf(SDTxBuffer, "Free SD Card Space: %lu Bytes\r\n\n", FreeSpace);
    pc.write	(SDTxBuffer, len);
    //------------------[ Open A Text File For Write & Read Data ]--------------------
    //Open the file
    FR_Status = f_open(&Fil, "TextFileWrite.txt", FA_WRITE | FA_READ | FA_CREATE_ALWAYS);
    if(FR_Status != FR_OK)
    {
      len = sprintf(SDTxBuffer, "Error! While Creating/Opening A New Text File, Error Code: (%i)\r\n", FR_Status);
      pc.write	(SDTxBuffer, len);
      break;
    }
    len = sprintf(SDTxBuffer, "Text File Created & Opened! Writing Data To The Text File..\r\n\n");
    pc.write	(SDTxBuffer, len);
    // (1) Write Data To The Text File [ Using f_puts() Function ]
    f_puts("Hello! From STM32 To SD Card Over SPI, Using f_puts()\n", &Fil);
    // (2) Write Data To The Text File [ Using f_write() Function ]
    strcpy(RW_Buffer, "Hello! From STM32 To SD Card Over SPI, Using f_write()\r\n");
    f_write(&Fil, RW_Buffer, strlen(RW_Buffer), &WWC);
    // Close The File
    f_close(&Fil);
    //------------------[ Open A Text File For Read & Read Its Data ]--------------------
    // Open The File
    FR_Status = f_open(&Fil, "TextFileWrite.txt", FA_READ);
    if(FR_Status != FR_OK)
    {
      len = sprintf(SDTxBuffer, "Error! While Opening (TextFileWrite.txt) File For Read.. \r\n");
      pc.write	(SDTxBuffer, len);
      break;
    }
    // (1) Read The Text File's Data [ Using f_gets() Function ]
    f_gets(RW_Buffer, sizeof(RW_Buffer), &Fil);
    len = sprintf(SDTxBuffer, "Data Read From (TextFileWrite.txt) Using f_gets():%s", RW_Buffer);
    pc.write	(SDTxBuffer, len);
    // (2) Read The Text File's Data [ Using f_read() Function ]
    f_read(&Fil, RW_Buffer, f_size(&Fil), &RWC);
    len = sprintf(SDTxBuffer, "Data Read From (TextFileWrite.txt) Using f_read():%s", RW_Buffer);
    pc.write	(SDTxBuffer, len);
    // Close The File
    f_close(&Fil);
    len = sprintf(SDTxBuffer, "File Closed! \r\n\n");
    pc.write	(SDTxBuffer, len);
    //------------------[ Open An Existing Text File, Update Its Content, Read It Back ]--------------------
    // (1) Open The Existing File For Write (Update)
    FR_Status = f_open(&Fil, "TextFileWrite.txt", FA_OPEN_EXISTING | FA_WRITE);
    FR_Status = f_lseek(&Fil, f_size(&Fil)); // Move The File Pointer To The EOF (End-Of-File)
    if(FR_Status != FR_OK)
    {
      len = sprintf(SDTxBuffer, "Error! While Opening (TextFileWrite.txt) File For Update.. \r\n");
      pc.write	(SDTxBuffer, len);
      break;
    }
    // (2) Write New Line of Text Data To The File
    FR_Status = (FRESULT)f_puts("This New Line Was Added During Update!\r\n", &Fil);
//    f_puts("This New Line Was Added During Update!\r\n", &Fil);
    f_close(&Fil);
    memset(RW_Buffer,'\0',sizeof(RW_Buffer)); // Clear The Buffer
    // (3) Read The Contents of The Text File After The Update
    FR_Status = f_open(&Fil, "TextFileWrite.txt", FA_READ); // Open The File For Read
    f_read(&Fil, RW_Buffer, f_size(&Fil), &RWC);
    len = sprintf(SDTxBuffer, "Data Read From (TextFileWrite.txt) After Update:%s", RW_Buffer);
    pc.write	(SDTxBuffer, len);
    f_close(&Fil);

#if 1
    //------------------[ Delete The Text File ]--------------------
    // Delete The File

    FR_Status = f_unlink("TextFileWrite.txt");
    if (FR_Status != FR_OK){
        len = sprintf(SDTxBuffer, "Error! While Deleting (TextFileWrite.txt) File.. \r\n");
    }    else	{
        len = sprintf(SDTxBuffer, "Success Deleting The (TextFileWrite.txt) File.. \r\n");
    }
    pc.write	(SDTxBuffer, len);

#endif

  } while(0);
  //------------------[ Test Complete! Unmount The SD Card ]--------------------
  FR_Status = f_mount(NULL, "", 0);
  if (FR_Status != FR_OK)
  {
      len = sprintf(SDTxBuffer, "Err Un-mounting SD, Code: (%i)\r\n", FR_Status);
  } else	{
      len = sprintf(SDTxBuffer, "SD Card Un-mounted Successfully! \r\n");
  }
  pc.write	(SDTxBuffer, len);
//  set_spi_prsc	(SPI_PRSC_LCD);	//	max speed for lcd
//  HAL_Delay	(5000);
}


extern	int32_t	get_date	()	;	//	return binary. From Hi to Lo bytes - 0x00, Year, Month, Date
extern	RTC_HandleTypeDef hrtc;	//	RTC Real Time Clock

char *	get_date_delim_time	(char * dest, char * delim)	{
	RTC_TimeTypeDef	rtc_time;
	RTC_DateTypeDef	rtc_date;
	HAL_RTC_GetTime	(&hrtc, &rtc_time, RTC_FORMAT_BIN);
	HAL_RTC_GetDate	(&hrtc, &rtc_date, RTC_FORMAT_BIN);	//	Need this to make it work. I know!
	sprintf	(dest, "20%02d%02d%02d%s%02d%02d%02d", rtc_date.Year, rtc_date.Month, rtc_date.Date, delim, rtc_time.Hours, rtc_time.Minutes, rtc_time.Seconds);
	return	(dest);
}

char *	createfilename	(char * dest)	{	//	Filename is YearMonthDate-HourMinSec.log
	get_date_delim_time	((char*)dest, (char*)"-");
	strcat	(dest, ".log");
	return	dest;	//	Return newly created file name
}


FRESULT	new_csv_file	()	{	//	Picks up global TodaysLogFileName
//now global		FATFS		FatFs;			//	File system object structure
//now global		FRESULT 	FR_Status;		//	Integer 0 to 19
	FIL 		Fil;			//	File object structure
	char	t[64];
	int		len;
	pc.write	("In new_csv_file ", 16);
	do	{
	    FR_Status = f_mount(&FatFs, "", 1);
	    if (FR_Status != FR_OK)
	    {
			len = sprintf(t, "Error! While Mounting SD Card: (%i)\r\n", FR_Status);
			pc.write	(t, len);
			break;
	    }
//	    len = sprintf(SDTxBuffer, "SD Card Mounted Successfully! \r\n\n");
//	    pc.write	(SDTxBuffer, len);
	    FR_Status = f_open(&Fil, TodaysLogFileName, FA_WRITE | FA_READ | FA_CREATE_ALWAYS);
	    if(FR_Status != FR_OK)
	    {
			len = sprintf(t, "Error! While Creating/Opening A New Txt File, Code: (%i)\r\n", FR_Status);
			pc.write	(t, len);
			break;
	    }
//	    len = sprintf(SDTxBuffer, "Text File Created & Opened! Writing Data To The .csv File..\r\n\n");
//	    pc.write	(SDTxBuffer, len);
	    // (1) Write Data To The Text File [ Using f_puts() Function ]
	    f_puts("Baby Deltic 5914 Locomotive log file\n", &Fil);
	    // (2) Write Data To The Text File [ Using f_write() Function ]
//	    len = sprintf(t, "Hello! From STM32 To SD Card Over SPI, Using f_write()\r\n");
//	    f_write(&Fil, t, len, &WWC);
	    // Close The File
	    f_close(&Fil);

	}	while	(0);
	FR_Status = f_mount(NULL, "", 0);
	if (FR_Status != FR_OK)
	{
		len = sprintf(t, "Error! While Un-mounting SD Card, Error Code: (%i)\r\n", FR_Status);
	} else	{
		len = sprintf(t, "SD Card Un-mounted Successfully! \r\n");
	}
	pc.write	(t, len);
	return	(FR_Status);
}


FRESULT	file_erase	(char * filename)	{
//now global		FATFS		FatFs;			//	File system object structure
//now global		FRESULT 	FR_Status;		//	Integer 0 to 19
//	FATFS *	FS_Ptr;
	pc.write	("In erase_file(", 14);
	pc.write	(filename, strlen(filename));
	pc.write	("), ", 3);

	do	{
		FR_Status = f_mount(&FatFs, "", 1);
		if	(FR_Status != FR_OK)	{
	    	pc.write	("In file_erase, f_mount fail\r\n", 29);
    		break;
	    }
		FR_Status = f_unlink(filename);		//	This is the line that erases the file
		if	(FR_Status != FR_OK)	{
	    	pc.write	("In file_erase, f_unlink fail\r\n", 30);
//			break;
		}
		FR_Status = f_mount(NULL, "", 0);
		if (FR_Status != FR_OK)		{
	    	pc.write	("In file_erase, unmount fail\r\n", 29);
//			break;
		}
		else
			pc.write	("Success!\r\n", 10);
	}	while	(0);
	return	(FR_Status);
}


extern	int parse_wav_header(const uint8_t* buffer);


int	open_wav	(char * wavname)	{
//now global		FATFS		FatFs;			//	File system object structure
	FIL 		Fil;			//	File object structure
//now global		FRESULT 	FR_Status;		//	Integer 0 to 19
//	char	name[32] { 0 }	;
	char	t[122];
	char	buff[64];
	int		len;
//	int i = 0;
	UINT	RRC { 0 }	;
	len = sprintf	(t, "At open_wav, trying to open [%s]\r\n", wavname);
	pc.write	(t, len);
	do	{
		FR_Status = f_mount(&FatFs, "", 1);
		if	(FR_Status != FR_OK)	{
	    	len = sprintf	(t, "In open_wav, f_mount fail\r\n");
	    	pc.write	(t, len);
    		break;
	    }
	    FR_Status = f_open(&Fil, wavname, FA_READ); // Open The File For Read
		if	(FR_Status != FR_OK)	{
	    	len = sprintf	(t, "In open_wav could not open file to read\r\n");
	    	pc.write	(t, len);
    		break;
	    }
		pc.write	("Opened file for read\r\n", 22);
//	    f_read(&Fil, buff, f_size(&Fil), &RRC);
	    f_read(&Fil, buff, 50, &RRC);
	    parse_wav_header	((const uint8_t*)buff);
//	    len = sprintf(SDTxBuffer, "Data Read From (TextFileWrite.txt) After Update:%s", RW_Buffer);
//	    pc.write	(SDTxBuffer, len);
	    f_close(&Fil);
	}	while	(0);
	f_mount	(nullptr, "", 0);
	return	(FR_Status);
}


//	13th Aug 2026	-	Odometer stuff

//	Odometer data to be text readable and in .csv format. Can use 3 comma separated fields for Distance, Date, Time.
//	In this way we could record all together, or clock separate from distance.
//	Valid file lines could look like :-

//		,260813,140227\n			,Date, and Time only
//		123456,260813,140227\n		Distance, Date, and Time
//		123456\n					Distance only

//


FRESULT	get_odometer_distance	(uint32_t & odo_reading)	{	//	Read most recent odometer uint32_t from file
//now global		FATFS		FatFs;			//	File system object structure
//now global		FRESULT 	FR_Status = FR_OK;		//	Integer 0 to 19

	FIL		Fil;			//	File object structure
	char	t[122];
	char	buff[110]	{ 0 }	;
	int		len;
	int32_t	fsize;
	bool	gotnumstart	{ false }	;
	UINT	RRC { 0 }	;	//	for byte count read from file
	//	Open file "odometer.csv" for read.
	do	{
		FR_Status = f_mount(&FatFs, "", 1);
		if	(FR_Status != FR_OK)	{
	    	len = sprintf	(t, "In get_odo, f_mount fail\r\n");
	    	pc.write	(t, len);
    		break;
	    }
	    FR_Status = f_open(&Fil, "odometer.csv", FA_READ); // Open The File For Read
		if	(FR_Status != FR_OK)	{
	    	len = sprintf	(t, "In get_odo could not open file\r\n");
	    	pc.write	(t, len);
    		break;
	    }
		len = sprintf	(t, "\n\nOpened file for read, size %ld\r\n", f_size(&Fil));
		pc.write	(t, len);
		fsize = f_size(&Fil) - 100;
		if	(fsize < 0)
			fsize = 0;
		f_lseek	(&Fil, fsize);	//	look back to last 100 chars of file
	    FR_Status = f_read (&Fil, buff, fsize, &RRC);			/* Read data from the file */
	    pc.write	(buff, strlen(buff));
	    f_close	(&Fil);
	    f_mount	(NULL, "", 0);
	    odo_reading = 0;

	    while	(!gotnumstart)	{
	    	if	((RRC > 0) && (isdigit(buff[RRC])) && ('\n' == buff[RRC-1]))	{
	    		gotnumstart = true;
	    		odo_reading = atol	(buff + RRC);
	    	}
	    	else
	    		RRC -= 1;
	    	if	(RRC == 0)
	    		break;
	    }

//	    len = sprintf	(t, "Extracted [%ld]\r\n", odo_reading);
//	    pc.write	(t, len);	//	Yes, it works.

	}	while	(0);
	return	(FR_Status);
}



FRESULT	set_odometer_rtc	(char * t)	{	//	Set odometer Date and Time only
	get_date_delim_time	(t + 1, (char*)",");						//	To send ",20261122,103027"
	t[0] = ',';
	strcat	(t, "\n");
	return	(log_a_block((char*)"odometer.csv", (char*)t));
}


FRESULT	set_odometer_distance_only	(uint32_t	new_odo_total)	{	//	Set odometer distance only to absolute 'new_odo_total'
	char 	t[16];
	sprintf	(t, "%ld\n", new_odo_total);						//	To send "123456789\n"
	return	(log_a_block((char*)"odometer.csv", (char*)t));
}


FRESULT	set_odometer_all	(uint32_t	new_odo_total)	{	//	Set odometer Distance, Date, Time, and distance to absolute 'new_odo_total'
	char	t[32];
	int		pos = sprintf	(t, "%ld", new_odo_total);
	set_odometer_rtc	(t + pos);
	return	(log_a_block((char*)"odometer.csv", (char*)t));
}


bool	clr_odometer	()	{	//	Delete odometer file, replace with single line new file with distance zero
	file_erase	((char*)"odometer.csv");
	return	(true);
}


bool	update_odometer	(uint32_t	new_increment)	{	//
	return	(true);
}


extern	int	odo_bugger	(uint32_t dist)	{	//	Call from Utils to test odometer functions
	char	t[94];
	uint32_t	d;
	FRESULT	r1, r2, r3, r4;
	static uint32_t	t0, t1, t2, t3, t4;
	t0 = uwTick;
	r1 = set_odometer_all			(dist);
	t1 = uwTick - t0;
	r2 = set_odometer_distance_only	(dist);
	t2 = uwTick - t0;
////	r2 = set_odometer_distance_only	(dist + 13);
	r3 = set_odometer_rtc			(t);
	t3 = uwTick - t0;
	r4 = get_odometer_distance		(d);
	t4 = uwTick - t0;
	int	len = sprintf	(t, "In odo_bugger, get_odometer_distance found %ld, %ld, %ld, %ld, %ld\r\n", t1, t2, t3, t4, d);
	pc.write	(t, len);
	return	(0);
}

