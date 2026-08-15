/*
 * Analog.cpp
 *
 *  Created on: Jan 24, 2024
 *      Author: Jon Freeman  B Eng (Hons) MIET
 */

//#include	"Project.hpp"	//	Where USING_ANALOG may be defined
#define	USING_ANALOG
#define	USING_ADC1

#ifdef	USING_ANALOG

#include	"main.h"
#include	"Serial.hpp"
#include	<cstdio>
#include	<cmath>

extern	Serial	pc;


typedef	uint16_t	SampSize	;
//typedef	uint32_t	SampSize	;

//#define	ADC1_buffsize_per_chan	(1 << 8)	//256	//	Average sum of half this numof samples
#define	ADC1_buffsize_per_chan	(1 << 5)	//32	//	Average sum of half this numof samples
//#define	ADC2_buffsize_per_chan	(1 << 5)	//32	//	Average sum of half this numof samples

#if defined	USING_ADC1
//extern	DMA_HandleTypeDef hdma_adc1;
extern	ADC_HandleTypeDef hadc1;	//1;
//enum	ADC1_Chans	{IU, IV, ADC1_LAST = IV}	;
enum	ADC1_Chans	{IV, ADC1_LAST = IV}	;

SampSize			ADC1_buff_L	[ADC1_buffsize_per_chan * (ADC1_Chans::ADC1_LAST + 1)] {0};	//	Ping pong buffer
SampSize  * const 	ADC1_buff_H	= ADC1_buff_L + (sizeof(ADC1_buff_L) / (sizeof(SampSize) * 2));

//uint32_t	adc1_sums	[(ADC1_Chans::ADC1_LAST + 1)] = {0L};		//	Store of sums of most recent 16 samples per value
uint32_t	adc1_sums	[3] = {0L};		//	Store of sums of most recent 16 samples per value

bool		adc1_buff_half_full_flag	= false	;
bool		adc1_buff_full_flag		= false	;
extern	int32_t	motI;

void	seeaddata	()	{
	char	t[96];
	int	len = sprintf	(t, "ADC %4x, %4x, %4x, %4x\r\n", ADC1_buff_L[0], ADC1_buff_L[1], ADC1_buff_L[2], ADC1_buff_L[3]);
	pc.write	(t, len);
}

void	adc1_summer	(SampSize * src)	{	//	scr points to currently not being written to half of ping pong buffer
	uint32_t	tmp	[ADC1_Chans::ADC1_LAST + 1] = {0L};
//	for	(int j = 0; j <= ADC_Chans::ADC_LAST; j++)
//		adc_sums[j] = 0L;
	for	(int i = 0; i < (ADC1_buffsize_per_chan >> 1); i++)
		for	(int j = 0; j <= ADC1_Chans::ADC1_LAST; j++)
			tmp[j] +=  *src++;
	for	(int j = 0; j <= ADC1_Chans::ADC1_LAST; j++)	{
		adc1_sums[j] = tmp[j];
	}
//	motI = adc1_sums[0];
}


uint32_t	adc_pk1	()	{
	uint32_t	rv = 0L;
	for	(int i = 0; i < ADC1_buffsize_per_chan; i++)
		if	(ADC1_buff_L[i] > rv)
			rv = ADC1_buff_L[i];
	return	(rv);
}

uint32_t	adc_trough1	()	{
	uint32_t	rv = 0xffffL;
	for	(int i = 0; i < ADC1_buffsize_per_chan; i++)
		if	(ADC1_buff_L[i] < rv)
			rv = ADC1_buff_L[i];
	return	(rv);
}

#endif



#if defined	USING_ADC2

//extern	DMA_HandleTypeDef hdma_adc2;
extern	ADC_HandleTypeDef hadc2;	//1;
enum	ADC2_Chans	{VLINK, TEMPERATURE, USER_POT, ADC2_LAST = USER_POT}	;

SampSize			ADC2_buff_L	[ADC2_buffsize_per_chan * (ADC2_Chans::ADC2_LAST + 1)] {0};	//	Ping pong buffer
SampSize  * const 	ADC2_buff_H	= ADC2_buff_L + (sizeof(ADC2_buff_L) / (sizeof(SampSize) * 2));
uint32_t	adc2_sums	[ADC2_Chans::ADC2_LAST + 1] {0L};		//	Store of sums of most recent 16 samples per value
bool		adc2_buff_half_full_flag	= false	;
bool		adc2_buff_full_flag		= false	;


void	adc2_summer	(SampSize * src)	{	//	scr points to currently not being written to half of ping pong buffer
	uint32_t	tmp	[ADC2_Chans::ADC2_LAST + 1] = {0L};
//	for	(int j = 0; j <= ADC_Chans::ADC_LAST; j++)
//		adc_sums[j] = 0L;
	for	(int i = 0; i < (ADC2_buffsize_per_chan >> 1); i++)
		for	(int j = 0; j <= ADC2_Chans::ADC2_LAST; j++)
			tmp[j] +=  *src++;
	for	(int j = 0; j <= ADC2_Chans::ADC2_LAST; j++)	{
		adc2_sums[j] = tmp[j];
	}
}
#endif

bool	adc_updates	()	{	//	Call this often
	bool	rv = false;		//	Returns true if update was due and performed, false when no update available
	if	(adc1_buff_half_full_flag)	{
		rv = true;
		adc1_buff_half_full_flag = false;
		adc1_summer	(ADC1_buff_L)	;
	}
	if	(adc1_buff_full_flag)	{
		rv = true;
		adc1_buff_full_flag = false;
		adc1_summer	(ADC1_buff_H)	;
	}
#if defined	USING_ADC2
	if	(adc2_buff_half_full_flag)	{
		rv = true;
		adc2_buff_half_full_flag = false;
		adc2_summer	(ADC2_buff_L)	;
	}
	if	(adc2_buff_full_flag)	{
		rv = true;
		adc2_buff_full_flag = false;
		adc2_summer	(ADC2_buff_H)	;
	}
#endif
	return	(rv);
}


bool	start_ADC	()	{	//	Inputs
	bool	rv = true;
	HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
#if defined	USING_ADC2
	HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED);
#endif
	if	(HAL_OK != HAL_ADC_Start_DMA(&hadc1, (uint32_t*)ADC1_buff_L, ADC1_buffsize_per_chan * (ADC1_Chans::ADC1_LAST + 1)))
		rv = false;
#if defined	USING_ADC2
	if	(HAL_OK != HAL_ADC_Start_DMA(&hadc2, (uint32_t*)ADC2_buff_L, ADC2_buffsize_per_chan * (ADC2_Chans::ADC2_LAST + 1)))
		rv = false;
#endif
	return	rv;
}


uint32_t	halfcnt1 = 0L;
uint32_t	fullcnt1 = 0L;
//uint32_t	halfcnt2 = 0L;
//uint32_t	fullcnt2 = 0L;
uint32_t	hadc_id = 0L;

void	HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef * hadc) 	{	//	Interrupt handler. Get out quick, do moves from Forever Loop
//void	HAL_ADC_ConvHalfCpltCallback(DMA_HandleTypeDef * hadc) 	{	//	Interrupt handler. Get out quick, do moves from Forever Loop
#ifdef	USING_ADC1
	if	(hadc == &hadc1)	{	//
//	if	(hadc == &hdma_adc1)	{	//
		adc1_buff_half_full_flag	= true;
		halfcnt1++;
	}
#endif
#ifdef	USING_ADC2
	if	(hadc == &hadc2)	{	//
//	if	(hadc == &hdma_adc2)	{	//
		adc2_buff_half_full_flag	= true;
		halfcnt2++;
	}
#endif
}


void	HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef * hadc) 	{	//	Interrupt handler. Get out quick, do moves from Forever Loop
#ifdef	USING_ADC1
	if	(hadc == &hadc1)	{	//
		adc1_buff_full_flag	= true;
		fullcnt1++;
	}
#endif
#ifdef	USING_ADC2
	if	(hadc == &hadc2)	{	//
		adc2_buff_full_flag	= true;
		fullcnt2++;
	}
#endif
//??	HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);	//	as good a place as any to toggle green Nucleo led
}

//void HAL_UART_DMATxCpltCallback(UART_HandleTypeDef *huart)	//	This called as well as HalfCplt
//void HAL_ADC_DMAConvCpltCallback(ADC_HandleTypeDef * hadc)	{	//	This called as well as HalfCplt
//	fullcnt2++;
//}





#ifdef	USING_RTC

extern	char * get_date	(char * dest)	;	//	get e.g. "16:50:11"
extern	char * get_time	(char * dest)	;	//	get e.g. "16:50:11"
extern	int32_t	get_date	()	;
extern	int32_t	get_time	()	;
extern	void	rtc_buggery	()	;

void	adc_cnt_report	()	{
	char	t[60] ;
	char	hms[20] ;
	int	len;
	int32_t	tim = get_date	();
//	len = sprintf	(t, "ADC Half cnt=%ld\r\n", halfcnt);
//	pc.write	(t, len);
//	len = sprintf	(t, "ADC Full cnt=%ld, ", fullcnt);
//	pc.write	(t, len);
	rtc_buggery	();
	get_time	(hms);
	tim = get_time();
	len = sprintf	(t, "%s, 0x%05lx\r\n", hms, tim);
	pc.write	(t, len);
	len = sprintf	(t, "A-D reading 0x%05lx, 0x%05lx, 0x%05lx 0x%05lx 0x%05lx\r\n", adc_sums[0], adc_sums[1], adc_sums[2], adc_sums[3], adc_sums[4]);
	pc.write	(t, len);
}
#endif


#if defined	TEMPERATURE

double	get_temperature_K	()	{	//	R40 = 4k7, NTC@20c = 10k
	//	um2850-getting-started-with-the-evspin32g4-evspin32g4nh-stmicroelectronics.pdf
	//	5.9	PCB temperature sensing
	constexpr	double	input_scaler = (3.3 / 65536.0);
	constexpr	double	resistor_ratio = 0.47;
	constexpr	double	Vref = 3.3;
	constexpr	double	NTC_beta = 3455.0;
	constexpr	double	one_over_298 = (1.0 / 298.0);

	double	v_in= ((double)adc2_sums[TEMPERATURE]) * input_scaler;				//	This is where temp average is built
	double	acc = log (((Vref / v_in) - 1.0) * resistor_ratio)	;	//
	acc /= NTC_beta;	//	div beta
	acc += one_over_298;
	return	(1.0 / acc);	//	Returns degrees Kelvin
}

double	get_temperature_C	()	{
	return	get_temperature_K() - 278.0;		//	Degrees K is 278 above degrees C
}
#endif


double	get_supply_voltage	()	{	//	Lithium cell voltage - approx 3.7V
	return	((double)adc1_sums[0]) / 14279.0;		//	Resistor dependent fiddle factor
}


//double	get_user_pot	()	{
//	return	((double)adc2_sums[USER_POT]) / 65536.0;	//	Because accumulated uint is 16 bit
//}

void	adc_cnt_report	()	{
	char	t[128] = {0};
//	char	hms[20] = {0};
	int	len;
//	uint32_t	ccer_copy = TIM1->CCER;
	len = sprintf	(t, "ADC1 Half cnt=%ld, ", halfcnt1);
	pc.write	(t, len);
	len = sprintf	(t, "ADC1 Full cnt=%ld, hadc=%lx\r\n", fullcnt1, hadc_id);
	pc.write	(t, len);
//	len = sprintf	(t, "Ch3=0x%08lx, Ch12=0x%08lx\r\n", adc1_sums[0], adc1_sums[1]);
	len = sprintf	(t, "Ch6=0x%08lx, Ch8=0x%08lx\r\n", adc1_sums[0], adc1_sums[1]);
	pc.write	(t, len);

//	len = sprintf	(t, "ADC2 Half cnt=%ld, ", halfcnt2);
//	pc.write	(t, len);
//	len = sprintf	(t, "ADC2 Full cnt=%ld\r\n", fullcnt2);
//	pc.write	(t, len);
//	len = sprintf	(t, "Vcc %2.3fv, Temp %2.3f, Pot. %1.3f, TIM1->CR2 0x%08lx, TIM1->BDTR 0x%08lx, TIM1->CCER 0x%08lx\r\n"
//			, get_supply_voltage(), get_temperature_C(), get_user_pot(), TIM1->CR2, TIM1->BDTR, TIM1->CCER);
//	len = sprintf	(t, "Vcc %2.3fv, Temp %2.3f, Pot. %1.3f\r\n"
//			, get_supply_voltage(), get_temperature_C(), get_user_pot());
//	TIM1->CR2 =	0x00003f00;	//	return to rm0440 p 1187
//	ccer_copy &= ~5;	//	reset bits 2 and 0 to 0
//	ccer_copy &= ~1;	//	reset bits 2 and 0 to 0
//	TIM1->CCER = ccer_copy;	// proved can do stuff with CCER bits
	//	CR2 Bits
	//	8	OIS1	output idle state 1
	//	9	OIS1N
	//	10	OIS2
	//	11	OIS2N
	//	12	OIS3
	//	13	OIS3N

	//	what of CCER
//	pc.write	(t, len);	//	BDTR 0x 0200 A040
//	len = sprintf	(t, "ADC1 0x%04x, 0x%04x, 0x%04x, 0x%04x, 0x%04x, 0x%04lx\r\n"
	len = sprintf	(t, "ADC1 0x%04x, 0x%04x\r\n"
		,	ADC1_buff_L[0],	ADC1_buff_L[1]);
	pc.write	(t, len);
}


	//	End of Inputs
#if 0
	//	Start of Outputs
extern	DAC_HandleTypeDef hdac1;


bool	start_DAC	()	{	//	Outputs
	bool	rv = true;
	if	(HAL_OK != HAL_DAC_Start	(&hdac1, DAC_CHANNEL_1))
			rv = false;
	if	(HAL_OK != HAL_DAC_Start	(&hdac1, DAC_CHANNEL_2))
			rv = false;
	return	(rv);
}


bool	DAC_write	(uint32_t outval, uint32_t Chan)	{	//	DAC output 1 or 2
	return	(HAL_OK == HAL_DAC_SetValue	(&hdac1, Chan, DAC_ALIGN_12B_R, outval));
}


bool	DAC_write	(float outvalnorm, uint32_t DAC_Chan)	{
	uint32_t	outval = (uint32_t)(outvalnorm * (1 << 12));
	if	(outval < 0L)
		outval = 0L;
	if	(outval >= (1 << 12))
		outval = (1 << 12) - 1;
	return	(DAC_write	(outval, DAC_Chan));
}
#endif
#endif


