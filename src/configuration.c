#include <pebble.h>
#include "configuration.h"
#include "message_handling.h"
#include "logging.h"

static void inbox_received_handler(DictionaryIterator *iter, void *context);

#define API_KEY_BUFFER 64
#define DEFAULT_API_KEY "f402bfb5a34389b2501c3e7007b46668"
static char s_api_key[API_KEY_BUFFER];
static bool s_use_default_key = true;
static bool s_use_fake_weather = false;
static configuration_callback_* s_cb;

#define KEY_API_KEY 1000
#define KEY_DEFAULT_API_KEY (KEY_API_KEY + 1)
#define KEY_USE_FAKE_WEATHER (KEY_API_KEY + 2)

static void set_key()
{
  if (persist_exists(KEY_DEFAULT_API_KEY))
  {
    s_use_default_key = persist_read_bool(KEY_DEFAULT_API_KEY);
    if (!s_use_default_key)
    {
      if (!persist_exists(KEY_API_KEY))
      {
        s_use_default_key = true;
      }
    }
  }
  
  if (!s_use_default_key)
  {
    int res = persist_read_string(KEY_API_KEY, s_api_key, API_KEY_BUFFER);
    if (res < 10)
    {
      s_use_default_key = true;
    }
  }
  
  if (s_use_default_key)
  {
    strncpy(s_api_key, DEFAULT_API_KEY, (strlen(DEFAULT_API_KEY)));
  } 
}

void configuration_init()
{
  mh_registerCallback(inbox_received_handler);
  
  set_key();
 
  if (persist_exists(KEY_USE_FAKE_WEATHER))
  {
    s_use_fake_weather = persist_read_bool(KEY_USE_FAKE_WEATHER);
  }
  
}

void inbox_received_handler(DictionaryIterator *iter, void *context)
{
  Tuple *API_tuple = dict_find(iter, KEY_API_KEY);
  Tuple *default_API_tuple = dict_find(iter, KEY_DEFAULT_API_KEY);
  Tuple *fake_weather_tuple = dict_find(iter, KEY_USE_FAKE_WEATHER);
  
  if (!API_tuple && !default_API_tuple && !fake_weather_tuple)
  {
    APP_I_LOG(APP_LOG_LEVEL_INFO, "No configuration data");
    return;
  }
    
  if(API_tuple) {
    strncpy(s_api_key, API_tuple->value->cstring, API_KEY_BUFFER);
    APP_I_LOG(APP_LOG_LEVEL_INFO, "Got api key %s", API_tuple->value->cstring);
    persist_write_string(KEY_API_KEY, API_tuple->value->cstring);
  }
 
  if(default_API_tuple) {
    APP_I_LOG(APP_LOG_LEVEL_INFO, "use default key %ld", default_API_tuple->value->int32);
    s_use_default_key = default_API_tuple->value->int32 == 1 ? true : false;
    persist_write_bool(KEY_DEFAULT_API_KEY, s_use_default_key);
  }
  
  if(fake_weather_tuple) {
    APP_I_LOG(APP_LOG_LEVEL_INFO, "use fake weather %ld", fake_weather_tuple->value->int32);
    s_use_fake_weather = fake_weather_tuple->value->int32 == 1 ? true : false;
    persist_write_bool(KEY_USE_FAKE_WEATHER, s_use_fake_weather);
  }  
    
  set_key();
  
  if (s_cb)
  {
    s_cb();
  }
}

const char* configuration_get_api_key()
{
  return s_api_key;
}

bool configuration_use_fake_weather()
{
  return s_use_fake_weather;
}

void configuration_set_change_callback(configuration_callback_* cb)
{
  s_cb = cb;
}
