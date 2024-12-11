#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>
#include "vuelib.h"



#define MAX_INPUT_LENGTH 32

#define INFO_DISPLAY_TIME 2

char info[50];


void conf_reset_all(int i2c_fd) {
    char input[MAX_INPUT_LENGTH];
    printf("\nAre you sure to reset all options? (Y/N)\n");
    if (fgets(input, sizeof(input), stdin) != NULL) {
        if (input[0] == 'Y' || input[0] == 'y') {
            set_auto_on_delay(i2c_fd, 0);
            set_power_cut_delay(i2c_fd, 1);
            set_watchdog_threshold(i2c_fd, 6);
            set_overvoltage_tolerance(i2c_fd, 0);
            set_undervoltage_tolerance(i2c_fd, 0);
            set_temp_cond1(i2c_fd, 65);
            set_fan_speed1(i2c_fd, 100);
            set_temp_cond2(i2c_fd, 60);
            set_fan_speed2(i2c_fd, 80);
            set_temp_cond3(i2c_fd, 55);
            set_fan_speed3(i2c_fd, 65);
            set_vin_calibration(i2c_fd, 0xE2);
            set_vout_calibration(i2c_fd, 0xE2);
            set_iout_calibration(i2c_fd, 0xEC);
            set_overvoltage_tolerance(i2c_fd, 50);
            set_undervoltage_tolerance(i2c_fd, 50);
            printf("All options are now reset!\n");
            sleep(INFO_DISPLAY_TIME);
        }
    }
}

void info_auto_on(int i2c_fd) {
    int delay = get_auto_on_delay(i2c_fd);
    if (delay == 0) {
        sprintf(info, "No");
    } else {
        sprintf(info, "Yes (with %d-second delay)", delay);
    }
}

void conf_auto_on(int i2c_fd) {
    char input[MAX_INPUT_LENGTH];
    int delay = 0;
    bool input_is_valid = false;

    printf("\nPlease input the auto-on delay (1~255, in seconds), input 0 to disable auto-on:\n");
    if (fgets(input, sizeof(input), stdin) != NULL) {
        delay = atoi(input);
        if (delay >= 0 && delay <= 255) {
            input_is_valid = true;
        } else {
            printf("Invalid input. Please enter a number between 0 and 255.\n");
        }
    }

    if (input_is_valid) {
        set_auto_on_delay(i2c_fd, delay);
        if (delay == 0) {
          printf("You have disabled auto-on when powered.\n");
        } else {
          printf("You have set the auto-on delay to: %d seconds\n", delay);
        }
        sleep(INFO_DISPLAY_TIME);
    }
}

void info_power_cut(int i2c_fd) {
    int delay = get_power_cut_delay(i2c_fd);
    if (delay == 0) {
        sprintf(info, "No");
    } else {
        sprintf(info, "Yes (with %d-second delay)", delay);
    }
}

void conf_power_cut(int i2c_fd) {
    char input[MAX_INPUT_LENGTH];
    int delay = 0;
    bool input_is_valid = false;

    printf("\nPlease input the power-cut delay (1~255, in seconds), input 0 to disable power-cut:\n");
    if (fgets(input, sizeof(input), stdin) != NULL) {
        delay = atoi(input);
        if (delay >= 0 && delay <= 255) {
            input_is_valid = true;
        } else {
            printf("Invalid input. Please enter a number between 0 and 255.\n");
        }
    }

    if (input_is_valid) {
        set_power_cut_delay(i2c_fd, delay);
        if (delay == 0) {
          printf("You have disabled power-cut after shutdown.\n");
        } else {
          printf("You have set the power-cut delay to: %d seconds\n", delay);
        }
        sleep(INFO_DISPLAY_TIME);
    }
}

void info_watchdog_threshold(int i2c_fd) {
    int threshold = get_watchdog_threshold(i2c_fd);
    if (threshold == 0) {
        sprintf(info, "Disabled");
    } else {
        sprintf(info, "allow %d missed heartbeats", threshold);
    }
}

void conf_watchdog_threshold(int i2c_fd) {
    char input[MAX_INPUT_LENGTH];
    int threshold = 0;
    bool input_is_valid = false;

    printf("\nPlease input the allowed missed heartbeats (1~255), input 0 to disable watchdog:\n");
    if (fgets(input, sizeof(input), stdin) != NULL) {
        threshold = atoi(input);
        if (threshold >= 0 && threshold <= 255) {
            input_is_valid = true;
        } else {
            printf("Invalid input. Please enter a number between 0 and 255.\n");
        }
    }

    if (input_is_valid) {
        set_watchdog_threshold(i2c_fd, threshold);
        if (threshold == 0) {
          printf("You have disabled watchdog.\n");
        } else {
          printf("You have set the allowed missed heartbeats to: %d\n", threshold);
        }
        sleep(INFO_DISPLAY_TIME);
    }
}

void info_overvoltage(int i2c_fd) {
    int tolerance = get_overvoltage_tolerance(i2c_fd);
    if (tolerance == 0) {
        sprintf(info, "Disabled");
    } else {
        sprintf(info, "%.02fV", ((float)tolerance)/100);
    }
}

void conf_overvoltage(int i2c_fd) {
    char input[MAX_INPUT_LENGTH];
    int tolerance = 0;
    bool input_is_valid = false;

    printf("\nPlease input the overvoltage tolerance (1~100, in 10mV, 50=0.5V), input 0 to disable:\n");
    if (fgets(input, sizeof(input), stdin) != NULL) {
        tolerance = atoi(input);
        if (tolerance >= 0 && tolerance <= 100) {
            input_is_valid = true;
        } else {
            printf("Invalid input. Please enter a number between 0 and 100.\n");
        }
    }

    if (input_is_valid) {
        set_overvoltage_tolerance(i2c_fd, tolerance);
        if (tolerance == 0) {
          printf("You have disabled overvoltage detection.\n");
        } else {
          printf("You have set the overvoltage tolerance to: %d\n", tolerance);
        }
        sleep(INFO_DISPLAY_TIME);
    }
}

void info_undervoltage(int i2c_fd) {
    int tolerance = get_undervoltage_tolerance(i2c_fd);
    if (tolerance == 0) {
        sprintf(info, "Disabled");
    } else {
        sprintf(info, "%.02fV", ((float)tolerance)/100);
    }
}

void conf_undervoltage(int i2c_fd) {
    char input[MAX_INPUT_LENGTH];
    int tolerance = 0;
    bool input_is_valid = false;

    printf("\nPlease input the undervoltage tolerance (1~100, in 10mV, 50=0.5V), input 0 to disable:\n");
    if (fgets(input, sizeof(input), stdin) != NULL) {
        tolerance = atoi(input);
        if (tolerance >= 0 && tolerance <= 100) {
            input_is_valid = true;
        } else {
            printf("Invalid input. Please enter a number between 0 and 100.\n");
        }
    }

    if (input_is_valid) {
        set_undervoltage_tolerance(i2c_fd, tolerance);
        if (tolerance == 0) {
          printf("You have disabled undervoltage detection.\n");
        } else {
          printf("You have set the undervoltage tolerance to: %d\n", tolerance);
        }
        sleep(INFO_DISPLAY_TIME);
    }
}

void info_temp_cond(int i2c_fd, int cond_id) {
    int cond = get_temp_cond_funcs[cond_id - 1](i2c_fd);
    if (cond == 0) {
        sprintf(info, "Disabled");
    } else {
        bool and = (cond & 0x80) > 0;
        cond = (cond & 0x7F);
        sprintf(info, "CPU >= %d°C %s GPU >= %d°C", cond, and ? "and" : "or", cond);
    }
}

void conf_temp_cond(int i2c_fd, int cond_id) {
    char input[MAX_INPUT_LENGTH];
    int cond = 0;
    bool input_is_valid = false;

    printf("\nPlease input temperature%d (1~100, in celsius degree), input 0 to disable:\n", cond_id);
    if (fgets(input, sizeof(input), stdin) != NULL) {
        cond = atoi(input);
        if (cond >= 0 && cond <= 100) {
            input_is_valid = true;
        } else {
            printf("Invalid input. Please enter a number between 0 and 100.\n");
        }
    }

    if (input_is_valid) {
        if (cond == 0) {
            set_temp_cond_funcs[cond_id - 1](i2c_fd, 0);
            printf("You have disabled temperature condition%d.\n", cond_id);
        } else {
            printf("Should both CPU and GPU reach this temperature? (Y/N, default=N)\n");
            if (fgets(input, sizeof(input), stdin) != NULL) {
                if (input[0] == 'Y' || input[0] == 'y') {
                    set_temp_cond_funcs[cond_id - 1](i2c_fd, cond|0x80);
                    printf("You have set temperature condtion%d to: CPU >= %d°C and GPU >= %d°C\n", cond_id, cond, cond);
                } else {
                    set_temp_cond_funcs[cond_id - 1](i2c_fd, cond);
                    printf("You have set temperature condtion%d to: CPU >= %d°C or GPU >= %d°C\n", cond_id, cond, cond);
                }
            }
        }
        sleep(INFO_DISPLAY_TIME);        
    }
}

void info_fan_speed(int i2c_fd, int speed_id) {
    int speed = get_fan_speed_funcs[speed_id - 1](i2c_fd);
    if (speed == 0) {
        sprintf(info, "Disabled");
    } else {
        sprintf(info, "%d%%", speed);
    }
}

void conf_fan_speed(int i2c_fd, int speed_id) {
    char input[MAX_INPUT_LENGTH];
    int speed = 0;
    bool input_is_valid = false;

    printf("\nPlease input the fan speed%d (0~100, in percentage):\n", speed_id);
    if (fgets(input, sizeof(input), stdin) != NULL) {
        speed = atoi(input);
        if (speed >= 0 && speed <= 100) {
            input_is_valid = true;
        } else {
            printf("Invalid input. Please enter a number between 0 and 100.\n");
        }
    }

    if (input_is_valid) {
        set_fan_speed_funcs[speed_id - 1](i2c_fd, speed);
        printf("You have set fan speed%d to: %d%%\n", speed_id, speed);
        sleep(INFO_DISPLAY_TIME);        
    }
}

void info_vin_calibration(int i2c_fd) {
    int cali = get_signed_char(get_vin_calibration(i2c_fd));
    sprintf(info, "%s%.2fV", cali >= 0 ? "+" : "", ((float)cali)/100);
}

void conf_vin_calibration(int i2c_fd) {
    char input[MAX_INPUT_LENGTH];
    int cali = 0;
    bool input_is_valid = false;

    printf("\nPlease input Vin calibration value (-128~127, in 10mV, -35=-0.35V):\n");
    if (fgets(input, sizeof(input), stdin) != NULL) {
        cali = atoi(input);
        if (cali >= -128 && cali <= 127) {
            input_is_valid = true;
        } else {
            printf("Invalid input. Please enter a number between -128 and 127.\n");
        }
    }

    if (input_is_valid) {
        set_vin_calibration(i2c_fd, cali);
        printf("You have set Vin calibration value to: %.2fV\n", ((float)cali)/100);
        sleep(INFO_DISPLAY_TIME);
    }
}

void info_vout_calibration(int i2c_fd) {
    int cali = get_signed_char(get_vout_calibration(i2c_fd));
    sprintf(info, "%s%.2fV", cali >= 0 ? "+" : "", ((float)cali)/100);
}

void conf_vout_calibration(int i2c_fd) {
    char input[MAX_INPUT_LENGTH];
    int cali = 0;
    bool input_is_valid = false;

    printf("\nPlease input Vout calibration value (-128~127, in 10mV, -35=-0.35V):\n");
    if (fgets(input, sizeof(input), stdin) != NULL) {
        cali = atoi(input);
        if (cali >= -128 && cali <= 127) {
            input_is_valid = true;
        } else {
            printf("Invalid input. Please enter a number between -128 and 127.\n");
        }
    }

    if (input_is_valid) {
        set_vout_calibration(i2c_fd, cali);
        printf("You have set Vout calibration value to: %.2fV\n", ((float)cali)/100);
        sleep(INFO_DISPLAY_TIME);
    }
}

void info_iout_calibration(int i2c_fd) {
    int cali = get_signed_char(get_iout_calibration(i2c_fd));
    sprintf(info, "%s%.2fA", cali >= 0 ? "+" : "", ((float)cali)/100);
}

void conf_iout_calibration(int i2c_fd) {
    char input[MAX_INPUT_LENGTH];
    int cali = 0;
    bool input_is_valid = false;

    printf("\nPlease input Iout calibration value (-128~127, in 10mA, -15=-0.15A):\n");
    if (fgets(input, sizeof(input), stdin) != NULL) {
        cali = atoi(input);
        if (cali >= -128 && cali <= 127) {
            input_is_valid = true;
        } else {
            printf("Invalid input. Please enter a number between -128 and 127.\n");
        }
    }

    if (input_is_valid) {
        set_iout_calibration(i2c_fd, cali);
        printf("You have set Iout calibration value to: %.2fA\n", ((float)cali)/100);
        sleep(INFO_DISPLAY_TIME);
    }
}


void print_interface(int i2c_fd) {
    int via_extender = is_powered_via_extender(i2c_fd);
    printf("============================================================\n");
    printf("| Vivid Unit Extender Console V%.2f (fw:0x%02X)\n", VUE_VERSION, get_fw_version(i2c_fd));
    printf("|\n");
    if (via_extender) {
    printf("| Vin: %.3fV   Vout: %.3fV   Iout: %.3fA\n", get_vin(i2c_fd, true), get_vout(i2c_fd, true), get_iout(i2c_fd, true));
    } else {
    printf("| Device is not powered via Vivid Unit Extender\n");
    }
    printf("|\n");
    printf("| Heartbeat Counter: %d    Missed Hartbeats: %d\n", get_heartbeat_counter(i2c_fd), get_missed_heartbeats(i2c_fd));
    printf("|\n");
    printf("| CPU: %d°C    GPU: %d°C    Fan Speed: %d%%\n", get_cpu_temperature(i2c_fd), get_gpu_temperature(i2c_fd), get_fan_speed(i2c_fd));
    printf("============================================================\n");
    printf("Configurable options:\n");
    printf(" 0) Reset all to factory settings\n");
    info_auto_on(i2c_fd);
    printf(" 1) Auto-ON when powered: %s\n", info);
    info_power_cut(i2c_fd);
    printf(" 2) Power-cut after shutdown: %s\n", info);
    info_watchdog_threshold(i2c_fd);
    printf(" 3) Watchdog: %s\n", info);
    info_overvoltage(i2c_fd);
    printf(" 4) Overvoltage tolerance: %s\n", info);
    info_undervoltage(i2c_fd);
    printf(" 5) Undervoltage tolerance: %s\n", info);
    info_temp_cond(i2c_fd, 1);
    printf(" 6) Temperature condition 1: %s\n", info);
    info_fan_speed(i2c_fd, 1);
    printf(" 7) Fan speed for condition 1: %s\n", info);
    info_temp_cond(i2c_fd, 2);
    printf(" 8) Temperature condition 2: %s\n", info);
    info_fan_speed(i2c_fd, 2);
    printf(" 9) Fan speed for condition 2: %s\n", info);
    info_temp_cond(i2c_fd, 3);
    printf("10) Temperature condition 3: %s\n", info);
    info_fan_speed(i2c_fd, 3);
    printf("11) Fan speed for condition 3: %s\n", info);
    info_vin_calibration(i2c_fd);
    printf("12) Input voltage calibration: %s\n", info);
    info_vout_calibration(i2c_fd);
    printf("13) Output voltage calibration: %s\n", info);
    info_iout_calibration(i2c_fd);
    printf("14) Output current calibration: %s\n", info);
    
    printf("Which option do you want to change? (0~14, press q to quit)\n\n");
}

void handle_user_input(int choice, int i2c_fd) {
    switch (choice) {
        case 0: conf_reset_all(i2c_fd); break;
        case 1: conf_auto_on(i2c_fd); break;
        case 2: conf_power_cut(i2c_fd); break;
        case 3: conf_watchdog_threshold(i2c_fd); break;
        case 4: conf_overvoltage(i2c_fd); break;
        case 5: conf_undervoltage(i2c_fd); break;
        case 6: conf_temp_cond(i2c_fd, 1); break;
        case 7: conf_fan_speed(i2c_fd, 1); break;
        case 8: conf_temp_cond(i2c_fd, 2); break;
        case 9: conf_fan_speed(i2c_fd, 2); break;
        case 10: conf_temp_cond(i2c_fd, 3); break;
        case 11: conf_fan_speed(i2c_fd, 3); break;
        case 12: conf_vin_calibration(i2c_fd); break;
        case 13: conf_vout_calibration(i2c_fd); break;
        case 14: conf_iout_calibration(i2c_fd); break;
        default: break;
    }
}

int get_key() {
    struct termios oldt, newt;
    int ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}

int main(int argc, char **argv) {
  
    if (geteuid() != 0) {
        restart_with_sudo(argc, argv);
        return 1;
    }
    
    int i2c_fd = init_i2c_device(I2C_BUS, I2C_ADDR);
    if (i2c_fd < 0) {
      return 1;
    }
  
    int user_input;
    int inputIndex = 0;
    char input[MAX_INPUT_LENGTH];

    while (1) {
        print_interface(i2c_fd);

        inputIndex = 0;
        while (1) {
            int key = get_key();
            if (key == 'q' || key == 'Q') { // press q to quit
                close_i2c_device(i2c_fd);
                return 0;
            }
            if (key == '\n') {
                if (inputIndex == 0) {
                    break;
                }
                input[inputIndex] = '\0';
                user_input = atoi(input);
                if (user_input >= 0 && user_input <= 14) {
                    handle_user_input(user_input, i2c_fd);
                } else {
                    printf("\nInvalid option.\n");
                }
                break;
            }
            if (isdigit(key) && inputIndex < MAX_INPUT_LENGTH - 1) {
                input[inputIndex++] = key;
                printf("%c", key);
            }
        }
    }
    
    close_i2c_device(i2c_fd);
    return 0;
}
