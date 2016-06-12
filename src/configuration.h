#pragma once
#include <pebble.h>

//! Callback for a weather 
typedef void(configuration_callback_)();

void configuration_init();

const char* configuration_get_api_key();

bool configuration_use_fake_weather();

void configuration_set_change_callback(configuration_callback_* cb);