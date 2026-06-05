# Industrial Fail-Safe Relay Controller

## Project Overview
`11_industrial_relay_controller` is an industrial-grade ESP32 relay controller designed for deterministic startup behavior, fail-safe initialization, and resilient runtime supervision. The firmware emphasizes non-blocking sequencing and watchdog-backed recovery so the controller can operate predictably in unattended edge deployments.

## Key Engineering Features

### Watchdog Supervision
The application enables the ESP32 task watchdog to detect stalled execution paths and force recovery if the control loop becomes unresponsive. A diagnostic fault-injection path is also included to validate reset behavior under simulated deadlock conditions.

### Fail-Safe Initialization
Relay outputs are driven to the OFF state before the GPIOs are configured as outputs. This startup ordering reduces the risk of inadvertent energization during power-on, boot transitions, or partial reset events.

### Non-Blocking Sequencing
Relay channels are activated sequentially using a time-based state machine rather than blocking delays. This preserves loop responsiveness, keeps watchdog servicing active, and allows the firmware to transition cleanly from startup sequencing to steady-state operation.

## Hardware Connections

| ESP32 GPIO | Peripheral Pin | Function |
| --- | --- | --- |
| GPIO 18 | IN1 | Relay channel 0 |
| GPIO 19 | IN2 | Relay channel 1 |
| GPIO 21 | IN3 | Relay channel 2 |
| GPIO 22 | IN4 | Relay channel 3 |

Relay outputs are configured for active-high drive logic, with the OFF state asserted during initialization.

## Safety & Resilience
Industrial relay control systems must behave predictably under power cycling, software faults, and transient execution delays. This design addresses those requirements through three core practices: deterministic output initialization, non-blocking control flow, and task watchdog supervision.

Deterministic initialization reduces the chance of unintended relay closure at boot, which is critical when controlling loads such as contactors, pumps, or safety interlocks. Non-blocking sequencing keeps the firmware responsive to system supervision and prevents long startup routines from starving diagnostics or recovery paths. Watchdog integration provides a last-line recovery mechanism if the application enters a deadlock or unbounded loop, improving availability in unattended environments.

## Test Procedure
1. Flash the firmware to the ESP32 and open the serial monitor at `115200` baud.
2. Verify the startup log shows the hardware initialization and sequencing state transitions.
3. During normal operation, enter `f` in the serial terminal to simulate a fatal deadlock.
4. Confirm the system stops servicing the loop and the watchdog triggers an automatic reset within the configured timeout.
5. After reboot, verify the relay outputs return to the safe OFF state before sequencing resumes.

Together, these patterns support a fail-safe operational model aligned with industrial reliability expectations and edge-device resilience requirements.
