#include <pebble.h>
#include "bat_monitor.h"

static GBitmap *s_battery_bitmap;
static Layer *s_canvas_layer;
static GRect s_bounds;

static void handle_battery(BatteryChargeState charge_state) {
  layer_mark_dirty(s_canvas_layer); 
}

static void canvas_update_proc(Layer *layer, GContext *ctx);

void bat_monitor_init(Window *window) {
  Layer *window_layer = window_get_root_layer(window);

  s_battery_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_FRAME);
  s_bounds = gbitmap_get_bounds(s_battery_bitmap);
  s_bounds.origin.x = 14;
  
  // Create canvas layer
  s_canvas_layer = layer_create(s_bounds);
  
  battery_state_service_subscribe(handle_battery);
  
  layer_add_child(window_layer, s_canvas_layer);
    
  // Assign the custom drawing procedure
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);  
  layer_mark_dirty(s_canvas_layer); 
}

void canvas_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_compositing_mode(ctx, GCompOpSet);
  // draw png of battery
  graphics_draw_bitmap_in_rect(ctx, s_battery_bitmap, gbitmap_get_bounds(s_battery_bitmap));
  
  // draw rect to indicate battery level.
  // x, 7, 4, y= 5
  // green shows the battery level, when it drops to 20% put red in the background.
  graphics_context_set_fill_color(ctx, GColorGreen);

  
  BatteryChargeState bcs = battery_state_service_peek();
  int offset = 10 - bcs.charge_percent/10;
  GRect rect;
  rect.origin.x = 6;
  rect.origin.y = 5 + offset;
  rect.size.w = 4;
  rect.size.h = 10 - offset;
  
  graphics_fill_rect(ctx, rect, 0, GCornerNone);
  
  if (bcs.charge_percent <= 20)
  {
    rect.origin.x = 6;
    rect.origin.y = 5;
    rect.size.w = 4;
    rect.size.h = offset;
    
    graphics_context_set_fill_color(ctx, GColorRed);
    graphics_fill_rect(ctx, rect, 0, GCornerNone);    
  }
}

void bat_monitor_deinit(Window *window) {
  battery_state_service_unsubscribe();
  layer_destroy(s_canvas_layer);
  gbitmap_destroy(s_battery_bitmap);
}

