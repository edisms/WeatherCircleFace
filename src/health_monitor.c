#include <pebble.h>
#include "health_monitor.h"

void health_monitor_refreah();

static Layer* s_progress_layer;

static int s_step_count = 0, s_step_goal = 0, s_step_average = 0;

static void progress_update_proc(Layer *layer, GContext *ctx);

// Is step data available?
bool step_data_is_available() {
  return HealthServiceAccessibilityMaskAvailable &
    health_service_metric_accessible(HealthMetricStepCount,
      time_start_of_today(), time(NULL));
}

// Daily step goal
static void get_step_goal() {
  const time_t start = time_start_of_today();
  const time_t end = start + SECONDS_PER_DAY;
  s_step_goal = (int)health_service_sum_averaged(HealthMetricStepCount,
    start, end, HealthServiceTimeScopeDaily);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Step goal %d", s_step_goal);
}

// Todays current step count
static void get_step_count() {
  s_step_count = (int)health_service_sum_today(HealthMetricStepCount);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Step count %d", s_step_count);
}

// Average daily step count for this time of day
static void get_step_average() {
  const time_t start = time_start_of_today();
  const time_t end = time(NULL);
  s_step_average = (int)health_service_sum_averaged(HealthMetricStepCount,
    start, end, HealthServiceTimeScopeDaily);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Step average %d", s_step_average);
}

static void health_handler(HealthEventType event, void *context) {
   APP_LOG(APP_LOG_LEVEL_DEBUG, "health_handler %d", event);
  if(event == HealthEventSignificantUpdate) {
    get_step_goal();
  }

  if(event != HealthEventSleepUpdate) {
    get_step_count();
    get_step_average();
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
  
  graphics_context_set_compositing_mode(ctx, GCompOpSet);
  
  /*
  s_step_count = 4000;
  s_step_average = 5000;
  s_step_goal = 10000;*/
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Update progress %d/%d/%d", s_step_count, s_step_goal, s_step_average);
  // get average for last 3 hours, get actual for last 3 hours, 
  // colour coded circle centre based on 
  //   act > ave =  green 
  //   act > 80% ave = amber
  //   act < 80% ave = red
  
  // outer bound shows progress relative to day average, where ave marker set to 75%.
  
  graphics_context_set_stroke_width(ctx, 6);
  //const GRect inset = grect_inset(layer_get_bounds(layer), GEdgeInsets(1));
  const GRect inset = layer_get_bounds(layer);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Bounds %d/%d/%d/%d", inset.origin.x, inset.origin.y, inset.size.w, inset.size.h);
  //graphics_context_set_fill_color(ctx,
  //  s_step_count >= s_step_average ? color_winner : color_loser);
  graphics_context_set_fill_color(ctx, GColorGreen);
  graphics_context_set_stroke_color(ctx, GColorGreen);
  
  graphics_fill_radial(ctx, inset, GOvalScaleModeFitCircle, 5, DEG_TO_TRIGANGLE(0), 
                    DEG_TO_TRIGANGLE((270 * s_step_count) / s_step_goal));
  APP_LOG(APP_LOG_LEVEL_DEBUG, "arc from 0 to %d", (360 * s_step_count) / s_step_goal);
  
  //graphics_fill_radial(ctx, inset, GOvalScaleModeFitCircle, 6,
  //  DEG_TO_TRIGANGLE(0),
  //  DEG_TO_TRIGANGLE(360 * (s_step_count / s_step_goal)));
  
  if(s_step_average > 1) {
    graphics_context_set_stroke_color(ctx, GColorBlack);
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_context_set_stroke_width(ctx, 8);
  
    int trigangle = DEG_TO_TRIGANGLE((360 * s_step_average) / s_step_goal);
    int line_width_trigangle = 1000;
    // draw a very narrow radial (it's just a line)
    graphics_fill_radial(ctx, inset, GOvalScaleModeFitCircle, 5,
      trigangle - line_width_trigangle, trigangle);
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "arc from %d to %d", 360 * (s_step_count / s_step_goal));
  }
  
  int thousands = s_step_count / 1000;
  int hundreds = s_step_count % 1000;
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
