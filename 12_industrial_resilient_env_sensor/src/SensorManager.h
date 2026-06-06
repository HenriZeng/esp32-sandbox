#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>

class SensorManager {
public:
    void initialize(uint8_t pin);
    void update(); 
    float getTemperature() const;
    float getHumidity() const;

private:
    uint8_t _pin;
    float _lastTemperature = 0.0f;
    float _lastHumidity = 0.0f;
    unsigned long _lastReadTime = 0;
    
    enum class State {
        IDLE,
        REQUESTING,
        READING_PULSES,
        VALIDATING
    };
    State _currentState = State::IDLE;

    // --- ADD THIS LINE BELOW ---
    bool _readData(); 
    // ---------------------------
};

#endif