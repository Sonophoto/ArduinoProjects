/* FILENAME: Test_I2C_TPA81.ino
 *   AUTHOR: Brig Young, brig sonophotostudios come-on
 *  PURPOSE: Attempts to read a TPA81 Termal Camera at all known addresses 
 *  and outputs all ten bytes of data from each device it finds. This can
 *  be used for address discovery or functional verification of devices.
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

// We have translated the manufacturers documented addresses to conventional
// I2C address numbers. The manufacturers documentation is incorrect because
// their addresses are left shifted to their position on the wire side of the
// I2C protocol. It is correct that these new addresses increment by 1 not 2.
// 
// The Wire library and others expect a right aligned 7bit address as follows:
byte address_list[] = {0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F};

byte data_buffer[] = {0,0,0,0,0,0,0,0,0,0};

byte tpa81_address;

void setup() {

Serial.begin(9600);
Wire.begin();
delay(1000);
}

int readTPA81(byte twi_address) {
   // Start a new master session on the bus
   Wire.beginTransmission((twi_address));
   Wire.write(0x00);
   Wire.endTransmission();
   // Request bytes from the device
   Wire.requestFrom((twi_address), 10);
   delay(40); // (1000ms / TPA81 Frame Rate) = 40ms
   Serial.print("Number of bytes returned: ");
   Serial.println(Wire.available());
   if (Wire.available()) {
       // Read byte values into our data buffer
      int bindex = 0;
      while(Wire.available()) {
         data_buffer[bindex] = Wire.read();
         bindex++; 
      }
   }   
}

void loop() {
   // Step thru each address
   for (int tindex = 0; tindex < 8; tindex++) {
      tpa81_address = address_list[tindex];
      Serial.print("Taking a reading from the TPA81 at address: ");
      Serial.println(tpa81_address);
      // Read the device at our current address
      (void) readTPA81(tpa81_address);
      // Print out any data payload for this address
      for (int pindex = 0; pindex < 10; pindex++) {
         Serial.print(data_buffer[pindex]);
         // Add whitespace or linefeed as appropriate
         (pindex < 9) ? Serial.print(" "): Serial.println("");
      }
      // Zero our data_buffer values
      for (int bindex = 0; bindex < 10; bindex++) {
         data_buffer[bindex] = 0x00;
      }
   delay(1000);
   }
   exit(0);
}   

