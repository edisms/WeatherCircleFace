#pragma once
#include "pebble.h"

void health_monitor_init(Window *window);

void health_monitor_deinit(Window *window);

void health_monitor_refreah();
