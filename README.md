## ArduinoProjects
A collection of original, GPL and BSD(2c) Licensed, Arduino projects that may be of interest for robotics and sensors projects

I only really use Arduino Mega 2560s because they have interrupts and hardware serial. Because of this you may need a Mega to run some of these projects. It may be possible to adapt them to Unos and others but that is left as an exercise as they say. Also note that some sensors may use an abridged interface and do not give complete access to all of the sensors features. I will try to point out any libraries that implement all chip/sensor features in these cases.

Each directory is a stand alone project or library. Some may really be a gist with a single function or interrupt service routine (ISR) Documentation is ALWAYS in the source code so read the comments ;-)

### WORKING:
#### Zombie Pony:
Giant pile of functions for running motors connected to an H-Bridge circuit.
#### Test_I2C_TPA81:
Scans for TPA81 thermal cameras on I2C bus and returns 10 data bytes from each device found.
#### Demo_TPA81:
Outputs a header with firmware version and ambient temperature then outputs a set of temp data every ~50ms
### STUB ONLY:
#### Weathino:
This is just a stub for a simple Arduino "weather station" fine place to start yours ;-) Note: I don't claim the name "Weathino" its fair game AFAIK. If you are interested in weather codes you might like this python script that looks up weather forecasts by locating your IP address: https://github.com/Sonophoto/PythonNotes/tree/master/weatherLookup

Some projects may require you to install other (3rd party) libraries before they will compile and run.

These are living projects, I will accept pulls that fit the stated purrpose, fix any bugs if they get reported, and be thrilled to have you fork this repo and use it to create something entirely different.

Attributions, Acknowledgements and Citations, as appropriate, are greatly appreciated.
