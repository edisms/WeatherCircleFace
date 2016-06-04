#pragma once
#include <pebble.h>

typedef void(bt_callback_)(bool connected);

void bt_monitor_init(Window *window);

void bt_monitor_deinit(Window *window);

void bt_connection_status_monitor(bt_callback_ bt);