#include <pebble.h>
#include "weather.h"
#include "logging.h"

#include "owm_weather/owm_weather.h"

static bool s_ready = false;

static void weather_callback(int segment, OWMWeatherStatus status);
  
static weather_callback_* s_cb;

static void js_ready_handler(void *context) {
  owm_weather_fetch(weather_callback);
}

void weather_init()
  {
      // Replace this with your own API key from OpenWeatherMap.org
  //char *api_key = "f402bfb5a34389b2501c3e7007b46668";
  owm_weather_init();
}

void weather_deinit()
{
  owm_weather_deinit();
  s_cb = 0;
  s_ready = false;
}

void weather_setup(weather_callback_* cb)
{
  s_cb = cb;  
}

void weather_refresh(bool clear)
{
  if (clear)
  {
    owm_weather_reset_data();
  }
  app_timer_register(3000, js_ready_handler, NULL);
}
  
void weather_get_segment(int segment, int* temp, int* wind, int* rain, int* snow, int* cloud)
{
  const OWMWeatherInfo* owm = owm_weather_peek_index(segment);
  *temp = owm->temp_c;
  *wind = owm->wind_speed;
  *rain = owm->rain;
  *snow = owm->snow;
  *cloud = owm->clouds;
}

const char *weather_location()
{
  bool ok;
  const char *message = owm_weather_state(&ok);
  if (ok)
  {
    const OWMWeatherLocationInfo* owm = owm_weather_location_peek();
    //APP_I_LOG(APP_LOG_LEVEL_DEBUG, "Peeked location %s", owm->name);
    return owm->name;
  }
  APP_I_LOG(APP_LOG_LEVEL_DEBUG, "No name?");
  return message;
}

bool weather_sun(int* rise, int *set){
  bool ok;
  owm_weather_state(&ok);
  if (ok)
  {
    const OWMWeatherLocationInfo* owm = owm_weather_location_peek();
    *rise = owm->sunrise;
    *set = owm->sunset;
  }
  return ok;  
}

void weather_callback(int segment, OWMWeatherStatus status) {
  switch(status) {
    case OWMWeatherStatusAvailable:
    {
      //OWMWeatherInfo *info = owm_weather_peek_index(0);
      //APP_LOG(APP_LOG_LEVEL_INFO,
      //  "Temperature (K/C/F): %d/%d/%d\n\nDescription/short:\n%s/%s\n\nPressure: %d\n\nWind speed/dir: %d/%d",
      //  info->temp_k, info->temp_c, info->temp_f, info->description,
      //  info->description_short, info->pressure, info->wind_speed, info->wind_direction);
      
      s_ready = true;
      if (s_cb)
      {
        s_cb();  
      }
    }
      break;
    case OWMWeatherStatusBuilding:
      APP_I_LOG(APP_LOG_LEVEL_INFO,  "OWMWeatherStatusBuilding");
      break;
    case OWMWeatherStatusNotYetFetched:
      APP_I_LOG(APP_LOG_LEVEL_INFO,  "OWMWeatherStatusNotYetFetched");
      break;
    case OWMWeatherStatusBluetoothDisconnected:
      APP_I_LOG(APP_LOG_LEVEL_INFO,  "OWMWeatherStatusBluetoothDisconnected");
      break;
    case OWMWeatherStatusPending:
      APP_I_LOG(APP_LOG_LEVEL_INFO, "OWMWeatherStatusPending");
      break;
    case OWMWeatherStatusFailed:
      APP_I_LOG(APP_LOG_LEVEL_INFO,  "OWMWeatherStatusFailed");
      break;
    case OWMWeatherStatusBadKey:
      APP_I_LOG(APP_LOG_LEVEL_INFO,  "OWMWeatherStatusBadKey");
      break;
    case OWMWeatherStatusLocationUnavailable:
      APP_I_LOG(APP_LOG_LEVEL_INFO,  "OWMWeatherStatusLocationUnavailable");
      break;
  }
}

bool weather_is_ready()
{
  return s_ready;
}

int weather_get_segment_time(int segment)
{
  const OWMWeatherInfo* owm = owm_weather_peek_index(segment);
  return owm->forecast_time;
}

