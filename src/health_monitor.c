#include <pebble.h>
#include "health_monitor.h"
#include "logging.h"

void health_monitor_refreah();

static Layer* s_progress_layer;
static int s_steps_day_goal;
static int s_steps_day_average;
static int s_steps_now_actual;
static int s_steps_now_average;

static void progress_update_proc(Layer *layer, GContext *ctx);

// Is step data available?
bool step_data_is_available() {
  return HealthServiceAccessibilityMaskAvailable &
    health_service_metric_accessible(HealthMetricStepCount,
      time_start_of_today(), time(NULL));
}

static void update_steps_data()
{
  const time_t start = time_start_of_today();
  const time_t now = time(NULL);
  const time_t end = start + SECONDS_PER_DAY;
  
  s_steps_day_goal = (int)health_service_sum_averaged(HealthMetricStepCount,
                                start, end, HealthServiceTimeScopeDaily);
  
  s_steps_day_average = (int)health_service_sum_averaged(HealthMetricStepCount,
                                start, end, HealthServiceTimeScopeWeekly);
  
  s_steps_now_average = (int)health_service_sum_averaged(HealthMetricStepCount,
                                start, now, HealthServiceTimeScopeWeekly);
  
  s_steps_now_actual = (int)health_service_sum_today(HealthMetricStepCount);
  APP_I_LOG(APP_LOG_LEVEL_DEBUG, "Day goal %d, day average %d, day now average %d, actual %d", 
          s_steps_day_goal, s_steps_day_average, s_steps_now_average, s_steps_now_actual);
}


static void health_handler(HealthEventType event, void *context) {
   APP_I_LOG(APP_LOG_LEVEL_DEBUG, "health_handler %d", event);

  if(event != HealthEventSleepUpdate) {
    update_steps_data();
    layer_mark_dirty(s_progress_layer);
  }
}

void health_monitor_init(Window *window) {
  Layer *window_layer = window_get_root_layer(window);

  // Subscribe to health events if we can
  if(step_data_is_available()) {
    health_service_events_subscribe(health_handler, NULL);
  }

  // progress area
  GRect p_bound = layer_get_bounds(window_get_root_layer(window));
  //p_bound.origin.x = p_bound.size.w - 34;
  p_bound.origin.y = p_bound.size.h - 34;
  p_bound.size.w = 34;
  p_bound.size.h = 34;
  s_progress_layer = layer_create(p_bound);
  layer_set_update_proc(s_progress_layer, progress_update_proc);  
  layer_add_child(window_layer, s_progress_layer);
}

void health_monitor_deinit(Window *window) {
  layer_destroy(s_progress_layer);
}

void health_monitor_refreah()
{
  health_handler(HealthEventSignificantUpdate, 0);
}

void progress_update_proc(Layer *layer, GContext *ctx)
{
  if (!step_data_is_available())
    return;
  
  if (s_steps_now_actual == 0)
    return;
  
  graphics_context_set_compositing_mode(ctx, GCompOpSet);
  
  const GRect inset = layer_get_bounds(layer);
  
  int ave = 100*s_steps_now_actual/s_steps_now_average;
  
  if (ave < 80)
  {
    graphics_context_set_fill_color(ctx, GColorRed);
  } else if (ave < 95)
  {
    graphics_context_set_fill_color(ctx, GColorYellow);
  } else
  {
    graphics_context_set_fill_color(ctx, GColorGreen);
  }
  
  graphics_fill_radial(ctx, inset, GOvalScaleModeFitCircle, 5, DEG_TO_TRIGANGLE(0), 
                    DEG_TO_TRIGANGLE((270 * s_steps_now_actual) / s_steps_day_average));
  
  graphics_context_set_fill_color(ctx, GColorLightGray);
  graphics_fill_radial(ctx, inset, GOvalScaleModeFitCircle, 3, 
                    DEG_TO_TRIGANGLE((270 * s_steps_now_actual) / s_steps_day_average), DEG_TO_TRIGANGLE(360));
  
  if(s_steps_day_average > 1) {
    graphics_context_set_fill_color(ctx, GColorBlack);
  
    int trigangle = DEG_TO_TRIGANGLE(270);
    int line_width_trigangle = 1000;
    // draw a very narrow radial (it's just a line)
    graphics_fill_radial(ctx, inset, GOvalScaleModeFitCircle, 5,
      trigangle - line_width_trigangle, trigangle);
  }
  
  int thousands = s_steps_now_actual / 1000;
  int hundreds = s_steps_now_actual % 1000;
  static char steps_string[8];
  if(thousands > 0) {
    snprintf(steps_string, sizeof(steps_string),
      "%d,%03d", thousands, hundreds);
  } else {
    snprintf(steps_string, sizeof(steps_string),
      "%d", hundreds);
  }
  
  GRect p = layer_get_bounds(layer);
  p.origin.y = (p.origin.y+14)/2;

  graphics_context_set_text_color(ctx, GColorBlack);
  graphics_draw_text(ctx, steps_string, fonts_get_system_font(FONT_KEY_GOTHIC_14), 
                     p, GTextOverflowModeFill, GTextAlignmentCenter, 0);

}

