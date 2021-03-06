/*
    motor2d

    Copyright (C) 2015 Florian Kesseler

    This project is free software; you can redistribute it and/or modify it
    under the terms of the MIT license. See LICENSE.md for details.
*/

#pragma once

#include "../joystick.h"
#include "tools.h"

typedef struct {
  joystick_Joystick *joystick;
} l_joystick_Joystick;


bool l_joystick_isJoystick(lua_State* state, int index);
l_joystick_Joystick* l_joystick_toJoystick(lua_State* state, int index);


void l_joystick_register(lua_State* state);
