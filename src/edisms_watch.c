#include <pebble.h>
#include "weather_circle.h"
#include "weather.h"
#include "bt_monitor.h"
#include "bat_monitor.h"
#include "health_monitor.h"
#include "logging.h"

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;

static void main_window_unload();
static void update_time();
static void init();
static void deinit();
static void main_window_load(Window *window);
static void main_window_unload(Window *window);
static void tick_handler(struct tm *tick_time, TimeUnits units_changed);
static void bt_callback(bool connected);

void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);

  static char s_date_buffer[11];
  strftime(s_date_buffer, sizeof(s_date_buffer), "%d/%m/%y", tick_time);  
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
  text_layer_set_text(s_date_layer, s_date_buffer);
}

void weather_callback()
{
  APP_I_LOG(APP_LOG_LEVEL_INFO, "weather_callback"); 
  redraw_weather();
}

void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Make sure the time is displayed from the start
  update_time();

  weather_init();
  
  weather_setup(weather_callback);
  
  weather_refresh();
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT|HOUR_UNIT, tick_handler);  
}

void deinit() {
  weather_deinit();
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}

void main_window_load(Window *window) {
  init_weather(window);
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create the TextLayer with specific bounds
  s_time_layer = text_layer_create(
      GRect(0, 65, bounds.size.w, 50));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_34_MEDIUM_NUMBERS));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  
  // Create the TextLayer with specific bounds
  s_date_layer = text_layer_create(
      GRect(0, 92, bounds.size.w, 50));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorBlack);
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24 ));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));  
  
  
  bt_monitor_init(window);
  bt_connection_status_monitor(bt_callback);
  bat_monitor_init(window);
  health_monitor_init(window);
}

void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  bt_monitor_deinit(window);
  bat_monitor_deinit(window);
  health_monitor_deinit(window);
}

void bt_callback(bool connected)
{
  if (connected)
  {
    weather_refresh();
    health_monitor_refreah();
  }
}
void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  
  APP_I_LOG(APP_LOG_LEVEL_INFO, "tick_handler minutes %d, units %d", tick_time->tm_min, units_changed);
  if (units_changed & HOUR_UNIT)
  {
    weather_refresh();
  }
  
  if ((tick_time->tm_min % 5 == 0) && !weather_is_ready())
  {
    weather_refresh();
  }
}

