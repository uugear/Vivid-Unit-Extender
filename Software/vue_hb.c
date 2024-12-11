#include <stdio.h>
#include <unistd.h>
#include "vuelib.h"

#define HEARTBEAT_INTERVAL 5

int main(int argc, char **argv) {
    if (geteuid() != 0)
    {
        printf("Please run %s with sudo.\n", argv[0]);
        return 1;
    }
    
    int i2c_fd = init_i2c_device(I2C_BUS, I2C_ADDR);
    if (i2c_fd < 0) {
      return 1;
    }
    
    printf("Vivid Unit heartbeat daemon V%.2f started.", VUE_VERSION);
    
    unsigned char hb_counter = 0;

    while (1) {
        // increase and submit heartbeat counter
        hb_counter ++;
        write_to_i2c(i2c_fd, I2C_DATA_HEARTBEAT_COUNTER, hb_counter);
      
        // read and submit CPU temperature
        int cpu_temp = read_temperature(CPU_TEMP_PATH);
        if (cpu_temp >= 0) {
          write_to_i2c(i2c_fd, I2C_DATA_CPU_TEMPERATURE, cpu_temp);
        }

        // read and submit GPU temperature
        int gpu_temp = read_temperature(GPU_TEMP_PATH);
        if (gpu_temp >= 0) {
          write_to_i2c(i2c_fd, I2C_DATA_GPU_TEMPERATURE, gpu_temp);
        }

        sleep(HEARTBEAT_INTERVAL);
    }

    close_i2c_device(i2c_fd);
    return 0;
}

