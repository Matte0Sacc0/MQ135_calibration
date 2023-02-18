/*
 * Author: Matteo Sacco
 * GitHub: Matte0Sacc0 -> https://github.com/Matte0Sacc0
 * Date: 2022-03-16
 * 
 * This sketch provides a correct calibration of the MQ135 sensor
 *  
 * ******************************************************
 * *** READ ALL THE PROCESS BEFORE RUNNING THE SKETCH ***
 * ******************************************************
 * 
 * 
 * ^^^ IMPORTANT ^^^
 *  The sensor needs to be pre-warmed up for at least 24 hours before use, as specified in the specs.
 *  To warm up the sensor plug it into the current with a power supply module and leave it running without
 *  touching it for 24h (36h or 48h are better if possible, if th room in which the sensor is left has an high
 *  humidity this time could be necessary), in this phase just plug in VCC and GND pins and do not connect it
 *  to any board to be safe.
 * 
 * 
 * !!! WARNING !!!
 *  After many attempts I have found that the potentiometer on the sensor needs to be placed
 *  at around 3/4 of the maximum sensitivity (clockwise rotation)
 * 
 * !!! WARNING !!!
 *  After the calibration has been done the regulation potentiometer must not be rotated again
 *  
 *  
 * ^^^ IMPORTANT ^^^
 *  Define the values of:
 *    int DEFAULT_TEMPERATURE = 20;
 *    int DEFAULT_HUMIDITY = 33;
 *  before running the sketch otherwise the sensor will be calibrated assuming room's temperature of 20°C and
 *  room's relative humidity of 33%. These are the default value used by the function "getRZero();", but in this sketch is otherwise
 *  used the more specific function "getCorrectedRZero(DEFAULT_TEMPERATURE, DEFAULT_HUMIDITY);", in which these 2 values matter.
 *  I've initialized them as the default ones so if you can't define them the sketch will be executed as default.
 * 
 *  
 * 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * 
 * --- STEPS: ---
 * 
 *  - Warm up the internal coil everytime you power up the sensor -> (2 minutes after the sensor has been powered up)
 *  - Define the gasSensor() -> "MQ135 gasSensor = MQ135(SENSOR_PIN);"
 *  - Begin the RZERO reading from the sensor until it begins to stabilize (around 30/40 minutes)
 *  - Third the value of he sum of each RZERO has to be divided for the value of count to obtaint the sensor's specific RZERO
 *    (...Read the advice below to obtain a better calibration...)
 *  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * 
 * 
 * ??? ADVICE ???
 *  To obtain a better calibration of the RZERO value it's better to discard the first counts of RZERO readings
 *  to do that open the Serial monitor on (115200 baud, or initialize it differently) and read the value of count
 *  at the round just before the value begins to stabilize.
 *  After that subtract the read RZERO sum to the total sum and the read count to the total count and use
 *  the new values to make the corrected division for the calibration.
 *  (...an example is shown in the 'MQ135_calibration.txt' file...)
 * 
 * 
 * ??? WHAT TO DO AFTER THE SENSOR HAS BEEN CALIBRATED ???
 *  After the sensor has been calibrated you can save the obtained RZERO as OBTAINED_RZERO and define the sensor
 *  as "MQ135 gasSensor = MQ135(SENSOR_PIN, OBTAINED_RZERO);" in every sketch where you use it.
 *  Remember to always warm up the sensor for 2 minutes after you plug it into the current because the internal coil needs
 *  to be dryed from the humidity of the room before readung the values.
 *  
 */

#include <MQ135.h>

#define SENSOR_PIN 34
MQ135 gasSensor = MQ135(SENSOR_PIN);

int DEFAULT_TEMPERATURE = 20;
int DEFAULT_HUMIDITY = 30;

void setup() {
  Serial.begin(115200);
}

float most_frequent_value(float values[], int n) {
    // count the occurrences of each value in the array
    int max_count = 0;
    float max_value = 0;
    for (int i = 0; i < n; i++) {
        int count = 0;
        for (int j = 0; j < n; j++) {
            if (values[j] == values[i]) {
                count++;
            }
        }
        if (count > max_count) {
            max_count = count;
            max_value = values[i];
        }
    }
    return max_value;
}

float weighted_mean(float values[], int n) {
    float most_frequent = most_frequent_value(values, n);
    
    float sum = 0.0;
    float weight_sum = 0.0;
    for (int i = 0; i < n; i++) {
        // the weight of each value gets calculate based on the attinence with the most frequent value
        float weight = n - abs(values[i] - most_frequent);
        sum += values[i] * weight;
        weight_sum += weight;
    }
    return sum / weight_sum;
}

void warmUpMQ135()
{
    Serial.println("Warming up sensor: (...this operation takes 2 minutes...)");
    for (int i = 0; i < 120; i++)
    {
      if ((i + 1) % 5 == 0)
      {
         float ps = (i + 1) * 0.84;
         Serial.print("_" + String((int)ps) + "%");
      }
      gasSensor.getPPM();
      delay(1000);
    }
    Serial.println("\n--- Sensor warmed up correctly");
}

void calibrateMQ135()
{
    warmUpMQ135();
    float rzero_sum = 0.0;
    int count = 0;
    float r_zeros[180]; 

    while (count < 180)
    {
      float rz = gasSensor.getCorrectedRZero(DEFAULT_TEMPERATURE, DEFAULT_HUMIDITY);
      r_zeros[count] = rz;
      rzero_sum += rz;
      count++;
      Serial.println(String(count) + " : " + String(rzero_sum));
      Serial.println(String(rz) + "\n------------------------------");
      delay(20000);
    }

    Serial.println("Calculating the weighted mean of all the values read ... ");
    float r_zero = weighted_mean(r_zeros, count);
    Serial.println("The R_Zero found after calibration is : " + String(r_zero));
}

void loop() {
  calibrateMQ135();
}
