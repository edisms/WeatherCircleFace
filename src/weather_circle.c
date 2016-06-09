#include <pebble.h>
#include "weather_circle.h"
#include "weather.h"
#include "logging.h"

static void canvas_update_proc(Layer *layer, GContext *ctx);

static Layer *s_canvas_layer;
static GRect s_bounds;
static GRect s_bounds_a;
static GRect s_bounds_b;
static GRect s_bounds_c;
static GRect s_bounds_d;

static TextLayer *s_location_layer;

void init_weather(Window* window){
  s_bounds = layer_get_bounds(window_get_root_layer(window));
  s_bounds_a = s_bounds;
  //APP_I_LOG(APP_LOG_LEVEL_DEBUG, "Original bounds %d, %d -  %d, %d", s_bounds_a.origin.x,s_bounds_a.origin.y, 
  //        s_bounds_a.size.h,s_bounds_a.size.w);  
  
  // Create canvas layer
  s_canvas_layer = layer_create(s_bounds_a);

  s_bounds_a.origin.x += 3;
  s_bounds_a.origin.y += 3;
  s_bounds_a.size.h -= 6;
  s_bounds_a.size.w -= 6;
  
  s_bounds_b = s_bounds_a;
  s_bounds_b.origin.x += 5;
  s_bounds_b.origin.y += 5;
  s_bounds_b.size.h -= 10;
  s_bounds_b.size.w -= 10;
  
  s_bounds_c = s_bounds_b;
  s_bounds_c.origin.x += 5;
  s_bounds_c.origin.y += 5;
  s_bounds_c.size.h -= 10;
  s_bounds_c.size.w -= 10;
  
  s_bounds_d = s_bounds_c;
  s_bounds_d.origin.x += 5;
  s_bounds_d.origin.y += 5;
  s_bounds_d.size.h -= 10;
  s_bounds_d.size.w -= 10;
  
  // Assign the custom drawing procedure
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);

  // Add to Window
  layer_add_child(window_get_root_layer(window), s_canvas_layer);
  
  
  // add the current location
  GRect b = layer_get_bounds(window_get_root_layer(window));
  s_location_layer = text_layer_create(
      GRect((b.size.w-70)/2, 45, 70, 40));
  text_layer_set_background_color(s_location_layer, GColorClear);
  text_layer_set_text_color(s_location_layer, GColorBlack);
  text_layer_set_font(s_location_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14 ));
  text_layer_set_text_alignment(s_location_layer, GTextAlignmentCenter);  
  text_layer_set_overflow_mode(s_location_layer,GTextOverflowModeTrailingEllipsis);
  
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_location_layer));
  
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
  
  APP_I_LOG(APP_LOG_LEVEL_DEBUG, "seg %d, temp %d, wind %d, rain %d, cloud %d", segment, raw_temp, raw_wind, raw_rain, raw_cloud);
  
  *temp = 43;
  *rain = 45;
  *wind = 45;
  *cloud = 45;
  
  raw_rain += raw_snow;

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
    *c_temp = GColorChromeYellow;
  } else 
  {
    c_temp->argb = GColorRedARGB8;
  } 
 
  if (raw_rain < 1)
  {
    *rain = 0;
  } else if (raw_rain < 2) {
    *rain = 5;    
  } else if (raw_rain < 4) {
    *rain = 10;    
  } else if (raw_rain < 8) {
    *rain = 15;    
  } else if (raw_rain < 16) {
    *rain = 20;    
  } else if (raw_rain < 32) {
    *rain = 25;    
  } else if (raw_rain < 48) {
    *rain = 30;    
  } else {
    *rain = 45;
  } 
  c_rain->argb = GColorBlueARGB8;
  
  if (raw_wind < 1)
    *wind = 0;
  else if (raw_wind < 5)
    *wind = 5;
  else if (raw_wind < 11)
    *wind = 10;
  else if (raw_wind < 19)
    *wind = 15;
  else if (raw_wind < 28)
    *wind = 20;
  else if (raw_wind < 38)
    *wind = 25;
  else if (raw_wind < 49)
    *wind = 30;
  else if (raw_wind < 61)
    *wind = 35;
  else 
    *wind = 45;
  *c_wind = GColorOrange;
    
  if (raw_cloud < 10)
    *cloud = 0;
  else if (raw_cloud < 20)
    *cloud = 5;
  else if (raw_cloud < 30)
    *cloud = 10;
  else if (raw_cloud < 40)
    *cloud = 15;
  else if (raw_cloud < 50)
    *cloud = 20;
  else if (raw_cloud < 60)
    *cloud = 25;
  else if (raw_cloud < 70)
    *cloud = 30;
  else if (raw_cloud < 80)
    *cloud = 35;
  else if (raw_cloud < 90)
    *cloud = 40;
  else 
    *cloud = 45;  

  c_cloud->argb = GColorDarkGrayARGB8;

}

//#define MINUTES_PER_DAY (24*60)
static void sunstate(Layer *layer, GContext *ctx)
{
  int start = weather_get_segment_time(0);
  int sunrise; 
  int sunset;

  weather_sun(&sunrise, &sunset);
  
  APP_I_LOG(APP_LOG_LEVEL_DEBUG, "Start %d, Sunrise in %d, sunset in %d", start, sunrise, sunset);
  
  sunset -= start;
  sunrise -= start;
  sunset /= (60); // convert to minutes
  sunrise /= (60);
  
  sunrise -= 90; // apply 90 minute offset, to line up with 'start' line
  sunset -= 90;
  
  if (sunset < 0)
    sunset+=24*60;
  if (sunrise < 0)
    sunrise+= 24*60;
  
  APP_I_LOG(APP_LOG_LEVEL_DEBUG, "Sunrise in %d, sunset in %d", sunrise, sunset);
  
  if (sunrise < sunset)
  {
    graphics_context_set_fill_color(ctx, GColorPastelYellow);
    graphics_fill_radial(ctx, s_bounds_a, GOvalScaleModeFitCircle, 20, DEG_TO_TRIGANGLE(sunrise*360/MINUTES_PER_DAY), DEG_TO_TRIGANGLE(sunset*360/MINUTES_PER_DAY));
    graphics_context_set_fill_color(ctx, GColorLightGray);
    graphics_fill_radial(ctx, s_bounds_a, GOvalScaleModeFitCircle, 20, DEG_TO_TRIGANGLE(sunset*360/MINUTES_PER_DAY), DEG_TO_TRIGANGLE(360));
    graphics_fill_radial(ctx, s_bounds_a, GOvalScaleModeFitCircle, 20, DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(sunrise*360/MINUTES_PER_DAY));
  }
  else
  {
    graphics_context_set_fill_color(ctx, GColorPastelYellow);
    graphics_fill_radial(ctx, s_bounds_a, GOvalScaleModeFitCircle, 20, DEG_TO_TRIGANGLE(sunrise*360/MINUTES_PER_DAY), DEG_TO_TRIGANGLE(360));
    graphics_fill_radial(ctx, s_bounds_a, GOvalScaleModeFitCircle, 20, DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(sunset*360/MINUTES_PER_DAY));
    graphics_context_set_fill_color(ctx, GColorLightGray);
    graphics_fill_radial(ctx, s_bounds_a, GOvalScaleModeFitCircle, 20, DEG_TO_TRIGANGLE(sunset*360/MINUTES_PER_DAY), DEG_TO_TRIGANGLE(sunrise*360/MINUTES_PER_DAY));
  }
  
}

void canvas_update_proc(Layer *layer, GContext *ctx) {
  //draw weather in 3 hour blocks
  if (!weather_is_ready())
  {
    APP_I_LOG(APP_LOG_LEVEL_DEBUG, "weather data not ready yet");
    return;
  }
  
  sunstate(layer, ctx);
  
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
    graphics_context_set_stroke_width(ctx, 4);
    
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
    
    //APP_I_LOG(APP_LOG_LEVEL_DEBUG,"Temp %d %d-> %d (0x%x), Wind %d %d-> %d (0x%x), Rain %d %d-> %d (0x%x)",
    //        temp, point_l_t, point_r_t, c_temp.argb, wind, point_l_w, point_r_w, c_wind.argb, rain, point_l_r, point_r_r, c_rain.argb);
    
    graphics_context_set_fill_color(ctx, c_temp);
    graphics_fill_radial(ctx, s_bounds_a, GOvalScaleModeFitCircle, 4, DEG_TO_TRIGANGLE(point_l_t), DEG_TO_TRIGANGLE(point_r_t));
    graphics_context_set_fill_color(ctx, c_wind);   
    graphics_fill_radial(ctx, s_bounds_b, GOvalScaleModeFitCircle, 4, DEG_TO_TRIGANGLE(point_l_w), DEG_TO_TRIGANGLE(point_r_w));
    graphics_context_set_fill_color(ctx, c_rain);   
    graphics_fill_radial(ctx, s_bounds_c, GOvalScaleModeFitCircle, 4, DEG_TO_TRIGANGLE(point_l_r), DEG_TO_TRIGANGLE(point_r_r)); 
    graphics_context_set_fill_color(ctx, c_cloud);   
    graphics_fill_radial(ctx, s_bounds_d, GOvalScaleModeFitCircle, 4, DEG_TO_TRIGANGLE(point_l_c), DEG_TO_TRIGANGLE(point_r_c)); 
    
    graphics_context_set_stroke_width(ctx, 2);
    graphics_context_set_stroke_color(ctx, GColorBlack);
    /*GPoint p0,p1;
    p0.x = 43;
    p0.y = 18;
    p1.x = 52;
    p1.y = 37;
    
    graphics_draw_line(ctx, p0, p1);*/
    
    // the first segment starts at (15*360/16), compare 'now' against first segments time and find angle for 'now'.
    int nowDelta = (time(0) - weather_get_segment_time(0)) / SECONDS_PER_MINUTE;
    int angle = (nowDelta-90)*360/MINUTES_PER_DAY;
    if (angle < 0)
      angle += 360;
    
    graphics_context_set_fill_color(ctx, GColorBlack);
  
    int trigangle = DEG_TO_TRIGANGLE(angle);
    int line_width_trigangle = 500;
    // draw a very narrow radial (it's just a line)
    graphics_fill_radial(ctx, s_bounds, GOvalScaleModeFitCircle, 24,
      trigangle - line_width_trigangle, trigangle);
    
    APP_I_LOG(APP_LOG_LEVEL_DEBUG, weather_location());
    
    text_layer_set_text(s_location_layer, weather_location());
    //text_layer_set_text(s_location_layer, "royal borough of windsor and maidenhead");

  }
}
  



