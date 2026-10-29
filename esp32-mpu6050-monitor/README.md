# ESP32 Wireless Motion Monitoring Prototype

An academic prototype by Asadulloh Yahyoxonov, a master's student in mechatronics and robotics. Developed with AI assistance and tested on physical ESP32 hardware. This is a research prototype, not a validated tool-wear classifier or safety system.

## Architecture

MPU6050 → ESP32-C3 → ESP-NOW → ESP32 gateway → local Wi-Fi dashboard, LED alarms and I2C LCD.

The sensor firmware targets 100 samples per second. The gateway supports stationary-reference calibration, relative tilt, acceleration and angular velocity display, packet counters, stale-data detection and browser CSV export. The dashboard source is in Uzbek.

## Hardware and setup

- ESP32-C3: MPU6050 SDA GPIO9, SCL GPIO8, 3.3 V, common ground.
- ESP32 Dev Module: red LED GPIO25, green GPIO26, yellow GPIO27; use series resistors.
- Optional PCF8574 LCD: SDA GPIO21, SCL GPIO22. Use level shifting if the module pulls I2C up to 5 V.
- Copy `gateway/config.example.h` to `gateway/config.h`, set your sensor MAC and a unique AP password of at least 8 characters. Keep that file private.
- Arduino ESP32 core used in the original build: 3.3.10. Board targets: `esp32:esp32:esp32c3:CDCOnBoot=cdc` and `esp32:esp32:esp32`.
- Flash `sensor_c3/sensor_c3.ino` and `gateway/gateway.ino` to the matching boards. Both radios use channel 6.
- Join the configured gateway access point and open `http://192.168.4.1`.
- Hold the sensor stationary during calibration, then use the dashboard to record and export CSV.
- After editing `dashboard.html`, run `build-dashboard.ps1` with Node.js available to regenerate the embedded header and check JavaScript syntax.

## Evidence and limitations

Historical hardware validation is recorded in `VALIDATION.md` and `VALIDATION-V2.md` (Uzbek). Gateway serial checks recorded 98–100 received packets/s in that test, successful calibration and LCD detection. These are observations, not throughput guarantees. The validation files describe the original local build; the sanitized configuration in this portfolio must be set for each device and has not been reflashed in this session.

Browser fixture checks used synthetic data. They do not establish end-to-end behavior on physical hardware. The new gateway HTTP API was not verified from the computer on the actual AP during the recorded V2 session.

ESP-NOW broadcast is unencrypted; source-MAC filtering is not cryptographic authentication. Relative tilt is based on the acceleration reference and is inaccurate under dynamic motion. Calibration is not a full six-position calibration. Alarm thresholds are experimental. Sampling and network loss limit vibration analysis; this does not establish reliable cutting-tool wear detection.

`CSV-MATLAB.md` documents the export format. No claim of MQTT or Iotellect integration is made: this project uses ESP-NOW and a local HTTP dashboard.
