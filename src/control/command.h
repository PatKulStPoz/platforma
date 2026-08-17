#pragma once

#include "tasks.h"
#include <string>

std::string strdrv(TaskedDriver driver) {
    switch (driver) {
        case TASK_DRIVER_BOTH:
        return "both wheels";
        case TASK_DRIVER_LEFT:
        return "left wheel";
        case TASK_DRIVER_RIGHT:
        return "right wheel";
        default:
        return "no wheels";
    }
}

bool parseAndExecute(DriverState* state, void (*print)(std::string), std::string input) {
    if (input.length() < 3) {
        return false;
    }

    int index = input.find(' ');

    std::string command;
    std::string argument;
    if (index == -1) {
        command = input;
        argument = "";
    } else {
        command = input.substr(0, index);
        argument = input.substr(index + 1);
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
    //print("DEBUG: Wheel: '" + strdrv(driver) + "', command: '" + command + "', argument: '" + argument + "'");

    if (command == "start") {
        state->pushTask(new StartTask(driver));
        print("Added starting " + strdrv(driver) + " task");
    } else if (command == "stop") {
        state->pushTask(new StopTask(driver));
        print("Added stopping " + strdrv(driver) + " task");
    } else if (command == "setlevel" || command == "setlvl") {
        float level = atof(argument.c_str());
        if (level < 0) {
            print("WARN: level too low!");
            level = 0;
        } else if (level > 1) {
            print("WARN: level too high!");
            level = 1;
        }
        print("Added set level of " + strdrv(driver) + " to " + std::to_string(level) + " task");
        state->pushTask(new SetLevelTask(driver, (int) (level * 255) ));
    } else if (command == "setdirection" || command == "setdir") {
        DriverDirection direction = !argument.empty() && (argument.at(0) == 'B' || argument.at(0) == 'b') ? DRIVER_BACKWARDS : DRIVER_FORWARD;

        print("Added direction of " + strdrv(driver) + " to " + std::to_string(direction) + " task");
        state->pushTask(new SetDirectionTask(driver, direction));
    } else if (command == "rotate" || command == "r" || command == "rot") {
        int level = atoi(argument.c_str());
        if (level < 0) {
            print("WARN: angle too low!");
            level = 0;
        }

        print("Added rotate " + strdrv(driver) + " " + std::to_string(level) + " degrees task");

        state->pushTask(new RotateTask(driver, level));
    } else if (command == "wait" || command == "w") {
        uint32_t level = atof(argument.c_str()) * 100;
        print("Added wait " + std::to_string(level / (float) 100) + " seconds task");

        state->pushTask(new WaitTicksTask(level));
    } else if (command == "waitf" || command == "wf") {
        print("Added wait until " + strdrv(driver) + " finishes task");
        state->pushTask(new WaitForFinishedTask(driver));
    } else {
        print("ERROR: Invalid task '" + command + "' with argument '" + argument + "'");
        return false;
    }

    return true; 
}

bool parseAndExecuteMulti(DriverState* state, void (*print)(std::string), std::string input) {
    int i = 0; 
    while (i < input.length() - 1) {
        if (input.at(0) == ' ' || input.at(0) == ';' || input.at(0) == '\n') {
            input = input.substr(1);
        } else if (input.at(i) == ';' || input.at(i) == '\n') {
            if (!parseAndExecute(state, print, input.substr(0, i))) return false;
            if (input.length() <= i + 1) {
                return true;
            }

            input = input.substr(i + 1);
            i = 0;
        } else {
            i++;
        }
    }

    return parseAndExecute(state, print, input);
}