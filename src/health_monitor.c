#include <pebble.h>
#include "health_monitor.h"

void health_monitor_refreah();

//static Layer *s_window_layer;//, *s_progress_layer, *s_average_layer;
static TextLayer *s_step_layer;

static char s_current_steps_buffer[16];
static int s_step_count = 0, s_step_goal = 0, s_step_average = 0;

GColor color_loser;
GColor color_winner;

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

static void display_step_count() {
  int thousands = s_step_count / 1000;
  int hundreds = s_step_count % 1000;
  static char s_emoji[5];

  if(s_step_count >= s_step_average) {
    text_layer_set_text_color(s_step_layer, color_winner);
    snprintf(s_emoji, sizeof(s_emoji), "\U0001F60C");
  } else {
    text_layer_set_text_color(s_step_layer, color_loser);
    snprintf(s_emoji, sizeof(s_emoji), "\U0001F4A9");
  }

  if(thousands > 0) {
    snprintf(s_current_steps_buffer, sizeof(s_current_steps_buffer),
      "%d,%03d %s", thousands, hundreds, s_emoji);
  } else {
    snprintf(s_current_steps_buffer, sizeof(s_current_steps_buffer),
      "%d %s", hundreds, s_emoji);
  }

  text_layer_set_text(s_step_layer, s_current_steps_buffer);
}

static void health_handler(HealthEventType event, void *context) {
   APP_LOG(APP_LOG_LEVEL_DEBUG, "health_handler %d", event);
  if(event == HealthEventSignificantUpdate) {
    get_step_goal();
  }

  if(event != HealthEventSleepUpdate) {
    get_step_count();
    get_step_average();
    display_step_count();
    //layer_mark_dirty(s_progress_layer);
    //layer_mark_dirty(s_average_layer);
  }
}
/*
static void progress_layer_update_proc(Layer *layer, GContext *ctx) {
  const GRect inset = grect_inset(layer_get_bounds(layer), GEdgeInsets(2));

  graphics_context_set_fill_color(ctx,
    s_step_count >= s_step_average ? color_winner : color_loser);

  graphics_fill_radial(ctx, inset, GOvalScaleModeFitCircle, 12,
    DEG_TO_TRIGANGLE(0),
    DEG_TO_TRIGANGLE(360 * (s_step_count / s_step_goal)));
}

static void average_layer_update_proc(Layer *layer, GContext *ctx) {
  if(s_step_average < 1) {
    return;
  }

  const GRect inset = grect_inset(layer_get_bounds(layer), GEdgeInsets(2));
  graphics_context_set_fill_color(ctx, GColorYellow);

  int trigangle = DEG_TO_TRIGANGLE(360 * s_step_average / s_step_goal);
  int line_width_trigangle = 1000;
  // draw a very narrow radial (it's just a line)
  graphics_fill_radial(ctx, inset, GOvalScaleModeFitCircle, 12,
    trigangle - line_width_trigangle, trigangle);
}
*/
void health_monitor_init(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);

  // Create a layer to hold the current step count
  s_step_layer = text_layer_create(
      GRect(0, window_bounds.size.h - 16, window_bounds.size.w, 16));
  
  text_layer_set_text_color(s_step_layer, GColorBlack);
  text_layer_set_background_color(s_step_layer, GColorClear);
  text_layer_set_font(s_step_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_step_layer, GTextAlignmentRight);
   
  text_layer_set_text(s_step_layer, "Inital data");
  
  // Subscribe to health events if we can
  if(step_data_is_available()) {
    health_service_events_subscribe(health_handler, NULL);
  }
    
  layer_add_child(window_layer, text_layer_get_layer(s_step_layer));
}

void health_monitor_deinit(Window *window) {
  layer_destroy(text_layer_get_layer(s_step_layer));
  //layer_destroy(s_progress_layer);
  //layer_destroy(s_average_layer);
}


