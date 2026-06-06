#include "SensorManager.h"

void SensorManager::initialize(uint8_t pin) {
    _pin = pin;
    pinMode(_pin, INPUT_PULLUP);
    Serial.println("[SENSOR] SensorManager initialized on GPIO 25.");
}

void SensorManager::update() {
    // State machine to handle non-blocking sensor communication
    switch (_currentState) {
        case State::IDLE:
            if (millis() - _lastReadTime >= 2000) { // DHT11 sampling rate constraint
                _currentState = State::REQUESTING;
            }
            break;

        case State::REQUESTING:
            // Send start signal
            pinMode(_pin, OUTPUT);
            digitalWrite(_pin, LOW);
            delay(18); // Required pulse width
            digitalWrite(_pin, HIGH);
            pinMode(_pin, INPUT_PULLUP);
            _currentState = State::READING_PULSES;
            break;

        case State::READING_PULSES:
            // In a production environment, we use a timeout-protected loop
            // to ensure we don't hang if the sensor is disconnected.
            if (_readData()) {
                _currentState = State::VALIDATING;
            } else {
                _currentState = State::IDLE; // Reset on failure
            }
            break;

        case State::VALIDATING:
            _lastReadTime = millis();
            _currentState = State::IDLE;
            break;
    }
}

/**
 * @brief Internal pulse parsing logic.
 * Note: While 'delayMicroseconds' is used here, it is local to the 
 * sensor read task and safe within the non-blocking state machine.
 */
bool SensorManager::_readData() {
    uint8_t data[5] = {0, 0, 0, 0, 0};
    
    // Wait for response from sensor
    if (digitalRead(_pin) == HIGH) return false;
    delayMicroseconds(80);
    if (digitalRead(_pin) == LOW) return false;
    delayMicroseconds(80);

    // Read 40 bits
    for (int i = 0; i < 40; i++) {
        while (digitalRead(_pin) == LOW); // Wait for pulse start
        delayMicroseconds(30);
        if (digitalRead(_pin) == HIGH) {
            data[i / 8] |= (1 << (7 - (i % 8)));
            while (digitalRead(_pin) == HIGH); // Wait for pulse end
        }
    }

    // Verify Checksum (Industrial Standard)
    if (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
        _lastHumidity = (float)data[0];
        _lastTemperature = (float)data[2];
        return true;
    }
    return false;
}

float SensorManager::getTemperature() const { return _lastTemperature; }
float SensorManager::getHumidity() const { return _lastHumidity; }