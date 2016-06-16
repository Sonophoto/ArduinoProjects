/* **************************************************************** *
                 _____          
   ARDUINO  /// / ___/ ///       
               / /__        
               \___/                                               
    __        __         _   _     _             
    \ \      / /__  __ _| |_| |__ (_)_ __   ___  
     \ \ /\ / / _ \/ _` | __| '_ \| | '_ \ / _ \ 
      \ V  V /  __/ (_| | |_| | | | | | | | (_) |
       \_/\_/ \___|\__,_|\__|_| |_|_|_| |_|\___/ 
            Rain or Shine its weather time!                                    
      
    FILENAME: Weathino.ino 
     PURPOSE: "Collects temperature, humidity, air pressure, sunlight and outputs on Serial0 every 2 mins"
        TAGS: "Arduino, Arduino.org, weather, weather station'
      AUTHOR: "Brig Young" 
   COPYRIGHT: "2016 Brig Young"
     LICENSE: "BSD 2 Clause, see LICENSE_BSD"         
 * **************************************************************** */
#include "dht22.hcpp"

// Macros
#define TWO_MINUTES 119000 // leave room for update_indicator()
#define BLINK_TIME 100
#define UPDATE_DELAY 200
#define POWER_PIN 13
#define ILLUMINATION_PIN A1

// Globals, these are the data points we are recording
int temperature;
int humidity;
int pressure;
int illumination;

// Dynamic globals
char * data_packet;
char * data_buffer;

// Proto functions
void update_indicator();
char* format_data_packet(int, int, int, int);

int get_termperature();
int get_humidity();
int get_pressure();
int get_illumination();

// Arduino setup and loop
void setup() {
   data_packet = (char*) calloc(64, sizeof(char)); // no free() its reused until exit()
   data_buffer = (char*) calloc(64, sizeof(char)); // ditto..
   pinMode(POWER_PIN, OUTPUT);
   Serial.begin(115200); 
   while (!Serial); // Spinlock for USB-Serial handle.
}

void loop() {
  temperature  = get_termperature();
  humidity     = get_humidity();
  pressure     = get_pressure();
  illumination = get_illumination();
  data_packet  = format_data_packet(temperature, humidity, pressure, illumination);  
  Serial.print(data_packet);
  update_indicator();
// development/debug delay() ONLY  
  delay(2000); 
  // delay(TWO_MINUTES);
}

/* ************************************************************************************
 * N O T H I N G   B U T   B O D I E S   B E L O W   T H I S   L I N E
 *
 */

char * format_data_packet(int temp, int hum, int pres, int light) {
  sprintf(data_buffer,"{%d, %d, %d, %d}\n", temp, hum, pres, light); 
  return data_buffer;
}

/*
 * DHT-22 Temperature and Relative Humidity Sensor
 *
 * These use a library written by Luca Niccoli and provided under an MIT License
 * Documentation: http://lultimouomo.github.io/DHT22/
 *
 * NOTE: Library has been modified to single file #include like SQL-Lite.
 */
int get_termperature() {
    return 25;
}
int get_humidity() {
    return 99;
}

/*
 * SOMETHING Barometric Pressure Sensor
 *
 * 
 * Documentation: 
 */
int get_pressure() {
    return 1000;
}

/*
 * Cadmium Sulfide Illumination Sensor
 *
 * Documentation: This is a simple CDS cell biased at 5v on a 10bit ADC. We average
 *                3 fast readings to get a more stable number. Our goal here is just
 *                daily fluctions for charting.
 */
int get_illumination() {
#define ILLUM_NUM_READS 3
#define ILLUM_DELAY_INTERVAL 200 // milli seconds
    int sum_readings = 0;
    for (int index = 0; index < ILLUM_NUM_READS; index++) {
       sum_readings += analogRead(ILLUMINATION_PIN);
       delay(ILLUM_DELAY_INTERVAL);
    }   
    return 100;
    // return sum_readings / NUM_READS;
}

void update_indicator() {
   for(int i = 0; i < 5; i++) {
      // We are flashing the POWER LED which begins HIGH
      digitalWrite(POWER_PIN, LOW); delay(UPDATE_DELAY);
      digitalWrite(POWER_PIN, HIGH); delay(UPDATE_DELAY); // last delay() is useless
   }
}


