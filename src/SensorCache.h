#ifndef SENSOR_CACHE_H
#define SENSOR_CACHE_H

#include <Arduino.h>
#include <ICM20948_WE.h>

class SensorCache {
public:
    SensorCache();
    
    bool begin();
    
    void update();
    
    uint16_t getBreathRaw() const { return breathRaw; }
    uint16_t getExpressionRaw() const { return expressionRaw; }
    uint16_t getPinchRaw() const { return pinchRaw; }
    
    float getBreathNormalized() const { return breathRaw / 4095.0f; }
    float getExpressionNormalized() const { return expressionRaw / 4095.0f; }
    float getPinchNormalized() const { return pinchRaw / 4095.0f; }
    
    float getBreathVoltage() const { return (breathRaw / 4095.0f) * 3.3f; }
    float getExpressionVoltage() const { return (expressionRaw / 4095.0f) * 3.3f; }
    float getPinchVoltage() const { return (pinchRaw / 4095.0f) * 3.3f; }
    
    float getAccelX() const { return accelX; }
    float getAccelY() const { return accelY; }
    float getAccelZ() const { return accelZ; }
    
    float getGyroX() const { return gyroX; }
    float getGyroY() const { return gyroY; }
    float getGyroZ() const { return gyroZ; }
    
    float getMagX() const { return magX; }
    float getMagY() const { return magY; }
    float getMagZ() const { return magZ; }
    
    float getTemp() const { return temperature; }
    
    bool isIMUAvailable() const { return imuAvailable; }
    
    void setUpdateInterval(unsigned long interval) { updateInterval = interval; }
    
private:

    static const uint8_t BREATH_PIN = 15;
    static const uint8_t EXPRESSION_PIN = 23;
    static const uint8_t PINCH_PIN = 16;
    
    static const uint8_t ICM20948_ADDR = 0x68;
    
    uint16_t breathRaw;
    uint16_t expressionRaw;
    uint16_t pinchRaw;
    
    float accelX, accelY, accelZ;
    float gyroX, gyroY, gyroZ;
    float magX, magY, magZ;
    float temperature;
    
    ICM20948_WE imu;
    bool imuAvailable;

    unsigned long lastUpdate;
    unsigned long updateInterval;
    
    void updateAnalogSensors();
    void updateIMU();
};

#endif