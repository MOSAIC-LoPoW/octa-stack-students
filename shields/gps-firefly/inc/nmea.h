// NMEA parser - only support for GPGGA and its coordinates, for now ;-)
// author: Christophe VG <contact@christophe.vg>

#include <stdint.h>

#ifndef __NMEA_H
#define __NMEA_H

// parser generates a struct containing the position
typedef struct nmea_position {
  struct {
    uint8_t hour;
    uint8_t min;
    float   sec;
  } time;
  struct {
    uint8_t deg;
    float   min;
    char    ns;
  } latitude;
  struct {
    uint8_t deg;
    float   min;
    char    ew;
  } longitude;
  uint8_t quality;
  float hdop;
  uint8_t nrofsats;
} nmea_position_t;

// TODO: to be implemented by top-level using functionality
extern void gps_position_handler(nmea_position_t pos);

void _reset_position(void);
void _reset_position2(void);

// feed the parser one byte at a time
void nmea_parse(uint8_t);

// function to explicitely get the current computed position
nmea_position_t nmea_get_position(void);

uint8_t nmea_have_position();

#endif
