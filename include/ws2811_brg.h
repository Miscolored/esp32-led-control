/*
Each X ms, a train of Y signals is sent on the data line
Each signal is composed of 24bit RGB (configurable order) value
delivered as PWM modulation of a base Z Hz carrier.

TODO
enter X, Y, Z
*/


// the PWM duty cycle for sending a 1 or 0
#define BIT0 30
#define BIT1 80

// the microsecond duration of the signal
#define BITLENGTH 1.12
#define BYTELENGTH 10.5
#define RGBLENGTH 30
#define RESET 50
#define TRAIN 4000



void colorToSignal(uint8_t color) {
    return
}