#include <pebble.h>
#include "bat_monitor.h"
#include "logging.h"

static GBitmap *s_battery_bitmap;
static GBitmap *s_charging_bitmap;
static Layer *s_bat_layer;
static GRect s_bat_bounds;

static void handle_battery(BatteryChargeState charge_state) {
  layer_mark_dirty(s_bat_layer);
}

static void bat_update_proc(Layer *layer, GContext *ctx);

void bat_monitor_init(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  s_bat_bounds = layer_get_bounds(window_get_root_layer(window));
  s_bat_bounds.origin.x = s_bat_bounds.size.w - 34;
  s_bat_bounds.size.w = 34;
  s_bat_bounds.size.h = 34;

  s_battery_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_NORMAL);
  s_charging_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_CHARGING);
  
  // Create canvas layer
  s_bat_layer = layer_create(s_bat_bounds);
  
  battery_state_service_subscribe(handle_battery);
  
  layer_add_child(window_layer, s_bat_layer);
    
  // Assign the custom drawing procedure
  layer_set_update_proc(s_bat_layer, bat_update_proc);  
  layer_mark_dirty(s_bat_layer);   
  
}

void bat_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_compositing_mode(ctx, GCompOpSet);
  GRect bounds;
  bounds.origin.x = 1;
  bounds.origin.y = 1;
  bounds.size.w = 30;
  bounds.size.h = 30;
  
  APP_I_LOG(APP_LOG_LEVEL_DEBUG,"bat_update_proc %d,%d %d,%d",  s_bat_bounds.origin.x, 
          s_bat_bounds.origin.y,s_bat_bounds.size.w, s_bat_bounds.size.h);
  
  graphics_context_set_stroke_width(ctx, 6);
  
  BatteryChargeState bcs = battery_state_service_peek();
  
  graphics_context_set_fill_color(ctx, GColorGreen);
  graphics_fill_radial(ctx, bounds, GOvalScaleModeFitCircle, 5, DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE((bcs.charge_percent/10)*36));
  if (bcs.charge_percent > 20)
    graphics_context_set_fill_color(ctx, GColorYellow);
  else
    graphics_context_set_fill_color(ctx, GColorRed);
  graphics_fill_radial(ctx, bounds, GOvalScaleModeFitCircle, 5, DEG_TO_TRIGANGLE((bcs.charge_percent/10)*36), DEG_TO_TRIGANGLE(360));
  
  GRect icon_bounds = gbitmap_get_bounds(s_battery_bitmap);
  GRect placement;
  
  placement.origin.x = (s_bat_bounds.size.w - icon_bounds.size.w)/2;
  placement.origin.y = (s_bat_bounds.size.h - icon_bounds.size.h)/2;
  placement.size.w = icon_bounds.size.w;
  placement.size.h = icon_bounds.size.h;
  
  if (bcs.is_charging)
    graphics_draw_bitmap_in_rect(ctx, s_charging_bitmap, placement);
  else
    graphics_draw_bitmap_in_rect(ctx, s_battery_bitmap, placement);
}


void bat_monitor_deinit(Window *window) {
  battery_state_service_unsubscribe();
  layer_destroy(s_bat_layer);
  gbitmap_destroy(s_battery_bitmap);
  gbitmap_destroy(s_charging_bitmap);
}

