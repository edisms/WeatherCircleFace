#include <pebble.h>
#include "weather_circle.h"
#include "weather.h"

static void canvas_update_proc(Layer *layer, GContext *ctx);

static Layer *s_canvas_layer;
static GRect s_bounds_a;
static GRect s_bounds_b;
static GRect s_bounds_c;
static GRect s_bounds_d;


void init_weather(Window* window){
  s_bounds_a = layer_get_bounds(window_get_root_layer(window));
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "Original bounds %d, %d -  %d, %d", s_bounds_a.origin.x,s_bounds_a.origin.y, 
  //        s_bounds_a.size.h,s_bounds_a.size.w);  
  
  // Create canvas layer
  s_canvas_layer = layer_create(s_bounds_a);

  s_bounds_a.origin.x += 3;
  s_bounds_a.origin.y += 3;
  s_bounds_a.size.h -= 6;
  s_bounds_a.size.w -= 6;
  
  s_bounds_b = s_bounds_a;
  s_bounds_b.origin.x += 4;
  s_bounds_b.origin.y += 4;
  s_bounds_b.size.h -= 8;
  s_bounds_b.size.w -= 8;
  
  s_bounds_c = s_bounds_b;
  s_bounds_c.origin.x += 4;
  s_bounds_c.origin.y += 4;
  s_bounds_c.size.h -= 8;
  s_bounds_c.size.w -= 8;
  
  s_bounds_d = s_bounds_c;
  s_bounds_d.origin.x += 4;
  s_bounds_d.origin.y += 4;
  s_bounds_d.size.h -= 8;
  s_bounds_d.size.w -= 8;
  
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "New bounds %d, %d -  %d, %d", s_bounds.origin.x,s_bounds.origin.y, s_bounds.size.h,s_bounds.size.w);  
  
  // Assign the custom drawing procedure
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);

  // Add to Window
  layer_add_child(window_get_root_layer(window), s_canvas_layer);
  
  // Redraw this as soon as possible
  layer_mark_dirty(s_canvas_layer);
  
}

void redraw_weather()
{
  layer_mark_dirty(s_canvas_layer);  
}

void getSegmentData(int segment, int *temp, GColor *c_temp, int *rain, GColor *c_rain, 
                    int *wind, GColor *c_wind, int *cloud, GColor *c_cloud)
{
  int raw_temp;
  int raw_wind;
  int raw_rain;
  int raw_snow;
  int raw_cloud;
  
  /*
  int base_temp;
  int base_wind;
  int base_rain;
  int base_snow;
    
  weather_get_segment(0, &base_temp, &base_wind, &base_rain, &base_snow);  */   
  weather_get_segment(segment, &raw_temp, &raw_wind, &raw_rain, &raw_snow, &raw_cloud);    
  
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "seg %d, temp %d, wind %d, rain %d, cloud %d", segment, raw_temp, raw_wind, raw_rain, raw_cloud);
  
  *temp = 45;
  *rain = 45;
  *wind = 45;
  *cloud = 45;

  if (raw_temp < 0)
  {
    c_temp->argb = GColorBlueMoonARGB8;
  } else if (raw_temp < 5)
  {
    c_temp->argb = GColorVeryLightBlueARGB8;
  } else if (raw_temp < 10)
  {
    c_temp->argb = GColorYellowARGB8;
  } else if (raw_temp < 20)
  {
    c_temp->argb = GColorOrangeARGB8;
  } else 
  {
    c_temp->argb = GColorRedARGB8;
  } 
 
  if (raw_rain < 1)
  {
    *rain = 5;
  } else if (raw_rain < 2) {
    *rain = 10;    
  } else if (raw_rain < 4) {
    *rain = 15;    
  } else if (raw_rain < 8) {
    *rain = 20;    
  } else if (raw_rain < 16) {
    *rain = 25;    
  } else if (raw_rain < 32) {
    *rain = 30;    
  } else {
    *rain = 45;
  } 
  c_rain->argb = GColorBlueARGB8;
  
  if (raw_wind < 1)
    *wind = 5;
  else if (raw_wind < 5)
    *wind = 10;
  else if (raw_wind < 11)
    *wind = 15;
  else if (raw_wind < 19)
    *wind = 20;
  else if (raw_wind < 28)
    *wind = 25;
  else if (raw_wind < 38)
    *wind = 30;
  else if (raw_wind < 49)
    *wind = 35;
  else if (raw_wind < 61)
    *wind = 40;
  else 
    *wind = 45;
    
  if (raw_cloud < 10)
    *cloud = 5;
  else if (raw_cloud < 20)
    *cloud = 10;
  else if (raw_cloud < 30)
    *cloud = 15;
  else if (raw_cloud < 40)
    *cloud = 20;
  else if (raw_cloud < 50)
    *cloud = 25;
  else if (raw_cloud < 60)
    *cloud = 30;
  else if (raw_cloud < 70)
    *cloud = 35;
  else if (raw_cloud < 80)
    *cloud = 40;
  else 
    *cloud = 45;  

  c_cloud->argb = GColorDarkGrayARGB8;

}

void canvas_update_proc(Layer *layer, GContext *ctx) {
  //draw weather in 3 hour blocks
  if (!weather_is_ready())
  {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "weather data not ready yet");
    return;
  }
  
  int segment_size = 360/(24/3);
  
  for (int segment = 0; segment < 9; segment++){
    int temp;
    GColor c_temp;
    int rain;
    GColor c_rain;
    int wind; 
    GColor c_wind;
    int cloud; 
    GColor c_cloud;    
    getSegmentData(segment, &temp, &c_temp, &rain, &c_rain, &wind, &c_wind, &cloud, &c_cloud);
    graphics_context_set_stroke_width(ctx, 3);
    
    // calculate the centre angle of each segment, then draw centred on that point.
    int point = segment * segment_size;
    int point_l_t = point - (temp +1)/2;
    int point_r_t = point_l_t + temp;
    int point_l_w = point - (wind +1)/2;
    int point_r_w = point_l_w + wind;
    int point_l_r = point - (rain +1)/2;
    int point_r_r = point_l_r + rain;
    int point_l_c = point - (cloud +1)/2;
    int point_r_c = point_l_c + cloud;   
    
    //APP_LOG(APP_LOG_LEVEL_DEBUG,"Temp %d %d-> %d (0x%x), Wind %d %d-> %d (0x%x), Rain %d %d-> %d (0x%x)",
    //        temp, point_l_t, point_r_t, c_temp.argb, wind, point_l_w, point_r_w, c_wind.argb, rain, point_l_r, point_r_r, c_rain.argb);
    
    graphics_context_set_stroke_color(ctx, c_temp);
    graphics_draw_arc(ctx, s_bounds_a, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(point_l_t), DEG_TO_TRIGANGLE(point_r_t));
    graphics_context_set_stroke_color(ctx, c_wind);   
    graphics_draw_arc(ctx, s_bounds_b, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(point_l_w), DEG_TO_TRIGANGLE(point_r_w));
    graphics_context_set_stroke_color(ctx, c_rain);   
    graphics_draw_arc(ctx, s_bounds_c, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(point_l_r), DEG_TO_TRIGANGLE(point_r_r)); 
    graphics_context_set_stroke_color(ctx, c_cloud);   
    graphics_draw_arc(ctx, s_bounds_d, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(point_l_c), DEG_TO_TRIGANGLE(point_r_c)); 
    
    graphics_context_set_stroke_width(ctx, 2);
    graphics_context_set_stroke_color(ctx, GColorBlack);
    GPoint p0,p1;
    p0.x = 43;
    p0.y = 18;
    p1.x = 51;
    p1.y = 34;
    
    graphics_draw_line(ctx, p0, p1);
    
  }
}
  



