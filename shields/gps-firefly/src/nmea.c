// NMEA parser - only support for GNGGA and its coordinates, for now ;-)
// constructed as a state machine

// example:
// $GNGGA,135507.000,5101.1610,N,00442.8453,E,...
// $.....,hhmmss.sss,ddmm.mmmm,[NS],ddmm.mmmm,[EW],...

#include <math.h>
#include <stdlib.h>

#include "nmea.h"

// local variable to store the results of parsing
static nmea_position_t position,
                       current_position;

nmea_position_t nmea_get_position() { return current_position;             }
uint8_t nmea_have_position()           { return current_position.quality > 0; }

void _reset_position(void) {
  position.time.hour     = 0;
  position.time.min      = 0;
  position.time.sec      = 0;
  position.latitude.deg  = 0;
  position.latitude.min  = 0;
  position.latitude.ns   = '_';
  position.longitude.deg = 0;
  position.longitude.min = 0;
  position.longitude.ew  = '_';
  position.quality=0;
}
void _reset_position2(void) {
	current_position.time.hour     = 0;
	current_position.time.min      = 0;
	current_position.time.sec      = 0;
	current_position.latitude.deg  = 0;
	current_position.latitude.min  = 0;
	current_position.latitude.ns   = '_';
	current_position.longitude.deg = 0;
  current_position.longitude.min = 0;
  current_position.longitude.ew  = '_';
  current_position.quality=0;
}

// states use parser functions to do parts of the parsing
// they return a result code indicating if they failed to parse, want to 
// continue parsing or are done for their part
// the codes are used as their int value 0, 1, 2 in the transitions table
// to choose the next state.
enum result_code { FAIL, REPEAT, OK };

// detects the start of a command
int find_start (uint8_t b) { return b == '$' ? OK : FAIL; }

// detects the GNGGA command
//$GNGGA,132322.000,5113.4908,N,00424.7987,E,1,6,1.23,-13.3,M,47.3,M,,*5B
int parse_gngga(uint8_t b) {
  static uint8_t pos = 0;
  const uint8_t string[] = "GNGGA,";

  // is it the expected next char?
  if(string[pos] == b) {
    if(pos == 5) {
      pos = 0;
      _reset_position();
      return OK;
    } else {
      pos++;
      return REPEAT;
    }
  }
  pos = 0;
  return FAIL;
}

// parses the UTC timestamp
int parse_time (uint8_t b) {
  static uint8_t pos = 0;
  switch(pos) {
    case 0: // Hhmmss.sss
    case 1: // hHmmss.sss
      if(b < '0' || b > '9') { pos = 0; return FAIL; }
      position.time.hour += (b - '0') * pow(10, (1-pos));
      break;
    case 2: // hhMmss.sss
    case 3: // hhmMss.sss
      if(b < '0' || b > '9') { pos = 0; return FAIL; }
      position.time.min += (b - '0') * pow(10, (3-pos));
      break;
    case 4: // hhmmSs.sss
    case 5: // hhmmsS.sss
      if(b < '0' || b > '9') { pos = 0; return FAIL; }
      position.time.sec += (b - '0') * pow(10, (5-pos));
      break;
    case 6: // skip .
      break;
    case 7: // hhmmss.Sss
    case 8: // hhmmss.sSs
    case 9: // hhmmss.ssS
      if(b < '0' || b > '9') { pos = 0; return FAIL; }
      position.time.sec += (b - '0') / pow(10, (pos - 6));
      break;
    case 10:
      pos = 0;
      if (b == ',')
        return OK;
      else
    	  return FAIL;

  }

  pos++;
  return REPEAT;

}

// parses the latitude information
int parse_latitude(uint8_t b) {
  static uint8_t pos = 0;
  switch(pos) {
    case 0: // Ddmm.mmmm
    case 1: // dDmm.mmmm
      if(b < '0' || b > '9') { pos = 0; return FAIL; }
      position.latitude.deg += (b - '0') * pow(10, (1-pos));
      break;
    case 2: // Ddmm.mmmm
    case 3: // dDmm.mmmm
      if(b < '0' || b > '9') { pos = 0; return FAIL; }
      position.latitude.min += (b - '0') * pow(10, (3-pos));
      break;
    case 4: break; // skip .
    case 5:
    case 6:
    case 7:
    case 8:
      if(b < '0' || b > '9') { pos = 0; return FAIL; }
      position.latitude.min += (b - '0') / pow(10, (pos - 4));
      break;
    case 9:
      if (b == ',')
      {
    	  pos = 0;
    	  return OK;
      }
      else
    	  break;
    case 10:
    	if(b < '0' || b > '9') { pos = 0; return FAIL; }
    	position.latitude.min += (b - '0') / pow(10, (pos - 6));
    	break;
    case 11:
    	pos = 0;
    	if (b == ',')
    	{
    	  	  return OK;
    	}
    	else
    	  	  return FAIL;


  }
  pos++;
  return REPEAT;

}

// parses the latitude orientation
int parse_ns_ind(uint8_t b) {
  if (b == ',') return OK;

  if(b != 'N' && b != 'S') { return FAIL; }
  position.latitude.ns = b;
  return REPEAT;
}

// parses the liongitude information
int parse_longitude(uint8_t b) {
  static uint8_t pos = 0;
  switch(pos) {
    case 0: // Dddmm.mmmm
      position.longitude.deg = 0;
      position.longitude.min = 0;
    case 1: // dDdmm.mmmm
    case 2: // ddDmm.mmmm
      if(b < '0' || b > '9') { pos = 0; return FAIL; }
      position.longitude.deg += (b - '0') * pow(10, (2-pos));
      break;
    case 3: // Ddmm.mmmm
    case 4: // dDmm.mmmm
      if(b < '0' || b > '9') { pos = 0; return FAIL; }
      position.longitude.min += (b - '0') * pow(10, (4-pos));
      break;
    case 5: break; // skip .
    case 6:
    case 7:
    case 8:
    case 9:
      if(b < '0' || b > '9') { pos = 0; return FAIL; }
      position.longitude.min += (b - '0') / pow(10, (pos - 5));
      break;
    case 10:
    	if (b == ',')
    	{
    		pos = 0;
    		return OK;
    	}
    	else
    		break;
    case 11:
    	if(b < '0' || b > '9') { pos = 0; return FAIL; }
    	position.longitude.min += (b - '0') / pow(10, (pos - 5));
    	break;
    case 12:
          pos = 0;
          if (b == ',')
            return OK;
          else
        	  return FAIL;

  }
  pos++;
  return REPEAT;
}

// parses the longitude orientation
int parse_ew_ind(uint8_t b) {
	if (b == ',') return OK;
	if(b != 'E' && b != 'W') { return FAIL; }
	position.longitude.ew = b;

	return REPEAT;
}

int parse_quality(uint8_t b) {
	if (b == ',') return OK;
	b -= '0';
	if(b > 8) { return FAIL; }
	position.quality = b;

	return REPEAT;
}

int parse_nr_sats(uint8_t b) {

	static uint8_t pos = 0;
	if (b == ',')
	{
		pos = 0;
		return OK;
	}

	if(b < '0' || b > '9') { pos = 0; return FAIL; }

	if (pos == 0)
	{
		position.nrofsats = (b - '0');
	}
	else
	{
		position.nrofsats *= 10;
		position.nrofsats += (b - '0');
	}

	pos++;
    return REPEAT;
}

int parse_hdop(uint8_t b) {
	static uint8_t pos = 0;
	static uint8_t dec = 0;

	if (b == ',')
	{
		pos = 0;
		dec = 0;
		// full parse, store current position
		current_position = position;
		return OK;
	}

	if (b == '.')
	{
		dec = 1;
		return REPEAT;
	}

	if(b < '0' || b > '9') { pos = 0; return FAIL; }

	if (pos == 0)
	{
		position.hdop = (b - '0');
		pos = 1;
	} else {
		if (dec == 0)
		{
			position.hdop *= 10;
			position.hdop += (b - '0');
		} else {
			position.hdop += (b - '0') / pow(10, dec);
			dec++;
		}
	}

	return REPEAT;
}

// function pointer for state_handlers
typedef int (* state_handler)(uint8_t);

enum state {
  start_state,
  gngga_state,
  time_state,
  latitude_state,
  ns_ind_state,
  longitude_state,
  ew_ind_state,
  quality_state,
  nr_sat_state,
  hdop_state
};

struct transition {
  state_handler handler;
  int result[3];
};

static struct transition transitions[] = {
  /* in state              what to look for   FAIL         REPEAT            OK               */
  /* -------------------   ----------------   -----------  ----------------  ---------------  */
  /* start_state */      { find_start,      { start_state, start_state,      gngga_state     } },
  /* gngga_state */      { parse_gngga,     { start_state, gngga_state,      time_state      } },
  /* time_state */       { parse_time,      { start_state, time_state,       latitude_state  } },
  /* latitude_state */   { parse_latitude,  { start_state, latitude_state,   ns_ind_state    } },
  /* ns_ind_state */     { parse_ns_ind,    { start_state, ns_ind_state,     longitude_state } },
  /* longitude_state */  { parse_longitude, { start_state, longitude_state,  ew_ind_state    } },
  /* ew_ind_state */     { parse_ew_ind,    { start_state, ew_ind_state,     quality_state   } },
  /* quality_state */    { parse_quality,   { start_state, quality_state,    nr_sat_state    } },
  /* nr_sat_state */     { parse_nr_sats,   { start_state, nr_sat_state,     hdop_state      } },
  /* hdop_state */       { parse_hdop,      { start_state, hdop_state,       start_state     } }
};

static int current_state = start_state;
  
void nmea_parse(uint8_t b) {
  if(b == '\n' || b == '\r') {   // IGNORES
    return;
  }
  int outcome   = transitions[current_state].handler(b);
  current_state = transitions[current_state].result[outcome];
}

