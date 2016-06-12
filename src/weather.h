#pragma once


//! Callback for a weather 
typedef void(weather_callback_)();

void weather_init();

void weather_deinit();

void weather_setup(weather_callback_* cb);

void weather_refresh(bool clear);

bool weather_is_ready();
  
void weather_get_segment(int segment, int* temp, int* wind, int* rain, int* snow, int* cloud);  

int weather_get_segment_time(int segment);

const char *weather_location();

bool weather_sun(int* rise, int* set);
