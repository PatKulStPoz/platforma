#include "command.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "tasks.h"
#include <string>
#include <stdio.h>

#include "../util/stringreader.h"

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

bool uart_echo_enabled = true;

typedef void (*Printer)(std::string);
typedef bool (*Command)(DriverState* state, TaskedDriver driver, StringReader& argument, Printer print);


template <typename T>
bool handleErr(Result<T> res, Printer print) {
    if (!res.success()) {
        print(res.err().msg());
        return true;
    }

    return false;
}

bool cmd_help(DriverState* state, TaskedDriver driver, StringReader& argument, Printer print);

bool cmd_start(DriverState* state, TaskedDriver driver, StringReader& argument, Printer print) {
    state->pushTask(new StartTask(driver));
    print("Added start " + strdrv(driver) + " task");
    return true;
}


bool cmd_stop(DriverState* state, TaskedDriver driver, StringReader& argument, Printer print) {
    state->pushTask(new StopTask(driver));
    print("Added stop " + strdrv(driver) + " task");
    return true;
}

bool cmd_setlevel(DriverState* state, TaskedDriver driver, StringReader& argument, Printer print) {
    Result<double> level = argument.readDouble();
    if (handleErr(level, print)) return false;

    if (level.result() < 0) {
        print("WARN: level too low!");
        return false;
    } else if (level.result() > 1) {
        print("WARN: level too high!");
        return false;
    }

    auto now = argument.readWordLowercase() == "now";
    if (now) {
        print("Set level of " + strdrv(driver) + " to " + std::to_string(level.result()));
        if (driver & TASK_DRIVER_LEFT) state->leftDriver()->setLevel(level.result() * 255);
        if (driver & TASK_DRIVER_RIGHT) state->rightDriver()->setLevel(level.result() * 255);
    } else {
        print("Added set level of " + strdrv(driver) + " to " + std::to_string(level.result()) + " task");
        state->pushTask(new SetLevelTask(driver, (int) (level.result() * 255) ));
    }
    return true;
}


bool cmd_setdirection(DriverState* state, TaskedDriver driver, StringReader& argument, Printer print) {
    auto dir = argument.readWordLowercase();

    DriverDirection direction = !dir.empty() && (dir.at(0) == 'b') ? DRIVER_BACKWARDS : DRIVER_FORWARD;

    print("Added direction of " + strdrv(driver) + " to " + std::to_string(direction) + " task");
    state->pushTask(new SetDirectionTask(driver, direction));
    return true;
}

bool cmd_rotate(DriverState* state, TaskedDriver driver, StringReader& argument, Printer print) {
    auto level = argument.readInt();

    if (handleErr(level, print)) return false;
    if (level.result() < 0) {
        print("WARN: angle too low!");
        return false;
    }

    print("Added rotate " + strdrv(driver) + " " + std::to_string(level.result()) + " degrees task");
    state->pushTask(new RotateTask(driver, level.result()));
    return true;
}


bool cmd_wait(DriverState* state, TaskedDriver driver, StringReader& argument, Printer print) {
    auto level = argument.readDouble();
    if (handleErr(level, print)) return false;
    print("Added wait " + std::to_string(level.result()) + " seconds task");

    state->pushTask(new WaitTicksTask(level.result() * 100));
    return true;
}

bool cmd_wait_for(DriverState* state, TaskedDriver driver, StringReader& argument, Printer print) {
    print("Added wait until " + strdrv(driver) + " finishes task");
    state->pushTask(new WaitForFinishedTask(driver));
    return true;
}

bool cmd_reset(DriverState* state, TaskedDriver driver, StringReader& argument, Printer print) {
    print("Added reset " + strdrv(driver) + " task");
    state->pushTask(new ResetTask(driver));
    return true;
}

bool cmd_cleartasks(DriverState* state, TaskedDriver driver, StringReader& argument, Printer print) {
    print("Cleared all tasks");
    state->clearTasks();
    return true;
}

bool cmd_behavior(DriverState* state, TaskedDriver driver, StringReader& argument, Printer print) {
    std::string str = argument.readWordLowercase();
    state->pushTask(new PopBehaviorTask(driver));

    if (str == "clear" || str == "default") {
        state->pushTask(new ResetBehaviorTask(driver));
        print("Added clear behavior " + strdrv(driver) + " task");
    } else if (str == "sync") {
        state->pushTask(new SetSyncBehaviorTask(driver));
        print("Added set sync behavior " + strdrv(driver) + " task");
    } else {
        print("Invalid argument");
        return false;
    }

    return true;
}

bool cmd_esp32_reboot(DriverState* state, TaskedDriver driver, StringReader& argument, Printer print) {
    print("See you in a moment!");
    vTaskDelay(pdMS_TO_TICKS(1000));
    esp_restart();
    return true;
}

bool cmd_uart_echo(DriverState* state, TaskedDriver driver, StringReader& argument, Printer print) {
    std::string str = argument.readWordLowercase();
    bool val = str.length() == 0 ? !uart_echo_enabled : (str.at(0) == 't' || str.at(0) == '1' || str == "on");
    uart_echo_enabled = val;
    print("UART echo is now " + std::string(val ? "ON" : "OFF"));
    return true;
}


typedef struct {
    std::vector<std::string> name;
    std::string help_args;
    std::string help_description;
    Command command;
} CommandDef;


std::vector<CommandDef> commands = {
    {{"help"}, "", "Show this message", cmd_help},
    {{"start"}, "", "Starts the wheel", cmd_start},
    {{"stop"}, "", "Stops the wheel", cmd_stop},
    {{"setlevel", "setlvl"}, "[decimal, <0;1>]", "Sets the output/speed of the wheel", cmd_setlevel},
    {{"setdirection", "setdir"}, "forward/backward", "Sets the direction of the wheel", cmd_setdirection},
    {{"behavior", "bh"}, "default/sync/clear", "Sets the current behavior", cmd_behavior}, 
    {{"rotate", "rot", "r"}, "[degrees]", "Rotates the wheel by given angle", cmd_rotate},
    {{"wait", "w"}, "[seconds]", "Waits X seconds before executing next task", cmd_wait},
    {{"waitfor", "wf"}, "", "Waits for current wheel behavior to finish", cmd_wait_for},
    {{"reset"}, "", "Resets wheel's state", cmd_reset},
    {{"cleartasks"}, "", "Force-clears all the tasks", cmd_cleartasks},
    {{"esp32.reboot"}, "", "Restarts the esp32" , cmd_esp32_reboot},
    {{"uart.echo"}, "on/off", "Toggles the uart settings", cmd_uart_echo}
};


bool cmd_help(DriverState* state, TaskedDriver driver, StringReader& argument, Printer print) {
    for (CommandDef cmd : commands) {
        print(cmd.name[0] + " " + cmd.help_args + (cmd.help_args.length() == 0 ? "" : " ") + " - " + cmd.help_description);
    }
    return true;
}



bool parseAndExecute(DriverState* state, Printer print, std::string input) {
    if (input.length() < 3) {
        return false;
    }

    StringReader reader = input;

    std::string command = reader.readWordLowercase();

    TaskedDriver driver = TASK_DRIVER_BOTH;
    if (command.length() >= 3 && command.at(1) == ':') {
        switch(command.at(0)) {
            case 'b':
            driver = TASK_DRIVER_BOTH;
            break;
            case 'l':
            driver = TASK_DRIVER_LEFT;
            break;
            case 'r':
            driver = TASK_DRIVER_RIGHT;
            break;
            default:
            driver = TASK_DRIVER_NONE;
        }        

        command = command.substr(2);
    }

    for (CommandDef cmd : commands) {
        for (std::string name : cmd.name) {
            if (name == command) {
                return cmd.command(state, driver, reader, print);
            }
        }
    }
    

    print("ERROR: Invalid command '" + command + "'");
    return false;
}

bool parseAndExecuteMulti(DriverState* state, void (*print)(std::string), std::string input) {
    int i = 0; 
    while (i < input.length() - 1) {
        if (input.at(0) == ' ' || input.at(0) == ';' || input.at(0) == '\n') {
            input = input.substr(1);
        } else if (input.at(i) == ';' || input.at(i) == '\n') {
            if (!parseAndExecute(state, print, input.substr(0, i))) {
                return false;
            }

            if (input.length() <= i + 1) {
                return true;
            }

            input = input.substr(i + 1);
            i = 0;
        } else {
            i++;
        }
    }

    return input.length() == 0 ? true : parseAndExecute(state, print, input);
}
//setlvl 0.5;l:rotate 180;waitf;l:setdir b;l:rotate 180;waitf;setdir f

void uartcmd_println(std::string text) {
    printf("[CMD] ");
    printf(text.c_str());
    printf("\n");
}

void uartcmd_task(void *arg) {
    DriverState* state = static_cast<DriverState*>(arg);

    int i = 0;
    char line[512];
    
    vTaskDelay(pdMS_TO_TICKS(600));
    while (getchar() != EOF) {}
    vTaskDelay(pdMS_TO_TICKS(10));

    printf("[CMD] READY!");

    while (true) {
        int c;

        while ((c = getchar()) != EOF) {
            if (c == '\n' || i >= 500) {
                if (uart_echo_enabled && i > 0) putchar('\n');
                break;
            }

            if (c == '\b') {
                if (i > 0) {
                    i--;
                    if (uart_echo_enabled) putchar(c);
                }
                continue;
            }

            if (uart_echo_enabled) putchar(c);
            line[i++] = c;
        }

        if (c == EOF || i == 0) {
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }

        line[i] = 0;
        parseAndExecuteMulti(state, uartcmd_println, std::string(line));
        i = 0;
    }
}

void uartcmd_setup(DriverState* state) {
    xTaskCreate(uartcmd_task, "uart_command_task", 4096, state, 5, nullptr);
}