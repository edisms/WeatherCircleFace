#pragma once

#include <pebble.h>

#define OWM_WEATHER_BUFFER_SIZE 32

#define OWM_WEATHER_MAX_SEGMENT_COUNT 11

//! Possible statuses of the weather library
typedef enum {
  //! Weather library has not yet initiated a fetch
  OWMWeatherStatusNotYetFetched = 0,
  //! Bluetooth is disconnected
  OWMWeatherStatusBluetoothDisconnected,
  //! Weather data fetch is in progress
  OWMWeatherStatusPending,
  //! Weather fetch failed
  OWMWeatherStatusFailed,
  //! Some weather data collected but not yet all available
  OWMWeatherStatusBuilding,
  //! Weather fetched and available
  OWMWeatherStatusAvailable,
  //! API key was bad
  OWMWeatherStatusBadKey,
  //! Location not available
  OWMWeatherStatusLocationUnavailable
} OWMWeatherStatus;

//! Struct containing weather data
typedef struct {
  //! Index/ segment id
  int segment;
  //! OpenWeather general description of the weather
  int condition_id;      
  //! Weather conditions string e.g: "Sky is clear"
  //char description[OWM_WEATHER_BUFFER_SIZE];
  //! Short conditions string e.g: "Clear"
  //char description_short[OWM_WEATHER_BUFFER_SIZE];
  //! Name of the location from the weather feed
  //char name[OWM_WEATHER_BUFFER_SIZE];
  //! Temperature in degrees Kelvin, Celcius, and Farenheit
  int temp_k;
  int temp_c;
  int temp_f;
  //! Air pressure in millibars
  int pressure;
  //! Wind speed in kph
  int wind_speed;
  //! Wind direction in meteorological degrees
  //int wind_direction;
  //! Date that the data was received
  time_t timestamp;
  //! Date that the weather represents in unix time
  int forecast_time;
  //! % cloud cover
  int clouds;
  //! rain in mm
  int rain;
  //! snow in mm
  int snow;
  
} OWMWeatherInfo;

typedef struct {
  char name[OWM_WEATHER_BUFFER_SIZE]; //! name of current location
  int sunrise; //! unixtime sunrise
  int sunset; //! unixtime sunset
} OWMWeatherLocationInfo;

//! Callback for a weather fetch
//! @param segment The segment id for which data has been received
//! @param status The current OWMWeatherStatus, which may have changed.
typedef void(OWMWeatherCallback)(int segment, OWMWeatherStatus status);

//! Initialize the weather library. The data is fetched after calling this, and should be accessed
//! and stored once the callback returns data, if it is successful.
//! @param api_key The API key or 'appid' from your OpenWeatherMap account.
void owm_weather_init(char *api_key);

//! Initialize the weather library with a base AppMessage key.
//! The data is fetched after calling this, and should be accessed
//! and stored once the callback returns data, if it is successful.
//! @param api_key The API key or 'appid' from your OpenWeatherMap account.
//! @param base_app_key The AppKey base to use
void owm_weather_init_with_base_app_key(char *api_key, int base_app_key);

//! Important: This uses the AppMessage system. You should only use AppMessage yourself
//! either before calling this, or after you have obtained your weather data.
//! @param callback Callback to be called once the weather.
//! @return true if the fetch message to PebbleKit JS was successful, false otherwise.
bool owm_weather_fetch(OWMWeatherCallback *callback);

//! Deinitialize and free the backing OWMWeatherInfo.
void owm_weather_deinit();

//! Peek at the current state of the weather library. You should check the OWMWeatherStatus of the
//! returned OWMWeatherInfo before accessing data members.
//! @return first OWMWeatherInfo object, internally allocated.
//! If NULL, owm_weather_init() has not been called.
OWMWeatherInfo* owm_weather_peek();

//! Peek at the current location state
//! @return the OWMWeatherLocationInfo, internally allocated.
//! If NULL, owm_weather_init() has not been called or the data is not ready.
OWMWeatherLocationInfo* owm_weather_location_peek();

//! Peek at the current state of the weather library. You should check the OWMWeatherStatus of the
//! returned OWMWeatherInfo before accessing data members.
//! @param index the index of the weather info to read
//! @return OWMWeatherInfo object, internally allocated.
//! If NULL, owm_weather_init() has not been called.
OWMWeatherInfo* owm_weather_peek_index(int index);

//! Peek at the current state of the weather library. 
//! @return size of owm segments
int owm_weather_count();

//! Peek at the current state of the weather library. if not ok returns an error string
//! @param ok returns true if state ok, false otherwise.
//! @return error message
const char* owm_weather_state(bool* ok); 
