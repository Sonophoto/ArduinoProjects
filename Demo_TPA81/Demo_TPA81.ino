/* FILENAME: Demo_TPA81.ino
 *   AUTHOR: Brig Young, sonophoto @ github
 *   PURPOSE: Outputs a header with firmware version and ambient temperature
 *   and then continuously outputs at a rate of 20 FPS a line of temperatures 
 *   representing:
 *   [Ambient, Pixel_1, Pixel_2, ... Pixel_8(linefeed)]
 *   Readily demonstrates FOV and Temp ranges using a candle flame.
 * 
 * Copyright (c) 2017 Brig Young All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <Wire.h>

// Utility constants
// relaible frame rate almost perfect 20 FPS
#define TPA81_FRAME_DELAY              48 // 20.04 FPS on Arduino Mega2560
// Fastest hardware frame rate, possibility of duplicate frames.
// #define TPA81_FRAME_DELAY              39 // 25.20 FPS on Arduino Mega2560
#define TPA81_AMBIENT_AND_PIXEL_COUNT   9
#define TPA81_PIXEL_COUNT               8          

// TPA81 constants and data byte offsets
#define TPA81_ADDRESS        0x68
#define TPA81_REG_VERSION    0x00
#define TPA81_REG_AMBIENT    0x01
#define TPA81_REG_PIXELS     0x01

// Variables for building our header
uint8_t firmware_version = 0x00;
uint8_t ambient_temperature = 0x00;

// A Packet of Pixel Data
// Ambient, Pixel_1, Pixel_2, Pixel_3, Pixel_4, Pixel_5, Pixel_6, Pixel_7, Pixel_8
uint8_t  pixel_data[9] = {0,0,0,0,0,0,0,0,0};

// A C++ String object for building output lines
String pixel_output = "";

void setup() {
   // Start-up our communication objects
   Serial.begin(9600);
   Wire.begin();
   delay(500);

   // Build our header and send it out the serial port
   firmware_version = tpa81_readByte(TPA81_ADDRESS, TPA81_REG_VERSION);
   ambient_temperature = tpa81_readByte(TPA81_ADDRESS, TPA81_REG_AMBIENT);
   String output_header = "\nTPA81 8x1 Thermal Camera Demo\nCopyright © 2017 Brig Young";
   output_header += "\nTPA81 Firmware Version:     ";
   output_header += firmware_version;
   output_header += "\nAmbient Temperature:       ";
   output_header += ambient_temperature;
   output_header += "\n\n";
   Serial.println(output_header);
}

void loop() {
  // Populate a set of pixel_data or abort
  if ( tpa81_readPixels(TPA81_ADDRESS, pixel_data) ) {
     Serial.println("ERROR: No Bytes from readPixels()");
     exit(1);
  }
  // Populate our output string with our pixel data
  for (int pindex = 0; pindex < TPA81_AMBIENT_AND_PIXEL_COUNT; pindex++) {
     pixel_output += pixel_data[pindex];
     (pindex < TPA81_PIXEL_COUNT) ? pixel_output += " " : pixel_output += "\n";
  }
  Serial.println(pixel_output);
  pixel_output.remove(0); // stoopid name for erase or clear...
  delay(TPA81_FRAME_DELAY);
}

// This function takes roughly 435 usecs to complete on Mega2560
uint8_t tpa81_readByte(uint8_t address, uint8_t data_register) {
  // Start a new session on the I2C bus
  Wire.beginTransmission(address);
  Wire.write(data_register);
  Wire.endTransmission();
  // Request a single byte
  Wire.requestFrom(address, 1);
  if (Wire.available()) {
     return Wire.read();
  }else{
     exit(0);
  }
}

// This function takes roughly 1250 usecs to complete on Mega2560
uint8_t tpa81_readPixels(uint8_t address, uint8_t * pixel_data) {
  // Start a new session on the I2C bus
  Wire.beginTransmission(address);
  Wire.write(TPA81_REG_PIXELS);
  Wire.endTransmission();
  // Request bytes from the device
  Wire.requestFrom(address, TPA81_AMBIENT_AND_PIXEL_COUNT);
  int bindex = 0;
  if (Wire.available()) {
     while(Wire.available()) {
        pixel_data[bindex] = Wire.read();
        bindex++;
     }
     return(0);
  }else{
     // ERROR: No Bytes Were Available
     return(1);
  }
}


