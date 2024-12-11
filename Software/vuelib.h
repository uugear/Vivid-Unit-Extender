#ifndef __VUELIB_H
#define __VUELIB_H

#include <stdbool.h>

#define VUE_VERSION     1.00f

#define CPU_TEMP_PATH   "/sys/devices/virtual/thermal/thermal_zone0/temp"
#define GPU_TEMP_PATH   "/sys/devices/virtual/thermal/thermal_zone1/temp"

#define I2C_BUS         "/dev/i2c-2"
#define I2C_ADDR        0x0A
#define I2C_LOCK        "/var/lock/vivid_unit_extender_i2c.lock"

// Read-Only Information Registers
#define I2C_BEGIN_INFORMATION                   0
#define I2C_FIRMWARE_VERSION                    0
#define I2C_INPUT_VOLTAGE_H                     1
#define I2C_INPUT_VOLTAGE_L                     2
#define I2C_OUTPUT_VOLTAGE_H                    3
#define I2C_OUTPUT_VOLTAGE_L                    4
#define I2C_OUTPUT_CURRENT_H                    5
#define I2C_OUTPUT_CURRENT_L                    6
#define I2C_FAN_SPEED                           7
#define I2C_MISSED_HEARTBEAT                    8
#define I2C_DEVICE_STATE                        9        // bit-0: power mode (0=VU direct, 1=via Extender)
#define I2C_RECENT_EVENTS                       10       // upper 4-bit: on event, lower 4-bit: off event

// Read-Writable Data Registers
#define I2C_BEGIN_DATA                          12
#define I2C_DATA_HEARTBEAT_COUNTER              12
#define I2C_DATA_CPU_TEMPERATURE                13
#define I2C_DATA_GPU_TEMPERATURE                14

// Configuration (EEPROM-backed) Registers
#define I2C_BEGIN_CONFIGURATION                 18
#define I2C_CONF_AUTO_ON_DELAY                  18
#define I2C_CONF_POWER_CUT_DELAY                19
#define I2C_CONF_WATCHDOG_THRESHOLD             20
#define I2C_CONF_OVER_VOLTAGE_TOLERANCE         21
#define I2C_CONF_UNDER_VOLTAGE_TOLERANCE        22
#define I2C_CONF_TEMPERATURE_CONDITION_1        23
#define I2C_CONF_FAN_SPEED_1                    24
#define I2C_CONF_TEMPERATURE_CONDITION_2        25
#define I2C_CONF_FAN_SPEED_2                    26
#define I2C_CONF_TEMPERATURE_CONDITION_3        27
#define I2C_CONF_FAN_SPEED_3                    28
#define I2C_CONF_VIN_CALIBRATION                29
#define I2C_CONF_VOUT_CALIBRATION               30
#define I2C_CONF_IOUT_CALIBRATION               31


extern int (*get_temp_cond_funcs[3])(int);
extern void (*set_temp_cond_funcs[3])(int, int);

extern int (*get_fan_speed_funcs[3])(int);
extern void (*set_fan_speed_funcs[3])(int, int);


void restart_with_sudo(int argc, char *argv[]);

int init_i2c_device(unsigned char * bus, unsigned char addr);

void close_i2c_device(int fd);

int write_to_i2c(int i2c_fd, int reg, int value);

int read_from_i2c(int i2c_fd, int reg);

signed char get_signed_char(unsigned char n);

int read_temperature(const char *path);

int get_fw_version(int i2c_fd);

int is_powered_via_extender(int i2c_fd);

float get_vin(int i2c_fd, bool calibrate);

float get_vout(int i2c_fd, bool calibrate);

float get_iout(int i2c_fd, bool calibrate);

int get_heartbeat_counter(int i2c_fd);

int get_missed_heartbeats(int i2c_fd);

int get_cpu_temperature(int i2c_fd);

int get_gpu_temperature(int i2c_fd);

int get_fan_speed(int i2c_fd);

int get_auto_on_delay(int i2c_fd);

void set_auto_on_delay(int i2c_fd, int delay);

int get_power_cut_delay(int i2c_fd);

void set_power_cut_delay(int i2c_fd, int delay);

int get_watchdog_threshold(int i2c_fd);

void set_watchdog_threshold(int i2c_fd, int threshold);

int get_overvoltage_tolerance(int i2c_fd);

void set_overvoltage_tolerance(int i2c_fd, int tolerance);

int get_undervoltage_tolerance(int i2c_fd);

void set_undervoltage_tolerance(int i2c_fd, int tolerance);

int get_temp_cond1(int i2c_fd);

void set_temp_cond1(int i2c_fd, int temp);

int get_fan_speed1(int i2c_fd);

void set_fan_speed1(int i2c_fd, int speed);

int get_temp_cond2(int i2c_fd);

void set_temp_cond2(int i2c_fd, int temp);

int get_fan_speed2(int i2c_fd);

void set_fan_speed2(int i2c_fd, int speed);

int get_temp_cond3(int i2c_fd);

void set_temp_cond3(int i2c_fd, int temp);

int get_fan_speed3(int i2c_fd);

void set_fan_speed3(int i2c_fd, int speed);

int get_vin_calibration(int i2c_fd);

void set_vin_calibration(int i2c_fd, int cali);

int get_vout_calibration(int i2c_fd);

void set_vout_calibration(int i2c_fd, int cali);

int get_iout_calibration(int i2c_fd);

void set_iout_calibration(int i2c_fd, int cali);

#endif  /* __VUELIB_H */
