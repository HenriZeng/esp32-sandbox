/**
 * @file RelayManager.cpp
 * @brief Industrial-grade implementation for 4-channel relay control.
 * * This module manages the lifecycle of the relay hardware, ensuring
 * fail-safe initialization and non-blocking operational states.
 */

 #include "RelayManager.h"

 /**
  * @brief Constructor
  * Initializes the channel counter to the default state.
  */
 RelayManager::RelayManager() : currentChannel(0) {}
 
 /**
  * @brief Atomic hardware initialization.
  * * IMPORTANT: To prevent "power-on bounce" or inadvertent activation,
  * we explicitly write the 'OFF' state before configuring the pins as OUTPUT.
  */
 void RelayManager::initialize() {
     for (int i = 0; i < CHANNEL_COUNT; i++) {
         // High-Level Trigger: LOW means OFF
         digitalWrite(RELAY_PINS[i], LOW); 
         pinMode(RELAY_PINS[i], OUTPUT);
     }
 }
 
 /**
  * @brief Sets a specific relay channel to the desired state.
  * @param channel The relay index (0 to CHANNEL_COUNT - 1).
  * @param on True to energize (HIGH), false to de-energize (LOW).
  */
 void RelayManager::setChannelState(uint8_t channel, bool on) {
     if (channel < CHANNEL_COUNT) {
         // High-Level Trigger logic: HIGH energizes the relay coil
         digitalWrite(RELAY_PINS[channel], on ? HIGH : LOW);
     }
 }
 
 /**
  * @brief Sequential activation logic.
  * Used for power-on sequencing to prevent inrush current surges.
  */
 void RelayManager::activateNext() {
     if (currentChannel < CHANNEL_COUNT) {
         setChannelState(currentChannel, true);
         currentChannel++;
     }
 }
 
 /**
  * @brief Reset the manager to its initial state.
  * Useful for system recovery or emergency shutdown procedures.
  */
 void RelayManager::reset() {
     currentChannel = 0;
     for (int i = 0; i < CHANNEL_COUNT; i++) {
         setChannelState(i, false);
     }
 }