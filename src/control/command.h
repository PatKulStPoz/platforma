#pragma once

#include "tasks.h"
#include "../state.h"
#include <string>
#include <stdio.h>
#include <functional>

typedef std::function<void (std::string)> Printer;

bool parseAndExecute(DriverState* state, Printer print, std::string input);
bool parseAndExecuteMulti(DriverState* state, Printer print, std::string input);
void uartcmd_setup(DriverState* state);