#include "owm_weather.h"

typedef enum {
  OWMWeatherAppMessageKeyRequest = 0,
  OWMWeatherAppMessageKeyReply,
  OWMWeatherAppMessageKeyTime,
  OWMWeatherAppMessageKeySegment,   
  OWMWeatherAppMessageKeyConditionId,   
  OWMWeatherAppMessageKeyDescription,
  OWMWeatherAppMessageKeyDescriptionShort,
  OWMWeatherAppMessageKeyName,
  OWMWeatherAppMessageKeyTempK,
  OWMWeatherAppMessageKeyPressure,
  OWMWeatherAppMessageKeyWindSpeed,
  OWMWeatherAppMessageKeyWindDirection,
  OWMWeatherAppMessageKeyRain,
  OWMWeatherAppMessageKeySnow,
  OWMWeatherAppMessageKeyClouds,
  OWMWeatherAppMessageKeyBadKey,
  OWMWeatherAppMessageKeyLocationUnavailable
} OWMWeatherAppMessageKey;

//static OWMWeatherInfo *s_info;
static OWMWeatherCallback *s_callback;
static OWMWeatherStatus s_status;

static int s_info_count = 0;

static OWMWeatherInfo s_info_segments[OWM_WEATHER_MAX_SEGMENT_COUNT]; //! database of weather information

static char s_api_key[33];
static int s_base_app_key = 0;

static int get_app_key(OWMWeatherAppMessageKey key) {
  return (int) key + s_base_app_key;
}

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  OWMWeatherInfo *info = 0;
  Tuple *reply_tuple = dict_find(iter, get_app_key(OWMWeatherAppMessageKeyReply));
  
  if(reply_tuple) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Got reply with %ld", reply_tuple->value->int32);
  }
  
  if(reply_tuple && reply_tuple->value->int32 == 1) {
    Tuple *seg_tuple = dict_find(iter, get_app_key(OWMWeatherAppMessageKeySegment));
    info = &s_info_segments[seg_tuple->value->int32];
    
    info->segment = seg_tuple->value->int32;

    Tuple *time_tuple = dict_find(iter, get_app_key(OWMWeatherAppMessageKeyTime));
    info->forecast_time = time_tuple->value->uint32;
    
    //Tuple *desc_tuple = dict_find(iter, get_app_key(OWMWeatherAppMessageKeyDescription));
    strncpy(info->description, "blank", OWM_WEATHER_BUFFER_SIZE);
    //strncpy(info->description, desc_tuple->value->cstring, OWM_WEATHER_BUFFER_SIZE);

    //Tuple *desc_short_tuple = dict_find(iter, get_app_key(OWMWeatherAppMessageKeyDescriptionShort));
    //strncpy(info->description_short, desc_short_tuple->value->cstring, OWM_WEATHER_BUFFER_SIZE);
    strncpy(info->description_short, "blank", OWM_WEATHER_BUFFER_SIZE);

    Tuple *temp_tuple = dict_find(iter, get_app_key(OWMWeatherAppMessageKeyTempK));
    info->temp_k = temp_tuple->value->int32;
    info->temp_c = info->temp_k - 273;
    info->temp_f = ((info->temp_c * 9) / 5 /* *1.8 or 9/5 */) + 32;
    info->timestamp = time(NULL);

    Tuple *pressure_tuple = dict_find(iter, get_app_key(OWMWeatherAppMessageKeyPressure));
    info->pressure = pressure_tuple->value->int32;

    Tuple *wind_speed_tuple = dict_find(iter, get_app_key(OWMWeatherAppMessageKeyWindSpeed));
    info->wind_speed = wind_speed_tuple->value->int32;

    Tuple *wind_direction_tuple = dict_find(iter, get_app_key(OWMWeatherAppMessageKeyWindDirection));
    info->wind_direction = wind_direction_tuple->value->int32;

    Tuple *rain_direction_tuple = dict_find(iter, get_app_key(OWMWeatherAppMessageKeyRain));
    info->rain = rain_direction_tuple->value->int32;

    Tuple *snow_direction_tuple = dict_find(iter, get_app_key(OWMWeatherAppMessageKeySnow));
    info->snow = snow_direction_tuple->value->int32;

    Tuple *cloud_direction_tuple = dict_find(iter, get_app_key(OWMWeatherAppMessageKeyClouds));
    info->clouds = cloud_direction_tuple->value->int32;     
    
    APP_LOG(APP_LOG_LEVEL_INFO, "Segment %ld, time %lu, description %s, desc_short %s",
            seg_tuple->value->int32, info->forecast_time, info->description, info->description_short);
    APP_LOG(APP_LOG_LEVEL_INFO, ", temp %d, rain %d, snow %d, clouds %d",
      info->temp_c, info->rain, info->snow, info->clouds);        
    
    s_status = OWMWeatherStatusBuilding;
    if (info->segment > s_info_count){
      s_info_count = info->segment;
      s_callback(0, s_status);
    }
  }
  
  if(reply_tuple && reply_tuple->value->int32 == 2) {
    s_status = OWMWeatherStatusAvailable;
    app_message_deregister_callbacks();
    s_callback(0, s_status);
  }
  
  Tuple *err_tuple = dict_find(iter, get_app_key(OWMWeatherAppMessageKeyBadKey));
  if(err_tuple) {
    s_status = OWMWeatherStatusBadKey;
    s_callback(0, s_status);
  }

  err_tuple = dict_find(iter, get_app_key(OWMWeatherAppMessageKeyLocationUnavailable));
  if(err_tuple) {
    s_status = OWMWeatherStatusLocationUnavailable;
    s_callback(0, s_status);
  }
}

static void fail_and_callback() {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to send request!");
  s_status = OWMWeatherStatusFailed;
  s_callback(0, s_status);
}

static bool fetch() {
  DictionaryIterator *out;
  AppMessageResult result = app_message_outbox_begin(&out);
  if(result != APP_MSG_OK) {
    fail_and_callback();
    return false;
  }

  dict_write_cstring(out, get_app_key(OWMWeatherAppMessageKeyRequest), s_api_key);
  dict_write_int16(out, get_app_key(OWMWeatherAppMessageKeySegment), OWM_WEATHER_MAX_SEGMENT_COUNT);

  result = app_message_outbox_send();
  if(result != APP_MSG_OK) {
    fail_and_callback();
    return false;
  }

  s_status = OWMWeatherStatusPending;
  s_callback(0, s_status);
  return true;
}

void owm_weather_init_with_base_app_key(char *api_key, int base_app_key) {
  //if(s_info) {
  //  free(s_info);
  //}

  s_base_app_key = base_app_key;

  if(!api_key) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "API key was NULL!");
    return;
  }

  strncpy(s_api_key, api_key, sizeof(s_api_key));

  //s_info = (OWMWeatherInfo*)malloc(sizeof(OWMWeatherInfo));
  s_status = OWMWeatherStatusNotYetFetched;
}

void owm_weather_init(char *api_key) {
  owm_weather_init_with_base_app_key(api_key, 0);
}

bool owm_weather_fetch(OWMWeatherCallback *callback) {
  //if(!s_info) {
  //  APP_LOG(APP_LOG_LEVEL_ERROR, "OWM Weather library is not initialized!");
  //  return false;
  //}

  if(!callback) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "OWMWeatherCallback was NULL!");
    return false;
  }

  s_callback = callback;

  if(!bluetooth_connection_service_peek()) {
    s_status = OWMWeatherStatusBluetoothDisconnected;
    s_callback(0, s_status);
    return false;
  }

  app_message_deregister_callbacks();
  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(2026, 656);

  return fetch();
}

void owm_weather_deinit() {
  s_callback = NULL;
  s_info_count = 0;

}

OWMWeatherInfo* owm_weather_peek() {
  if ( s_status == OWMWeatherStatusAvailable){
    return &s_info_segments[0];
  }
  return NULL;
}

//! Peek at the current state of the weather library. You should check the OWMWeatherStatus of the
//! returned OWMWeatherInfo before accessing data members.
//! @param index the index of the weather info to read
//! @return OWMWeatherInfo object, internally allocated.
//! If NULL, owm_weather_init() has not been called.
OWMWeatherInfo* owm_weather_peek_index(int index){
  if ( s_status == OWMWeatherStatusAvailable){
    if (index <= s_info_count)
      return &s_info_segments[index];
  }
  return NULL;  
}

//! Peek at the current state of the weather library. 
//! @return size of owm segments
int owm_weather_count()
{
  return s_info_count;
}
