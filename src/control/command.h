#pragma once

#include "tasks.h"
#include <string>

bool parseAndExecute(DriverState* state, void (*print)(std::string), std::string input) {
    if (input.length() < 3) {
        return false;
    }

    int index = input.find(' ');

    std::string command;
    std::string argument;
    if (index != -1) {
        command = input;
        argument = "";
    } else {
        command = input.substr(0, index);
        argument = input.substr(index);
    }

    TaskedDriver driver = TASK_DRIVER_BOTH;
    
    if (command.length() >= 3 && command.at(1) == ':') {
        switch(command.at(0)) {
            case 'L':
            case 'l':
            driver = TASK_DRIVER_LEFT;
            break;
            case 'R':
            case 'r':
            driver = TASK_DRIVER_RIGHT;
            break;
            default:
            driver = TASK_DRIVER_NONE;
        }        

        command = command.substr(2);
    }

    if (command == "start") {
        state->pushTask(new StartTask(driver));
    } else if (command == "stop") {
        state->pushTask(new StopTask(driver));
    } else if (command == "setlevel" || command == "setlvl") {
        float level = atof(argument.c_str());
        if (level < 0) {
            print("WARN: level too low!");
            level = 0;
        } else if (level > 1) {
            print("WARN: level too high!");
            level = 1;
        }

        state->pushTask(new SetLevelTask(driver, (int) (level * 255) ));
    } else if (command == "setdirection" || command == "setdir") {
        state->pushTask(new SetDirectionTask(driver, !argument.empty() && (argument.at(0) == 'B' || argument.at(0) == 'b') ? DRIVER_BACKWARDS : DRIVER_FORWARD ));
    } else if (command == "rotate" || command == "r" || command == "rot") {
        int level = atoi(argument.c_str());
        if (level < 0) {
            print("WARN: angle too low!");
            level = 0;
        }

        state->pushTask(new RotateTask(driver, level));
    } else if (command == "wait" || command == "w") {
        uint32_t level = atof(argument.c_str()) * 100;

        state->pushTask(new WaitTicksTask(level));
    } else if (command == "waitf" || command == "wf") {
        state->pushTask(new WaitForFinishedTask(driver));
    } else {
        return false;
    }

    return true; 
}

bool parseAndExecuteMulti(DriverState* state, void (*print)(std::string), std::string input) {
    for (int i = 0; i < input.length(); i++) {
        if (input.at(i) == ';' || input.at(i) == '\n') {
            if (!parseAndExecute(state, print, input.substr(0, i))) return false;
            if (input.length() <= i + 1) {
                return true;
            }

            input = input.substr(i + 1);
            i = 0;
        }
    }

    return parseAndExecute(state, print, input);
}