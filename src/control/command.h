#pragma once

#include "tasks.h"
#include "../state.h"
#include <string>
#include <stdio.h>


bool parseAndExecute(DriverState* state, void (*print)(std::string), std::string input);
bool parseAndExecuteMulti(DriverState* state, void (*print)(std::string), std::string input);
void uartcmd_setup(DriverState* state);