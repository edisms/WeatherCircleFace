#include <pebble.h>
#include "weather_circle.h"

static void canvas_update_proc(Layer *layer, GContext *ctx);

static Layer *s_canvas_layer;
static GRect s_bounds;

void init_weather(Window* window){
  s_bounds = layer_get_bounds(window_get_root_layer(window));
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Original bounds %d, %d -  %d, %d", s_bounds.origin.x,s_bounds.origin.y, s_bounds.size.h,s_bounds.size.w);  
  
  // Create canvas layer
  s_canvas_layer = layer_create(s_bounds);

  s_bounds.origin.x += 3;
  s_bounds.origin.y += 3;
  s_bounds.size.h -= 6;
  s_bounds.size.w -= 6;
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "New bounds %d, %d -  %d, %d", s_bounds.origin.x,s_bounds.origin.y, s_bounds.size.h,s_bounds.size.w);  
  
  // Assign the custom drawing procedure
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);

  // Add to Window
  layer_add_child(window_get_root_layer(window), s_canvas_layer);
  
  // Redraw this as soon as possible
  layer_mark_dirty(s_canvas_layer);
  
}

void canvas_update_proc(Layer *layer, GContext *ctx) {
  //draw weather in 3 hour blocks
  int segment = 360/(24/3);
  int i;
  uint8_t colours[] = {GColorVeryLightBlueARGB8, GColorCobaltBlueARGB8, GColorYellowARGB8, GColorDarkGrayARGB8, 
                       GColorLightGrayARGB8, GColorRedARGB8 , GColorBlueMoonARGB8, GColorLightGrayARGB8 };
  for (i = 0; i < 360; i+= segment)
  {
    graphics_context_set_stroke_width(ctx, 3);
    
    GColor c;
    c.argb = colours[i/segment];
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Drawing segment %d to %d in %d (%d)", i, i+ segment, c.argb, i/segment);
    graphics_context_set_stroke_color(ctx, c);
    graphics_draw_arc(ctx, s_bounds, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(i), DEG_TO_TRIGANGLE(i+segment));
  }
}

