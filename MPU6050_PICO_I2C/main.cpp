#include <iostream>


#define I2C_PORT i2c1
#include "pico/stdlib.h"
#include "hardware/i2c.h"




class KC_MPU6050{
    //List of Address
    const int addr          = 0x68;
    uint8_t WHO_AM_I        = 0x75;
    uint8_t DLPF            = 0x1A;
    uint8_t GYRO_CONFIG     = 0x1B;
    uint8_t GYRO_OUT        = 0x43;
    uint8_t PWR_MGMT_1      = 0x6B;
    uint8_t ACCEL_CONFIG    = 0x1C;
    uint8_t ACCEL_OUT       = 0x3B;


    //Special value
    float gyro_lsb = 131;
    float accel_lsb = 16384;
    int16_t GyroRawX = 0, GyroRawY = 0, GyroRawZ = 0;
    int16_t AccelRawX = 0, AccelRawY = 0, AccelRawZ = 0;

public:

    
    float GyroX = 0, GyroY = 0, GyroZ = 0;
    float AccelX = 0, AccelY = 0, AccelZ = 0;

    int mpu_init(); //Checking if the value is connected
    int mpu_DLPF(uint8_t dlfp_cfg); //Setting Up Low pass filter
    int mpu_GYRO_CONFIG(uint8_t fs_sel); //Setting up GYRO_CONFIG
    int mpu_ACCEL_CONFIG(uint8_t afs_sel);
    void get_gyro(); //Getting Gyro Data
    void get_accel();
    void power_mode();
};


int KC_MPU6050::mpu_init(){

    //Check if the device is connected
    sleep_ms(1000);
    uint8_t chipID[1];
    i2c_write_blocking(I2C_PORT,addr,&WHO_AM_I,1,true);
    i2c_read_blocking(I2C_PORT,addr,chipID,1,false);

    if(chipID[0] == 0x68){
        printf("Sucessfully connected to MPU-6050");
        return true;
    }
       printf("False to connected to MPU-6050");
    return false;

}
int KC_MPU6050::mpu_DLPF(uint8_t dlfp_cfg){

    if(dlfp_cfg> 7 || dlfp_cfg < 0){
        std::cout << "Please enter configuration value between 0 and 7" << std::endl;
        return false;
        
    }
    uint8_t data[2];
    data[0] = DLPF;
    data[1] = dlfp_cfg;
    i2c_write_blocking(I2C_PORT,addr,data,2,false);
    std::cout << "Sucessfully Set the low pass filter" << std::endl;
    return true;
}


int KC_MPU6050::mpu_GYRO_CONFIG(uint8_t fs_sel){
    if(fs_sel < 0 || fs_sel > 3){
        std::cout << "Please enter a value betweem 0 and 3" << std::endl;
        return 0;
    }

    int range = 250;
    uint8_t data[2];
    data[0] = GYRO_CONFIG;

    switch (fs_sel)
    {
    case 0:
        data[1] = 0x00;
        gyro_lsb = 131;
        break;

    case 1:
        data[1] = 0x08;
        gyro_lsb = 65.5;
        range = 500;
        break;
    
    case 2:
        data[1] = 0x10;
        gyro_lsb = 32.8;
        range = 1000;
        break;
    
    case 3:
        data[1] = 0x18;
        gyro_lsb = 16.4;
        range = 2000;
        break;

    default:
        data[1] = 0x00;
        gyro_lsb = 131;
        break;
    }

    i2c_write_blocking(I2C_PORT,addr,data,2,false);
    std::cout << "Setting Gyro Full Scale Range to +-"<< range <<"degree/s" << std::endl;
    return 1;
}


void KC_MPU6050::get_gyro(){

    uint8_t data[6];
    data[0] = GYRO_OUT;
    i2c_write_blocking(I2C_PORT,addr,&GYRO_OUT,1,true);
    i2c_read_blocking(I2C_PORT,addr,data,6,false);

    GyroRawX = (data[0] << 8) | data[1];
    GyroRawY = (data[2] << 8) | data[3];
    GyroRawZ = (data[4] << 8) | data[5];

    GyroX = GyroRawX/gyro_lsb;
    GyroY = GyroRawY/gyro_lsb;
    GyroZ = GyroRawZ/gyro_lsb;
    power_mode();
}

void KC_MPU6050::power_mode(){
    uint8_t data[2];
    data[0] = PWR_MGMT_1;
    data[1] = 0x00;

    i2c_write_blocking(I2C_PORT,addr,data,2,false);
}


int KC_MPU6050::mpu_ACCEL_CONFIG(uint8_t afs_sel){
    
    if(afs_sel < 0 || afs_sel > 3){
        std::cout << "Please enter a value between 0 and 3" << std::endl;
        return 0;
    }

    int range = 2;
    uint8_t data[2];
    data[0] = ACCEL_CONFIG;

    switch(afs_sel){
        case 0:
            data[1] = 0x00;
            accel_lsb = 16384;
            range = 2;
            break;
        
        case 1:
            data[1] = 0x08;
            accel_lsb = 8192;
            range = 4;
            break;

        case 2:
            data[1] = 0x10;
            accel_lsb = 4096;
            range = 8;
            break;
        
        case 3:
            data[1] = 0x18;
            accel_lsb = 2048;
            range = 16;
            break;
        
        default:
            data[0] = 0x00;
            accel_lsb = 16384;
            range = 2;
            break;
    }

    std::cout << "Setting Accelerometer range to +-" << range <<"g"<< std::endl;

    return 1;
}

void KC_MPU6050::get_accel(){
    uint8_t data[6];
    data[0] = ACCEL_OUT;

    i2c_write_blocking(I2C_PORT,addr,&ACCEL_OUT,1,true);
    i2c_read_blocking(I2C_PORT,addr,data,6,false);

    AccelRawX = (data[0] << 8) | data[1];
    AccelRawY = (data[2] << 8) | data[3];
    AccelRawZ = (data[4] << 8) | data[5];

    AccelX = AccelRawX/accel_lsb;
    AccelY = AccelRawY/accel_lsb;
    AccelZ = AccelRawZ/accel_lsb;
    power_mode();
}

int main(int argc, char** argv){

    stdio_init_all();

    sleep_ms(1000);

    int start = 0;
    printf("Starting MPU Progarm\n");
    while(start == 0){
        printf("Please Enter a number different from 0 to start :\n");
        scanf("%d",&start);
        sleep_ms(1000);
    }
    

    
    i2c_init(I2C_PORT, 400000);
    gpio_set_function(2,GPIO_FUNC_I2C);
    gpio_set_function(3,GPIO_FUNC_I2C);
    gpio_pull_up(2);
    gpio_pull_up(3);


    KC_MPU6050 MPU;

    if(!MPU.mpu_init())
        return -1;

    //Choosing Low pass filter
    if(!MPU.mpu_DLPF(7))
        return -1;

    if(!MPU.mpu_GYRO_CONFIG(1))
        return -1;
    
    if(!MPU.mpu_ACCEL_CONFIG(1))
        return -1;

    while (true)
    {
        MPU.get_gyro();
        std::cout << "The Gyro Scope Data for (X,Y,Z) in degree/s is (" << MPU.GyroX << "," << MPU.GyroY << "," << MPU.GyroZ << ") degree/s\n";
        MPU.get_accel();
        std::cout << "The Accelerometer Data for (X,Y,Z) in g is (" <<MPU.AccelX << "," << MPU.AccelY <<"," << MPU.AccelZ << ") g\n";
        sleep_ms(500);
    }
    
}