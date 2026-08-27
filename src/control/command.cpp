#include "command.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "tasks.h"
#include <string>
#include <stdio.h>

#include "../datastorage.h"
#include "../util/stringreader.h"

std::string to_string(TaskedDriver driver) {
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

std::string to_string(HallId driver) {
    switch (driver) {
        case HALL_BACK:
        return "backward";
        case HALL_FRONT:
        return "forward";
        default:
        return "none";
    }
}

std::string to_string(DriverDirection driver) {
    switch (driver) {
        case DRIVER_BACKWARDS:
        return "backward";
        case DRIVER_FORWARD:
        return "forward";
        default:
        return "none";
    }
}


bool uart_echo_enabled = true;

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
    state->getTasks()->pushTask(new StartTask(driver));
    print("Added start " + to_string(driver) + " task");
    return true;
}


bool cmd_stop(DriverState* state, TaskedDriver driver, StringReader& argument, Printer print) {
    state->getTasks()->pushTask(new StopTask(driver));
    print("Added stop " + to_string(driver) + " task");
    return true;
}

bool cmd_setlevel(DriverState* state, TaskedDriver driver, StringReader& argument, Printer print) {
    Result<double> level = argument.readDouble();
    if (handleErr(level, print)) return false;

    if (level.result() < 0) {
        print("WARN: level too low!");
        return false;
    } else if (level.result() > 100) {
        print("WARN: level too high!");
        return false;
    }

    auto now = argument.readWordLowercase() == "now";
    if (now) {
        print("Set level of " + to_string(driver) + " to " + std::to_string(level.result()));
        if (driver & TASK_DRIVER_LEFT) state->leftDriver()->setLevel(level.result() * 255 / 100);
        if (driver & TASK_DRIVER_RIGHT) state->rightDriver()->setLevel(level.result() * 255 / 100);
    } else {
        print("Added set level of " + to_string(driver) + " to " + std::to_string(level.result()) + " task");
        state->getTasks()->pushTask(new SetLevelTask(driver, (int) (level.result() * 255 / 100) ));
    }
    return true;
}


bool cmd_setdirection(DriverState* state, TaskedDriver driver, StringReader& argument, Printer print) {
    auto dir = argument.readWordLowercase();

    DriverDirection direction = !dir.empty() && (dir.at(0) == 'b') ? DRIVER_BACKWARDS : DRIVER_FORWARD;

    if (driver & TASK_DRIVER_LEFT && !state->leftDriver()->getConfig().allow_backwards) {
        print("WARNING: Left wheel doesn't support direction changing!");
        driver = (TaskedDriver) (driver & TASK_DRIVER_RIGHT);
    }

    if (driver & TASK_DRIVER_RIGHT && !state->rightDriver()->getConfig().allow_backwards) {
        print("WARNING: Right wheel doesn't support direction changing!");
        driver = (TaskedDriver) (driver & TASK_DRIVER_LEFT);
    }

    print("Added direction of " + to_string(driver) + " to " + to_string(direction) + " task");
    state->getTasks()->pushTask(new SetDirectionTask(driver, direction));
    return true;
}


bool cmd_setbrake(DriverState* state, TaskedDriver driver, StringReader& argument, Printer print) {
    auto str = argument.readWordLowercase();

    bool val = str.length() == 0 ? !uart_echo_enabled : (str.at(0) == 't' || str.at(0) == '1' || str == "on");

    if (driver & TASK_DRIVER_LEFT && !state->leftDriver()->getConfig().has_brake) {
        print("WARNING: Left wheel doesn't support brakes!");
        driver = (TaskedDriver) (driver & TASK_DRIVER_RIGHT);
    }

    if (driver & TASK_DRIVER_RIGHT && !state->rightDriver()->getConfig().has_brake) {
        print("WARNING: Right wheel doesn't support brakes!");
        driver = (TaskedDriver) (driver & TASK_DRIVER_LEFT);
    }

    print("Added set brake of " + to_string(driver) + " to " + std::to_string(val) + " task");
    state->getTasks()->pushTask(new SetBrakeTask(driver, val));
    return true;
}

bool cmd_rotate(DriverState* state, TaskedDriver driver, StringReader& argument, Printer print) {
    auto level = argument.readInt();

    if (handleErr(level, print)) return false;
    if (level.result() < 0) {
        print("WARN: angle too low!");
        return false;
    }

    print("Added rotate " + to_string(driver) + " " + std::to_string(level.result()) + " degrees task");
    state->getTasks()->pushTask(new RotateTask(driver, level.result()));
    return true;
}


bool cmd_wait(DriverState* state, TaskedDriver driver, StringReader& argument, Printer print) {
    auto level = argument.readDouble();
    if (handleErr(level, print)) return false;
    print("Added wait " + std::to_string(level.result()) + " seconds task");

    state->getTasks()->pushTask(new WaitTicksTask(level.result() * 100));
    return true;
}

bool cmd_wait_for(DriverState* state, TaskedDriver driver, StringReader& argument, Printer print) {
    print("Added wait until " + to_string(driver) + " finishes task");
    state->getTasks()->pushTask(new WaitForFinishedTask(driver));
    return true;
}


void driver_state_print(Driver* driver, Printer print) {
    print(" level = " + std::to_string(driver->getLevel() * 100 / 255));
    print(" direction = " + to_string(driver->getDirection()));
    print(" brake = " + std::to_string(driver->getBrake()));

    print(" current_level = " + std::to_string(driver->getCurrentLevel() * 100 / 255));
    print(" target_level = " + std::to_string(driver->getTargetLevel() * 100 / 255));
    print(" hall_ticks = " + std::to_string(driver->getHallTicks()));
    print(" driver_ticks = " + std::to_string(driver->getDriverTicks()));
    print(" hall_direction = " + to_string(driver->getHallDirection()));
    print(" behavior = " + driver->getBehaviorToStringWithExtraSafe());

}


bool cmd_state(DriverState* state, TaskedDriver driver, StringReader& argument, Printer print) {
    if (driver & TASK_DRIVER_LEFT) {
        print("Left wheel: ");
        driver_state_print(state->leftDriver(), print);
    }
    if (driver & TASK_DRIVER_RIGHT) {
        print("Right wheel: ");
        driver_state_print(state->leftDriver(), print);
    }
    return true;
}

bool cmd_reset(DriverState* state, TaskedDriver driver, StringReader& argument, Printer print) {
    print("Added reset " + to_string(driver) + " task");
    state->getTasks()->pushTask(new ResetTask(driver));
    return true;
}

bool cmd_cleartasks(DriverState* state, TaskedDriver driver, StringReader& argument, Printer print) {
    print("Cleared all tasks");
    state->getTasks()->clearTasks();
    return true;
}

bool cmd_behavior(DriverState* state, TaskedDriver driver, StringReader& argument, Printer print) {
    std::string str = argument.readWordLowercase();
    state->getTasks()->pushTask(new PopBehaviorTask(driver));

    if (str == "clear" || str == "default") {
        state->getTasks()->pushTask(new ResetBehaviorTask(driver));
        print("Added clear behavior " + to_string(driver) + " task");
    } else if (str == "sync") {
        state->getTasks()->pushTask(new SetSyncBehaviorTask(driver));
        print("Added set sync behavior " + to_string(driver) + " task");
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

bool cmd_battery_status(DriverState* state, TaskedDriver driver, StringReader& argument, Printer print) {
    print("Battery: " + std::to_string(state->getBatteryPercentage()) + "% (" + std::to_string(state->getBatteryVoltage()) + " mV)");
    return true;
}

bool cmd_config_print(DriverState* state, TaskedDriver driver, StringReader& argument, Printer print) {
    if (driver & TASK_DRIVER_LEFT) {
        print("Left Wheel:");
        print(" hall_ticks_per_full_rotation = " + std::to_string(state->leftDriver()->getConfig().hall_sensor_ticks_per_full_rotation));
        print(" driver_ticks_per_full_rotation = " + std::to_string(state->leftDriver()->getConfig().driver_ticks_per_full_rotation));
        print(" brake = " + std::to_string(state->leftDriver()->getConfig().has_brake));
        print(" allow_backwards = " + std::to_string(state->leftDriver()->getConfig().allow_backwards));
        print(" level_scale = " + std::to_string(state->leftDriver()->getConfig().level_scale * 100 / 255.0));
    }

    if (driver & TASK_DRIVER_RIGHT) {
        print("Right Wheel:");
        print(" hall_ticks_per_full_rotation = " + std::to_string(state->rightDriver()->getConfig().hall_sensor_ticks_per_full_rotation));
        print(" driver_ticks_per_full_rotation = " + std::to_string(state->rightDriver()->getConfig().driver_ticks_per_full_rotation));
        print(" brake = " + std::to_string(state->rightDriver()->getConfig().has_brake));
        print(" allow_backwards = " + std::to_string(state->rightDriver()->getConfig().allow_backwards));
        print(" level_scale = " + std::to_string(state->rightDriver()->getConfig().level_scale * 100 / 255.0));
    }

    print("General:");
    print(" wifi_ssid = " + std::string(state->getConfig().wifi_ssid));
    print(" wifi_password = " + std::string(state->getConfig().wifi_password));
    print(" wifi_ap = " + std::to_string(state->getConfig().wifi_ap));
    print(" hostname = " + std::string(state->getConfig().hostname));

    return true;
}

bool cmd_config_set_hall_ticks(DriverState* state, TaskedDriver driver, StringReader& argument, Printer print) {
    Result<uint32_t> level = argument.readUInt();
    if (handleErr(level, print)) return false;

    if (driver & TASK_DRIVER_LEFT) {
        state->leftDriver()->getConfig().hall_sensor_ticks_per_full_rotation = level.result();        
    }

    if (driver & TASK_DRIVER_RIGHT) {
        state->rightDriver()->getConfig().hall_sensor_ticks_per_full_rotation = level.result();        
    }
    
    print("Set hall sensor tick count for full rotation of " + to_string(driver) + " to " + std::to_string(level.result()));

    return true;
}

bool cmd_config_set_driver_ticks(DriverState* state, TaskedDriver driver, StringReader& argument, Printer print) {
    Result<uint32_t> level = argument.readUInt();
    if (handleErr(level, print)) return false;

    if (driver & TASK_DRIVER_LEFT) {
        state->leftDriver()->getConfig().driver_ticks_per_full_rotation = level.result();        
    }

    if (driver & TASK_DRIVER_RIGHT) {
        state->rightDriver()->getConfig().driver_ticks_per_full_rotation = level.result();        
    }
    
    print("Set driver tick count for full rotation of " + to_string(driver) + " to " + std::to_string(level.result()));

    return true;
}

bool cmd_config_set_allow_backwards(DriverState* state, TaskedDriver driver, StringReader& argument, Printer print) {
    Result<bool> level = argument.readBool();
    if (handleErr(level, print)) return false;

    if (driver & TASK_DRIVER_LEFT) {
        state->leftDriver()->getConfig().allow_backwards = level.result();        
    }

    if (driver & TASK_DRIVER_RIGHT) {
        state->rightDriver()->getConfig().allow_backwards = level.result();        
    }
    
    print("Set allow backwards of " + to_string(driver) + " to " + std::to_string(level.result()));

    return true;
}

bool cmd_config_set_has_brake(DriverState* state, TaskedDriver driver, StringReader& argument, Printer print) {
    Result<bool> level = argument.readBool();
    if (handleErr(level, print)) return false;

    if (driver & TASK_DRIVER_LEFT) {
        state->leftDriver()->getConfig().has_brake = level.result();        
    }

    if (driver & TASK_DRIVER_RIGHT) {
        state->rightDriver()->getConfig().has_brake = level.result();        
    }
    
    print("Set has brake of " + to_string(driver) + " to " + std::to_string(level.result()));

    return true;
}

bool cmd_config_set_level_scale(DriverState* state, TaskedDriver driver, StringReader& argument, Printer print) {
    Result<double> level = argument.readDouble();
    if (handleErr(level, print)) return false;

    if (level.result() < 0) {
        print("WARN: level too low!");
        return false;
    } else if (level.result() > 100) {
        print("WARN: level too high!");
        return false;
    }

    if (driver & TASK_DRIVER_LEFT) {
        state->leftDriver()->getConfig().level_scale = (level.result() * 255 / 100);        
    }

    if (driver & TASK_DRIVER_RIGHT) {
        state->rightDriver()->getConfig().level_scale = (level.result() * 255 / 100);        
    }
    
    print("Set level scale of " + to_string(driver) + " to " + std::to_string(level.result()));

    return true;
}

bool cmd_config_set_wifi_ssid(DriverState* state, TaskedDriver driver, StringReader& argument, Printer print) {
    std::string str = argument.readGreedyString();
    strcpy(state->getConfig().wifi_ssid, str.c_str());
    
    print("Set connecting wifi ssid to '" + str + "'");
    print("Note: To connect restart the device, which you can do with esp32.reboot command");

    return true;
}

bool cmd_config_set_wifi_password(DriverState* state, TaskedDriver driver, StringReader& argument, Printer print) {
    std::string str = argument.readGreedyString();
    strcpy(state->getConfig().wifi_password, str.c_str());

    print("Set connecting wifi password to '" + str + "'");
    print("Note: To connect restart the device, which you can do with esp32.reboot command");

    return true;
}

bool cmd_config_set_wifi_ap(DriverState* state, TaskedDriver driver, StringReader& argument, Printer print) {
    Result<bool> val = argument.readBool();
    if (handleErr(val, print)) return false;

    print("Set wifi mode to " + std::string(val.result() ? "access point" : "station"));
    print("Note: To change the mode, restart the device, which you can do with esp32.reboot command");

    return true;
}

bool cmd_config_set_hostname(DriverState* state, TaskedDriver driver, StringReader& argument, Printer print) {
    std::string str = argument.readGreedyString();
    strcpy(state->getConfig().hostname, str.c_str());

    print("Set hostname to '" + str + "'");
    print("Note: To update it, restart the device, which you can do with esp32.reboot command");

    return true;
}


bool cmd_config_save(DriverState* state, TaskedDriver driver, StringReader& argument, Printer print) {
    if (driver & TASK_DRIVER_LEFT) {
        save_data("left_driver", &state->leftDriver()->getConfig());
    }

    if (driver & TASK_DRIVER_RIGHT) {
        save_data("right_driver", &state->rightDriver()->getConfig());
    }

    save_data("main", &state->getConfig());

    print("Saved configuration!");
    cmd_config_print(state, driver, argument, print);

    return true;
}

bool cmd_config_load(DriverState* state, TaskedDriver driver, StringReader& argument, Printer print) {
    if (driver & TASK_DRIVER_LEFT) {
        load_data("left_driver", &state->leftDriver()->getConfig());
    }

    if (driver & TASK_DRIVER_RIGHT) {
        load_data("right_driver", &state->rightDriver()->getConfig());
    }

    load_data("main", &state->getConfig());

    print("Loaded configuration!");
    cmd_config_print(state, driver, argument, print);

    return true;
}

void test_wheel(Driver* driver, Printer print) {
    driver->reset();
    driver->clearCount();
    driver->setLevel(255);
    driver->start();

    uint32_t tick = 0;
    while (tick++ < 1000) {
        driver->update();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    driver->clearCount();
    tick = 0;
    uint32_t hallCount = driver->getConfig().hall_sensor_ticks_per_full_rotation * 100;
    while (driver->getHallTicks() < hallCount) {
        driver->update();
        tick++;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    uint32_t driverTicks = driver->getDriverTicks();
    print("Driver ticks for " + std::to_string(hallCount) + " -> " + std::to_string(driverTicks));
    print("Driver ticks for full rotation  -> " + std::to_string(driverTicks / 100));
    print("Time for full rotation -> " + std::to_string(tick * 10 / 100));

    driver->reset();
}

bool cmd_wheel_tester(DriverState* state, TaskedDriver driver, StringReader& argument, Printer print) {
    print("Preparing for testing... Previous state will be invalid!");
    state->getTasks()->clearTasks();
    if (driver & TASK_DRIVER_LEFT) {
        state->leftDriver()->reset();
        state->leftDriver()->clearCount();
    }
    if (driver & TASK_DRIVER_RIGHT) {
        state->rightDriver()->reset();
        state->rightDriver()->clearCount();
    }

    vTaskDelay(pdMS_TO_TICKS(100));
    
    if (driver & TASK_DRIVER_LEFT) {
        print("== Testing left wheel");
        test_wheel(state->leftDriver(), print);
    }
    
    if (driver & TASK_DRIVER_RIGHT) {
        print("== Testing right wheel");
        test_wheel(state->rightDriver(), print);
    }



    return true;
}



typedef struct CommandDef {
    std::vector<std::string> name;
    std::string help_args;
    std::string help_description;
    Command command = NULL;
    std::vector<CommandDef> subcmd = std::vector<CommandDef>(0);
} CommandDef;


std::vector<CommandDef> commands = {
    {{"help"}, "", "Show this message", cmd_help},
    {{"start"}, "", "Starts the wheel", cmd_start},
    {{"stop"}, "", "Stops the wheel", cmd_stop},
    {{"setlevel", "setlvl"}, "[decimal, <0;100>]", "Sets the output/speed of the wheel", cmd_setlevel},
    {{"setdirection", "setdir"}, "forward/backward", "Sets the direction of the wheel", cmd_setdirection},
    {{"setbrake", "brake"}, "[bool]", "Sets the direction of the wheel", cmd_setbrake},
    {{"behavior", "bh"}, "default/sync/clear", "Sets the current behavior", cmd_behavior}, 
    {{"rotate", "rot", "r"}, "[degrees]", "Rotates the wheel by given angle", cmd_rotate},
    {{"wait", "w"}, "[seconds]", "Waits X seconds before executing next task", cmd_wait},
    {{"waitfor", "wf"}, "", "Waits for current wheel behavior to finish", cmd_wait_for},
    {{"state"}, "", "Prints state of the wheel", cmd_state},
    {{"reset"}, "", "Resets wheel's state", cmd_reset},
    {{"cleartasks"}, "", "Force-clears all the tasks", cmd_cleartasks},
    {{"esp32.reboot"}, "", "Restarts the esp32" , cmd_esp32_reboot},
    {{"uart.echo"}, "on/off", "Toggles the uart settings", cmd_uart_echo},
    {{"battery.status"}, "", "Prints the battery status", cmd_battery_status},
    {{"config"}, "", "Configuration...", NULL , {
        {{"print"}, "", "Prints current config", cmd_config_print},
        {{"set"}, "", "Set value...", NULL,  {
            {{"hall_ticks_per_full_rotation", "hall_count", "hall", "hall_ticks"}, "[number]", "Number of 'ticks' from hall sensor for full rotation", cmd_config_set_hall_ticks},
            {{"driver_ticks_per_full_rotation", "driver_count", "driver", "driver_ticks"}, "[number]", "Number of 'ticks' from wheel driver for full rotation", cmd_config_set_driver_ticks},
            {{"has_brake", "brake"}, "[bool]", "Sets the has brake value of the wheel", cmd_config_set_has_brake},
            {{"allow_backwards", "backwards"}, "[bool]", "Sets the allow backwards value of the wheel", cmd_config_set_allow_backwards},
            {{"scale_level", "level"}, "[decimal, <0;100>]", "Sets the level scaler for the wheel", cmd_config_set_level_scale},
            {{"wifi_ssid"}, "[ssid]", "Sets ssid of wifi to connect to", cmd_config_set_wifi_ssid},
            {{"wifi_password"}, "[password/empty]", "Sets password of wifi to connect to. Setting it to empty skips password auth", cmd_config_set_wifi_password},
            {{"wifi_ap"}, "[bool]", "Toggles the access point wifi mode", cmd_config_set_wifi_ap},
            {{"hostname"}, "[hostname]", "Set's the hostname of the device", cmd_config_set_hostname},

        }},
        {{"save"}, "", "Saves current configuration.", cmd_config_save},
        {{"load"}, "", "Load previously saved configuration", cmd_config_load}
    }},
    {{"wheel_test"}, "", "Test both wheels and tries to receive data from them. Useful for configuration and debugging. Only use when not on ground!", cmd_wheel_tester},
};


void cmd_help_print(int space, std::vector<CommandDef> commands, Printer print) {
    std::string prefix;
    for (int i = 0; i < space; i++) {
        prefix += " ";
    }
    
    for (CommandDef cmd : commands) {
        print(prefix + cmd.name[0] + " " + cmd.help_args + (cmd.help_args.length() == 0 ? "" : " ") + " - " + cmd.help_description);
        if (!cmd.subcmd.empty()) {
            cmd_help_print(space + 2, cmd.subcmd, print);
        }
    }
}

bool cmd_help(DriverState* state, TaskedDriver driver, StringReader& argument, Printer print) {
    cmd_help_print(0, commands, print);
    return true;
}


int parseAndExecuteInner(DriverState* state, TaskedDriver driver, Printer print, std::vector<CommandDef> commands, std::string command, StringReader reader) {
    for (CommandDef cmd : commands) {
        for (std::string name : cmd.name) {
            if (name == command) {
                if (!cmd.subcmd.empty()) {
                    int i = reader.index();
                    int res = parseAndExecuteInner(state, driver, print, cmd.subcmd, reader.readWordLowercase(), reader);
                    if (res >= 0) return res;
                    reader.index(i);
                }

                if (cmd.command != NULL) {
                    return cmd.command(state, driver, reader, print);
                }
                break;
            }
        }
    }

    return -1;
}

bool parseAndExecute(DriverState* state, Printer print, std::string input) {
    if (input.length() < 3) {
        print("ERROR: Invalid command '" + input + "'");
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
    
    int res = parseAndExecuteInner(state, driver, print, commands, command, reader);
    if (res == -1) {
        print("ERROR: Invalid command '" + command + "'");
    }
    return res > 0;
}

bool parseAndExecuteMulti(DriverState* state, Printer print, std::string input) {
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

            if (c == '\b' || c == 0x7F) {
                if (i > 0) {
                    i--;
                    if (uart_echo_enabled) {
                        putchar('\b');
                        putchar(' ');
                        putchar('\b');
                    }
                }
                continue;
            }

            if (c < 32) {
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