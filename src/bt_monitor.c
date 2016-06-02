#include <pebble.h>
#include "bt_monitor.h"
static TextLayer *s_connection_layer;

static void handle_bluetooth(bool connected) {
  text_layer_set_text(s_connection_layer, connected ? "connected" : "disconnected");
  APP_LOG(APP_LOG_LEVEL_DEBUG, connected ? "connected" : "disconnected");
}


void bt_monitor_init(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
  
  s_connection_layer = text_layer_create(GRect(0, 0, bounds.size.w, 34));
  text_layer_set_text_color(s_connection_layer, GColorBlack);
  text_layer_set_background_color(s_connection_layer, GColorClear);
  text_layer_set_font(s_connection_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_connection_layer, GTextAlignmentLeft);
  handle_bluetooth(connection_service_peek_pebble_app_connection());
  
    connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = handle_bluetooth
  });
  
  layer_add_child(window_layer, text_layer_get_layer(s_connection_layer));
}


void bt_monitor_deinit(Window *window) {
  connection_service_unsubscribe();
  text_layer_destroy(s_connection_layer);
}