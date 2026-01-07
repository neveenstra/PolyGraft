#include "SensorCache.h"

SensorCache::SensorCache() 
    : imu(ICM20948_ADDR)
    , breathRaw(0)
    , expressionRaw(0)
    , pinchRaw(0)
    , accelX(0), accelY(0), accelZ(0)
    , gyroX(0), gyroY(0), gyroZ(0)
    , magX(0), magY(0), magZ(0)
    , temperature(0)
    , imuAvailable(false)
    , lastUpdate(0)
    , updateInterval(10)
{
}

bool SensorCache::begin() {
    pinMode(BREATH_PIN, INPUT);
    pinMode(EXPRESSION_PIN, INPUT);
    pinMode(PINCH_PIN, INPUT);
    analogReadResolution(12);
    

    Wire.begin();
    Wire.setClock(400000);
    
    if (!imu.init()) {
        Serial.println("ICM-20948 initialization failed!");
        imuAvailable = false;
    } else {
        Serial.println("ICM-20948 initialized successfully");
        imuAvailable = true;
        
        imu.setAccRange(ICM20948_ACC_RANGE_16G);
        

        imu.setAccDLPF(ICM20948_DLPF_6);
        imu.setAccSampleRateDivider(10);
        
        imu.setGyrRange(ICM20948_GYRO_RANGE_2000);

        imu.setGyrDLPF(ICM20948_DLPF_6);
        imu.setGyrSampleRateDivider(10);
        
        imu.setMagOpMode(AK09916_CONT_MODE_100HZ);
        
        imu.enableAcc(true);
        imu.enableGyr(true);
        
        imu.autoOffsets();
    }
    
    update();
    
    return imuAvailable;
}

void SensorCache::update() {
    unsigned long currentTime = millis();
    
    if (currentTime - lastUpdate >= updateInterval) {
        updateAnalogSensors();
        updateIMU();
        lastUpdate = currentTime;
    }
}

void SensorCache::updateAnalogSensors() {
    breathRaw = analogRead(BREATH_PIN);
    expressionRaw = analogRead(EXPRESSION_PIN);
    pinchRaw = analogRead(PINCH_PIN);
}

void SensorCache::updateIMU() {
    if (!imuAvailable) {
        return;
    }
    
    imu.readSensor();
    
    xyzFloat accel;
    imu.getGValues(&accel);
    accelX = accel.x;
    accelY = accel.y;
    accelZ = accel.z;
    
    xyzFloat gyro;
    imu.getGyrValues(&gyro);
    gyroX = gyro.x;
    gyroY = gyro.y;
    gyroZ = gyro.z;
    
    xyzFloat mag;
    imu.getMagValues(&mag);
    magX = mag.x;
    magY = mag.y;
    magZ = mag.z;
    
    temperature = imu.getTemperature();
}