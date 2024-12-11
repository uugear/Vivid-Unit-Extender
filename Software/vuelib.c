#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include "vuelib.h"

int (*get_temp_cond_funcs[3])(int) = { get_temp_cond1, get_temp_cond2, get_temp_cond3 };
void (*set_temp_cond_funcs[3])(int, int) = { set_temp_cond1, set_temp_cond2, set_temp_cond3 };

int (*get_fan_speed_funcs[3])(int) = { get_fan_speed1, get_fan_speed2, get_fan_speed3 };
void (*set_fan_speed_funcs[3])(int, int) = { set_fan_speed1, set_fan_speed2, set_fan_speed3 };

void restart_with_sudo(int argc, char *argv[]) {
    char *new_argv[argc + 2];
    new_argv[0] = "sudo";
    for (int i = 0; i < argc; i++) {
        new_argv[i + 1] = argv[i];
    }
    new_argv[argc + 1] = NULL;
    execvp("sudo", (char*[]){"sudo", "-E", argv[0], NULL});
    perror("Failed to restart with sudo");
    exit(1);
}

int lock_file() {
    int lock_fd = open(I2C_LOCK, O_CREAT | O_RDWR, 0666);
    if (lock_fd < 0) {
        perror("Failed to open lock file");
        return -1;
    }
    if (flock(lock_fd, LOCK_EX) < 0) {
        perror("Failed to acquire lock");
        close(lock_fd);
        return -1;
    }
    return lock_fd;
}

void unlock_file(int lock_fd) {
    flock(lock_fd, LOCK_UN);
    close(lock_fd);
}


int init_i2c_device(unsigned char * bus, unsigned char addr) {
    // open I2C bus
    int i2c_fd = open(I2C_BUS, O_RDWR);
    if (i2c_fd < 0) {
        perror("Failed to open I2C bus");
        return -1;
    }
    // specify the address of the I2C device
    if (ioctl(i2c_fd, I2C_SLAVE, addr) < 0) {
        perror("Failed to communicate with I2C device");
        close(i2c_fd);
        return -1;
    }
    return i2c_fd;
}

void close_i2c_device(int fd) {
    close(fd);
}

// Write to the I2C register
int write_to_i2c(int i2c_fd, int reg, int value) {

    unsigned char buffer[2] = { reg, value };

    int lock_fd = lock_file();
    if (lock_fd < 0) {
        perror("Failed to lock I2C device for write");
        return -1;
    }

    // write the register index and value to I2C device
    if (write(i2c_fd, buffer, 2) != 2) {
        perror("Failed to write to I2C device");
        unlock_file(lock_fd);
        return -1;
    }

    unlock_file(lock_fd);
    return 0;
}

// Read from the I2C register
int read_from_i2c(int i2c_fd, int reg) {

    unsigned char buffer[1] = { reg };

    int lock_fd = lock_file();
    if (lock_fd < 0) {
        perror("Failed to lock I2C device for read");
        return -1;
    }

    // write the register index to I2C device
    if (write(i2c_fd, buffer, 1) != 1) {
        perror("Failed to write register address to I2C device");
        unlock_file(lock_fd);
        return -1;
    }

    // read the value from the register
    if (read(i2c_fd, buffer, 1) != 1) {
        perror("Failed to read from I2C device");
        unlock_file(lock_fd);
        return -1;
    }

    unlock_file(lock_fd);
    return buffer[0];
}

signed char get_signed_char(unsigned char n) {
  return (signed char)(n > 127 ? ((int)n - 256) : n);
}

// Read temperature from a thermal zone file
int read_temperature(const char *path) {
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        perror("Failed to open temperature file");
        return -1;
    }
    int temp;
    fscanf(file, "%d", &temp);  // temperature in millidegrees Celsius
    fclose(file);
    return temp / 1000;  // convert to degrees Celsius
}

int get_fw_version(int i2c_fd) {
  return read_from_i2c(i2c_fd, I2C_FIRMWARE_VERSION);
}

int is_powered_via_extender(int i2c_fd) {
  int state = read_from_i2c(i2c_fd, I2C_DEVICE_STATE);
  return (state&0x01);
}

float get_vin(int i2c_fd, bool calibrate) {
  int h = read_from_i2c(i2c_fd, I2C_INPUT_VOLTAGE_H);
  int l = read_from_i2c(i2c_fd, I2C_INPUT_VOLTAGE_L);
  float c = calibrate ? (float)get_signed_char(read_from_i2c(i2c_fd, I2C_CONF_VIN_CALIBRATION)) / 100 : 0;
  return ((float)((h << 8) | l)) / 1000 + c;
}

float get_vout(int i2c_fd, bool calibrate) {
  int h = read_from_i2c(i2c_fd, I2C_OUTPUT_VOLTAGE_H);
  int l = read_from_i2c(i2c_fd, I2C_OUTPUT_VOLTAGE_L);
  float c = calibrate ? (float)get_signed_char(read_from_i2c(i2c_fd, I2C_CONF_VOUT_CALIBRATION)) / 100 : 0;
  return ((float)((h << 8) | l)) / 1000 + c;
}

float get_iout(int i2c_fd, bool calibrate) {
  int h = read_from_i2c(i2c_fd, I2C_OUTPUT_CURRENT_H);
  int l = read_from_i2c(i2c_fd, I2C_OUTPUT_CURRENT_L);
  float c = calibrate ? (float)get_signed_char(read_from_i2c(i2c_fd, I2C_CONF_IOUT_CALIBRATION)) / 100 : 0;
  return ((float)((h << 8) | l)) / 1000 + c;
}

int get_heartbeat_counter(int i2c_fd) {
  return read_from_i2c(i2c_fd, I2C_DATA_HEARTBEAT_COUNTER);
}

int get_missed_heartbeats(int i2c_fd) {
  return read_from_i2c(i2c_fd, I2C_MISSED_HEARTBEAT);
}

int get_cpu_temperature(int i2c_fd) {
  return read_from_i2c(i2c_fd, I2C_DATA_CPU_TEMPERATURE);
}

int get_gpu_temperature(int i2c_fd) {
  return read_from_i2c(i2c_fd, I2C_DATA_GPU_TEMPERATURE);
}

int get_fan_speed(int i2c_fd) {
  read_from_i2c(i2c_fd, I2C_FAN_SPEED);
}

int get_auto_on_delay(int i2c_fd) {
  read_from_i2c(i2c_fd, I2C_CONF_AUTO_ON_DELAY);
}

void set_auto_on_delay(int i2c_fd, int delay) {
  write_to_i2c(i2c_fd, I2C_CONF_AUTO_ON_DELAY, delay);
}

int get_power_cut_delay(int i2c_fd) {
  read_from_i2c(i2c_fd, I2C_CONF_POWER_CUT_DELAY);
}

void set_power_cut_delay(int i2c_fd, int delay) {
  write_to_i2c(i2c_fd, I2C_CONF_POWER_CUT_DELAY, delay);
}

int get_watchdog_threshold(int i2c_fd) {
  read_from_i2c(i2c_fd, I2C_CONF_WATCHDOG_THRESHOLD);
}

void set_watchdog_threshold(int i2c_fd, int threshold) {
  write_to_i2c(i2c_fd, I2C_CONF_WATCHDOG_THRESHOLD, threshold);
}

int get_overvoltage_tolerance(int i2c_fd) {
  read_from_i2c(i2c_fd, I2C_CONF_OVER_VOLTAGE_TOLERANCE);
}

void set_overvoltage_tolerance(int i2c_fd, int tolerance) {
  write_to_i2c(i2c_fd, I2C_CONF_OVER_VOLTAGE_TOLERANCE, tolerance);
}

int get_undervoltage_tolerance(int i2c_fd) {
  read_from_i2c(i2c_fd, I2C_CONF_UNDER_VOLTAGE_TOLERANCE);
}

void set_undervoltage_tolerance(int i2c_fd, int tolerance) {
  write_to_i2c(i2c_fd, I2C_CONF_UNDER_VOLTAGE_TOLERANCE, tolerance);
}

int get_temp_cond1(int i2c_fd) {
  read_from_i2c(i2c_fd, I2C_CONF_TEMPERATURE_CONDITION_1);
}

void set_temp_cond1(int i2c_fd, int temp) {
  write_to_i2c(i2c_fd, I2C_CONF_TEMPERATURE_CONDITION_1, temp);
}

int get_fan_speed1(int i2c_fd) {
  read_from_i2c(i2c_fd, I2C_CONF_FAN_SPEED_1);
}

void set_fan_speed1(int i2c_fd, int speed) {
  write_to_i2c(i2c_fd, I2C_CONF_FAN_SPEED_1, speed);
}

int get_temp_cond2(int i2c_fd) {
  read_from_i2c(i2c_fd, I2C_CONF_TEMPERATURE_CONDITION_2);
}

void set_temp_cond2(int i2c_fd, int temp) {
  write_to_i2c(i2c_fd, I2C_CONF_TEMPERATURE_CONDITION_2, temp);
}

int get_fan_speed2(int i2c_fd) {
  read_from_i2c(i2c_fd, I2C_CONF_FAN_SPEED_2);
}

void set_fan_speed2(int i2c_fd, int speed) {
  write_to_i2c(i2c_fd, I2C_CONF_FAN_SPEED_2, speed);
}

int get_temp_cond3(int i2c_fd) {
  read_from_i2c(i2c_fd, I2C_CONF_TEMPERATURE_CONDITION_3);
}

void set_temp_cond3(int i2c_fd, int temp) {
  write_to_i2c(i2c_fd, I2C_CONF_TEMPERATURE_CONDITION_3, temp);
}

int get_fan_speed3(int i2c_fd) {
  read_from_i2c(i2c_fd, I2C_CONF_FAN_SPEED_3);
}

void set_fan_speed3(int i2c_fd, int speed) {
  write_to_i2c(i2c_fd, I2C_CONF_FAN_SPEED_3, speed);
}

int get_vin_calibration(int i2c_fd) {
  read_from_i2c(i2c_fd, I2C_CONF_VIN_CALIBRATION);
}

void set_vin_calibration(int i2c_fd, int cali) {
  write_to_i2c(i2c_fd, I2C_CONF_VIN_CALIBRATION, cali);
}

int get_vout_calibration(int i2c_fd) {
  read_from_i2c(i2c_fd, I2C_CONF_VOUT_CALIBRATION);
}

void set_vout_calibration(int i2c_fd, int cali) {
  write_to_i2c(i2c_fd, I2C_CONF_VOUT_CALIBRATION, cali);
}

int get_iout_calibration(int i2c_fd) {
  read_from_i2c(i2c_fd, I2C_CONF_IOUT_CALIBRATION);
}

void set_iout_calibration(int i2c_fd, int cali) {
  write_to_i2c(i2c_fd, I2C_CONF_IOUT_CALIBRATION, cali);
}
