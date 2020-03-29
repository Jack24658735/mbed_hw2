#include "mbed.h"

BusOut display(D6, D7, D9, D10, D11, D5, D4, D8);
char table[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};

DigitalIn Switch(SW3);
DigitalOut redLED(LED1);
DigitalOut greenLED(LED2);
Serial pc(USBTX, USBRX);
AnalogOut Aout(DAC0_OUT);
AnalogIn Ain(A0);
Timer t;

const int Sample_pts = 1000; // set up how many sample points 
const int PERIOD_SAMPLE = 30; // take 30 periods to calculate the freq
const int LED_ON = 0; // define the value to turn on LED
const int LED_OFF = 1; // define the value to turn off LED

double timer; // store the timer value
double ADCdata; // take in the data for calculating the freq
double ADCdata_s[1000]; // data for FFT analysis
double prev_data, next_data = 0.0; // store temp data
int freq;   // freq result
int temp_freq;  // for ssd display
int output_freq; // for output wave use
int digit_cnt = 1; // count how many digits for the frequency value
int period_cnt = 0; // count how many period passed now

void output_sine() {
    // This part is for output the sine wave
    int cnt = 0; 
    // for displaying the graph on picoscope for a moment
    
    while (cnt < 1) {
        for (float i = 0; i < 1; i += 0.0001) {
            // M_PI is the pi value defined in math.h
            Aout = 0.5 + 0.5 * sin(i * 2 * M_PI * output_freq);
            wait(0.001);
        }
        cnt++;
    }
    // Then, return to the main for getting the new button value
}

int main(){

    // This part is for FFT analysis
    // Since FFT analysis needs some times, we have to wait a moment
    for (int i = 0; i < Sample_pts; ++i) {
        ADCdata_s[i] = Ain;
        wait(1./Sample_pts);
    }
    for (int i = 0; i < Sample_pts; ++i) {
        pc.printf("%1.3f\r\n", ADCdata_s[i]);
    }
/*******************************************************/

    // This part is for display and calculate frequency
    t.start(); // start the timer
    while (1) {
        ADCdata = Ain;
        prev_data = next_data;
        next_data = ADCdata; // obtain new value from Ain
    
        // detect the crossing point
        if (prev_data < 0.001 && next_data >= 0.001) {
            // press the button and period counter reaches the goal I set up
            if (period_cnt == PERIOD_SAMPLE && Switch == 0) {
                greenLED = LED_OFF;
                redLED = LED_ON;
                timer = t.read();
                freq = round((PERIOD_SAMPLE - 1) / timer);
                //pc.printf("%d\n", freq);

                temp_freq = freq;   // temp_freq is used for displaying
                output_freq = freq; // output_freq is used to output
                
                // obtain how many digits
                while (temp_freq != 0) {
                    temp_freq /= 10;
                    digit_cnt *= 10;
                }

                // separate the digits for displaying
                while (digit_cnt > 1) {
                    digit_cnt /= 10;
                    if (digit_cnt == 1) {
                        // The last digit
                        display = table[freq / digit_cnt] | 0x80;
                    }
                    else {
                        display = table[freq / digit_cnt];
                    }
                    wait(1);
                    freq %= digit_cnt;
                }
                
                display = 0x00; // turn off the display
                redLED = LED_OFF;
                greenLED = LED_ON;
                // call the function to output the sine wave
                output_sine(); 
                period_cnt = 0;
                t.reset();
                
            } else if (period_cnt == PERIOD_SAMPLE) {
                period_cnt = 0;
                t.reset();
            } else {
                period_cnt++;   // increase the period counter
                greenLED = LED_ON;
                redLED = LED_OFF;
            }
        }
        if (period_cnt != 0) {
            wait_ms(0.001);
        }
    }
}
