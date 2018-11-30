# Arduino-Xbox-Trillian
Arduino code for the wireless xbox controller system used on Trillian.

You must install the libraries in the "Required Libraries" folder to compile the arduino sketch.

These are the pins on the arduino which you connect to the signal wire of the pwm cable comming from either the motor controller, relay, or sensor.

| Arduino Pin Number | Description |
| ------------- | ------------- |
| 28  | Back Left motor controller pin |
| 43  | Front Left motor controller pin |
| 32  | Back Right motor controller pin |
| 38  | Front Right motor controller pin |
| 42  | Back Left motor controller pin |
| 22  | Relay pin to extend the shooter piston |
| 26  | Relay pin to retract the shooter piston |
| 36  | Relay pin to control power to the air compressor |
| 24  | Input pin for the air pressure cut-off switch |

There is smoothing code in the arduino sketch which is unused. This smoothing code will be used to gradually decrease power to the motor controllers when the driver suddenly stops moving forward or backward. This code is currently not implemented as the code to run the motors will need to be greatly modified.
