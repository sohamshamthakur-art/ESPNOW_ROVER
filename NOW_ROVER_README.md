# NOW_ROVER

## Simple Overview
In simple language, here is exactly what this project does: Reads your hand movements using an accelerometer → Packages that data into a small digital packet → Blasts it wirelessly to the rover using the ESP-NOW protocol (no router required) → The rover instantly catches the packet → Calculates the exact tilt angle of your hand → Translates the forward/backward tilt into proportional motor speeds → Translates the left/right tilt into a servo steering angle → Drives the rover seamlessly!

## Technical Overview
From an engineering perspective, this system is an exploration of low-latency wireless control and basic sensor-to-actuator kinematics.
* **Wireless Protocol:** It utilizes ESP-NOW for peer-to-peer, connectionless communication, which drastically reduces latency compared to standard TCP/IP over a local network.
* **Sensor Integration:** The transmitter interfaces with an MPU6050 IMU over I2C, reading 6-axis acceleration and gyroscope data at a rapid interval (every 25ms). The sensor's low-pass filter bandwidth is set to 21Hz to smooth out hand jitters.
* **Kinematics & Math:** The receiver parses the incoming packet and uses the atan2 function on the gravity vectors (Y and Z for the servo, X and Z for the motors) to calculate the absolute tilt angles.
* **Actuation & Filtering:** The motor drive logic features a built-in deadband of ±10 degrees to prevent the motors from twitching when the transmitter is relatively flat. Once pushed past the deadband, it maps the tilt (up to 30 degrees) to a PWM value (0-255) for smooth, proportional acceleration.

## The Motivation
The traditional joystick control gets boring. The core motivation for this project was to:
1. Explore gesture-controlled robotics, creating a more intuitive, "Jedi-like" interface between human and machine.
2. Learn the ESP-NOW protocol, understanding how to send high-speed, headless data packets between microcontrollers without relying on an external WiFi infrastructure.
3. Explore the way for Swarm Robotics. By understanding peer-to-peer data broadcasting, this single-rover architecture can eventually be scaled up to control massive swarms of synchronized robots or other interconnected smart nodes.

## PINOUTS (TRANSMITTER)

| MPU6050 | ESP32 CAM (NOT NECESSARY TO USE IT) |
| :--- | :--- |
| VCC | 3V3 |
| GND | GND |
| SDA | GPIO 14 |
| SCL | GPIO 15 |

## PINOUTS (RECEIVER)

| SERVO MOTOR | ESP32 |
| :--- | :--- |
| VCC | 5V |
| GND | GND |
| DATA | GPIO 33 |

| L298N | ESP32 |
| :--- | :--- |
| ENA | GPIO 25 |
| IN1 | GPIO 26 |
| IN2 | GPIO 27 |
| IN3 | GPIO 12 |
| IN4 | GPIO 13 |
| ENB | GPIO 14 |

| BATTERY | L298N |
| :--- | :--- |
| 12V | VCC |
| GND | GND (COMMON GND TO ESP AND DRIVER) |

*Note: OUT 1, 2, 3, 4 on the L298N are for BO MOTORS.*

## How It Works & How to Use It

### The Working Process: 
The transmitter continuously reads the MPU6050 and sends a DataPacket struct containing the X, Y, and Z values. When the receiver's ESP32 catches this packet, a callback function (onReceive) is triggered. 

The receiver extracts the X-axis tilt to determine if the L298N motor driver should push the DC motors forward, reverse, or stop completely. Simultaneously, it extracts the Y-axis tilt and maps it from a -75 to 75-degree range directly into a 0 to 180-degree position for the servo motor.

### Getting Started (Code Changes):
To get this working on your hardware, you only need to make a few tweaks:
* **Target MAC Address:** You must find the MAC address of your receiver ESP32 and update the receiverMAC array in the sender code (currently set to 0x7C, 0x9E, 0xBD, 0xF4, 0x6B, 0x64) so the remote knows exactly who to talk to.
* **Deadband Tuning:** If you have shaky hands, you can increase the deadband variable in the receiver code from 10.0 to something higher.
* **Sensitivity:** You can adjust the maxAngle variable (currently 30.0) in the receiver code. Lowering this will make the rover reach its top speed with less hand tilt.

## Future Enhancements
This setup is a launchpad for more advanced robotics:
* **Swarm Control:** Modifying the ESP-NOW sender to broadcast to a multicast address, allowing one hand gesture to drive 5, 10, or 50 rovers simultaneously.
* **Two-Way Telemetry:** Implementing bi-directional ESP-NOW communication so the rover can send battery voltage or obstacle-avoidance ping data back to an OLED screen on the remote.
* **Omnidirectional Movement:** Upgrading the chassis to use Mecanum wheels, mapping the gyroscope's Z-axis to allow the rover to strafe side-to-side.

## Bye-bye!
Thanks for checking out the project. Have fun building your rover, and good luck with your future swarm!
