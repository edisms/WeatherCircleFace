#include <pebble.h>
#include "bt_monitor.h"
#include "logging.h"

static BitmapLayer *s_connection_layer;
static GBitmap *s_ok_bitmap;
static GBitmap *s_nok_bitmap;
static bt_callback_* s_cb;

void bt_connection_status_monitor(bt_callback_ cb)
{
  s_cb = cb;
}

static void handle_bluetooth(bool connected) {
  APP_I_LOG(APP_LOG_LEVEL_DEBUG, connected ? "connected" : "disconnected");
  if (connected)
      bitmap_layer_set_bitmap(s_connection_layer, s_ok_bitmap);
  else
      bitmap_layer_set_bitmap(s_connection_layer, s_nok_bitmap);
  
  if (s_cb)
    s_cb(connected);
}


void bt_monitor_init(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  
  s_ok_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BLUETOOTH_OK);
  s_nok_bitmap= gbitmap_create_with_resource(RESOURCE_ID_BLUETOOTH_NOTOK);
  
  GRect bounds = gbitmap_get_bounds(s_ok_bitmap);
  GRect placement; 
  
  placement.origin.x = (34 - bounds.size.w)/2;
  placement.origin.y = (34 - bounds.size.h)/2;
  placement.size.w = bounds.size.w;
  placement.size.h = bounds.size.h;

  s_connection_layer = bitmap_layer_create(placement);
  
  bitmap_layer_set_bitmap(s_connection_layer, s_ok_bitmap);
  bitmap_layer_set_compositing_mode(s_connection_layer, GCompOpSet);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_connection_layer));
  
  handle_bluetooth(connection_service_peek_pebble_app_connection());
  
    connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = handle_bluetooth
  });
   
  layer_add_child(window_layer, bitmap_layer_get_layer(s_connection_layer));
  
  s_cb = 0;
}


void bt_monitor_deinit(Window *window) {
  s_cb = 0;
  
  connection_service_unsubscribe();
  bitmap_layer_destroy(s_connection_layer);
  // Destroy the image data
  gbitmap_destroy(s_ok_bitmap);
  gbitmap_destroy(s_nok_bitmap);
}