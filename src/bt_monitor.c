#include <pebble.h>
#include "bt_monitor.h"
//static TextLayer *s_connection_layer;
static BitmapLayer *s_connection_layer;
static GBitmap *s_ok_bitmap;
static GBitmap *s_nok_bitmap;

static void handle_bluetooth(bool connected) {
  //text_layer_set_text(s_connection_layer, connected ? "connected" : "disconnected");
  APP_LOG(APP_LOG_LEVEL_DEBUG, connected ? "connected" : "disconnected");
  if (connected)
      bitmap_layer_set_bitmap(s_connection_layer, s_ok_bitmap);
  else
      bitmap_layer_set_bitmap(s_connection_layer, s_nok_bitmap);
}


void bt_monitor_init(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  //GRect bounds = layer_get_frame(window_layer);
  
  s_ok_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BLUETOOTH_OK);
  s_nok_bitmap= gbitmap_create_with_resource(RESOURCE_ID_BLUETOOTH_NOTOK);

  s_connection_layer = bitmap_layer_create(gbitmap_get_bounds(s_ok_bitmap));
  bitmap_layer_set_bitmap(s_connection_layer, s_ok_bitmap);
  bitmap_layer_set_compositing_mode(s_connection_layer, GCompOpSet);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_connection_layer));
  
  
  /*
  
  s_connection_layer = text_layer_create(GRect(0, 0, bounds.size.w, 34));
  text_layer_set_text_color(s_connection_layer, GColorBlack);
  text_layer_set_background_color(s_connection_layer, GColorClear);
  text_layer_set_font(s_connection_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_connection_layer, GTextAlignmentLeft);
  */
  
  // Load the image data
  //s_bitmap = gbitmap_create_with_resource(RESOURCE_ID_battery);
  
  // Get the bounds of the image
  //GRect bitmap_bounds = gbitmap_get_bounds(s_bitmap);
  
  // Set the compositing mode (GCompOpSet is required for transparency)
  //graphics_context_set_compositing_mode(ctx, GCompOpSet);
  
  // Draw the image
  //graphics_draw_bitmap_in_rect(ctx, s_bitmap, bitmap_bounds);  
  
  handle_bluetooth(connection_service_peek_pebble_app_connection());
  
    connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = handle_bluetooth
  });
   
  layer_add_child(window_layer, bitmap_layer_get_layer(s_connection_layer));
}


void bt_monitor_deinit(Window *window) {
  connection_service_unsubscribe();
  bitmap_layer_destroy(s_connection_layer);
  // Destroy the image data
  gbitmap_destroy(s_ok_bitmap);
  gbitmap_destroy(s_nok_bitmap);
}