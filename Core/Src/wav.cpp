/*
 * wav.cpp
 *
 *  Created on: 8 Aug 2026
 *      Author: jon34
 */



#include <string.h>
#include <wav.hpp>
#include	"Serial.hpp"

extern	Serial	pc;

WavData	data;

//Utility function interpreting 4 bytes as 32-bit unsigned int
uint32_t get_uint32(const uint8_t* buf)
{
    int res = buf[0] + (buf[1] << 8) + (buf[2] << 16) + (buf[3] << 24);
    return res;
}

//Utility function interpreting 2 bytes as 16-bit unsigned int
uint16_t get_uint16(const uint8_t* buf)
{
    int res = buf[0] + (buf[1] << 8);
    return res;
}

//int parse_wav_header(const uint8_t* buffer, WavData* data)
int parse_wav_header(const uint8_t* buffer)
{
	char	t[128];
	int		len;
    //Check for WAVE file IDs to make sure we're parsing a valid WAV file
    int r = strncmp((const char*)buffer, "RIFF", 4);
    if (r != 0) {
        return -1;
    }

    r = strncmp((const char*)&buffer[8], "WAVE", 4);
    if (r != 0) {
        return -1;
    }

    uint32_t file_size = get_uint32(&buffer[4]);

    r = strncmp((const char*)&buffer[12], "fmt", 3);
    if (r != 0) {
        return -1;
    }

    //Save data to the WavData struct.
    data.fmt_chunk_size = get_uint32(&buffer[16]);
    data.format = (WaveFormat)get_uint16(&buffer[20]);
    data.n_channels = get_uint16(&buffer[22]);
    data.sample_rate = get_uint32(&buffer[24]);
    data.bits_per_sample = get_uint16(&buffer[34]);
    data.data_size = get_uint32(&buffer[40]);

    len = sprintf	(t, "fmt_chunk_size %ld\r\n", data.fmt_chunk_size);
    pc.write	(t, len);
    len = sprintf	(t, "format %ld\r\n", (int32_t)data.format);
    pc.write	(t, len);
    len = sprintf	(t, "n_channels %d\r\n", data.n_channels);
    pc.write	(t, len);
    len = sprintf	(t, "sample rate %ld\r\n", data.sample_rate);
    pc.write	(t, len);
    len = sprintf	(t, "bits per sample %d\r\n", data.bits_per_sample);
    pc.write	(t, len);
    len = sprintf	(t, "data size %ld\r\n", data.data_size);
    pc.write	(t, len);
    len = sprintf	(t, "file size %ld\r\n", file_size);
    pc.write	(t, len);

    return 1;
}



