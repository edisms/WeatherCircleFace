#include <pebble.h>
#include "message_handling.h"
#include "logging.h"

#define CB_COUNT 3

static AppMessageInboxReceived s_cbs[CB_COUNT];

static void inbox_received_handler(DictionaryIterator *iter, void *context);

void mh_init()
{
/*  app_message_deregister_callbacks();
  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(2026, 656);
*/
}

void mh_terminate()
{
   app_message_deregister_callbacks();
}
  

void mh_registerCallback(AppMessageInboxReceived cb)
{
  for (int i = 0; i < CB_COUNT; i++)
  {
    if (s_cbs[i] == 0 || s_cbs[i] == cb)
    {
      APP_I_LOG(APP_LOG_LEVEL_DEBUG, "storing %p", cb);
      s_cbs[i] = cb;
      app_message_register_inbox_received(inbox_received_handler);
      app_message_open(2026, 656);      
      return;
    }  
  }
}

void inbox_received_handler(DictionaryIterator *iter, void *context)
{
  APP_I_LOG(APP_LOG_LEVEL_DEBUG, "callback");
  for (int i = 0; i < CB_COUNT; i++)
  {
    if (s_cbs[i] != 0)
    {
      APP_I_LOG(APP_LOG_LEVEL_DEBUG, "calling %p", s_cbs[i]);
      s_cbs[i](iter, context);
    }  
  } 
}

