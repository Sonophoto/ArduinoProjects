
/* **************************************************************** *
                 _____          
   ARDUINO  /// / ___/ ///       
               / /__        
               \___/                                               

     _____               _     _          ____                   
    |__  /___  _ __ ___ | |__ (_) ___    |  _ \ ___  _ __  _   _   
      / // _ \| '_ ` _ \| '_ \| |/ _ \   | |_) / _ \| '_ \| | | |
     / /| (_) | | | | | | |_) | |  __/   |  __/ (_) | | | | |_| | 
    /____\___/|_| |_| |_|_.__/|_|\___|   |_|   \___/|_| |_|\__, |
        Like rats from a sunk ship                         |___/
      
    FILENAME: GPL_ZombiePony.ino 
     PURPOSE: "1st gen. Skid Steering API Uses 8-bit PWM to drive two poles of an H-bridge circuit powering a bi-polar motor"
        TAGS: "Arduino, Arduino.org, '2 pole', '2 wire', 'H bridge', 'remote control', zombie, robot, bi-polar, motor, 'motion control', 'no encoder', 'open loop'
      AUTHOR: "Brig Young" 
   COPYRIGHT: "2015, 2016 Brig Young, Module Master LLC" 
     LICENSE: "GPLv2, see LICENSE"         
     SPONSOR: "http://modulemaster.com/rebuilds/about-us/"
 * **************************************************************** */
 
/*
 * This code was originally created to script and demonstrate numerous
 * motion regimes for a remote control skid steering system.
 * There is some sanity checking on the inputs, like durations and 
 * throttle settings to catch typos and keep them within boundaries.
 *
 * The great thing about this library is that it immediately allows
 * you to integrate any old surplus motors that run on 12VDC 
 * (up to whatever voltage you want to use) and control them with an
 * Arduino Mega (or Uno, possibly others) This includes drill motors
 * automotive motors/solenoids, all kinds of motors.
 
 * There are detailed comments at the end of this file on what to do
 * with all of this gibberish and the hardware needed.
 * It has been released under the GNU GPLv2 as Open Source compliments
 * of Module Master LLC, http://modulemaster.com/rebuilds/about-us/
 *
 *     THIS CODE HAS NO WARRANTY AND NO SUPPORT WHATSOEVER.
 *                See LICENSE file for details...
 */
 
  
// First, lets give our pins some mnemonics. 
// 
// CHANGE PIN NUMBERS TO MATCH YOUR ELECTRONICS SETUP
// These pins MUST BE PWM pins but any 4 pwm pins should work
// avoid pins 5 and 6 on Arduino Megas, they have timing issues with PWM
// LF == LeftForward, LR == LeftReverse, RF == RightForward, RR == RightReverse
#define  LEFT_FWD_MOTOR_PIN   9 // Forward pin
#define  LEFT_REV_MOTOR_PIN  10
#define RIGHT_FWD_MOTOR_PIN  11 // Forward pin
#define RIGHT_REV_MOTOR_PIN  12 

// These pins control the Motor Enable/Disable relays, one per motor pole.
// See the detailed comments at the end of this file for more info.
//
// CHANGE PIN NUMBERS TO MATCH YOUR ELECTRONICS SETUP
// These pins should be digital pins. Our pin numbers here are from a Mega.
// You do not have to use these pin numbers, just make sure you plug your
// relay control lines into the pin numbers you name here.
#define  LEFT_FWD_LOCK_PIN  31
#define  LEFT_REV_LOCK_PIN  33
#define RIGHT_FWD_LOCK_PIN  35
#define RIGHT_REV_LOCK_PIN  37

// Turn direction mnemonics
#define        LEFT  1
#define       RIGHT  2
#define        LEAD  1
#define        DRAG  2

// motor UNLOCK status mnemonics
// We do not have a motor "lock", we have a motor "unlock" get it?
#define     ENABLED  1
#define    DISABLED  0

// Setup our Braking Mnemonics see the calibration array below.
#define       COAST  0
#define        SOFT  1
#define       LIGHT  2
#define  LOW_MEDIUM  3
#define      MEDIUM  4
#define HIGH_MEDIUM  5
#define        HARD  6
#define        LOCK  7
#define        STOP  8

// SETUP our Throttle
#define         OFF  0
#define       CREEP  1
#define       CRAWL  2
#define        TROT  3
#define  LOW_CRUISE  4
#define      CRUISE  5
#define HIGH_CRUISE  6
#define  MAX_CRUISE  7
#define        FULL  8

// This is where we keep track of the motor's status. this code only
// uses the unlock data member for now but we can add encoders,
// current sensors, and temp sensors for the motor and the H-bridge.
struct motor_T {
   int unlock;
   int left_current;        // mA
   int left_rpm;            // RPM
   int left_speed;          // cm/s
   int left_motor_temp;     // C 
   int left_driver_temp;    // C
   int right_current;
   int right_rpm;
   int right_speed;
   int right_motor_temp;
   int right_driver_temp;
} motor_state;


/* **************************************************************** *
 * M O T O R   C A L I B R A T I O N S
 *
 * These tables normalize the physical behaviour of common geared
 * bipolar motors while used as a set with one running forward and 
 * the other running in reverse to generate a straight line of travel
 * forward or backward.
 * These were created on one set of motors and need to be adjusted
 * by emperical observations to normalize behaviour at 10% (creep) 
 * 50% (lowcruise) and 100% (max-cruise) on your motors.
 *
 * If you can do the math, fit your data with a 3rd order polynomial
 * to generate your intermediate values. If not, just keep measuring
 * your motors directly to fill in the value of the arrays below.
 * There should be some more details of how-to at the end of file.
 *
 * using these arrays, our calls to motor_set_wires require only
 * percentage of throttle - no messing about with nasty pwm values.
 * REMEMBER! this is an array of 11 slots, stop (0) and ten speeds. 
 * **************************************************************** */

// L E F T   M O T O R   C A L I B R A T I O N
// This is a fair starting point for a set of motors with reduction gears
// This is the STRONG SIDE GOING FORWARD.
// guide to index        0    1    2    3    4    5    6    7    8    9   10
int  LEFT_FWD_PWM[11] = {0,  23,  46,  69,  92, 114, 137, 160, 183, 206, 228};
int  LEFT_REV_PWM[11] = {0,  26,  52,  78, 104, 128, 153, 178, 204, 229, 255};

// R I G H T   M O T O R   C A L I B R A T I O N 
// This is the STRONG SIDE GOING REVERSE.
// guide to index        0    1    2    3    4    5    6    7    8    9   10
int RIGHT_FWD_PWM[11] = {0,  26,  52,  78, 104, 128, 153, 178, 204, 229, 255};
int RIGHT_REV_PWM[11] = {0,  23,  46,  69,  92, 114, 137, 160, 183, 206, 228};

// T H R O T T L E   C A L I B R A T I O N S
// We can customize this as needed
// {OFF, CREEP, CRAWL, TROT, LOW_CRUISE, CRUISE, HIGH_CRUISE, MAX_CRUISE, FULL};
// guide to index         0   1   2   3   4   5   6   7    8
int   THROTTLE_SET[9]  = {0, 10, 20, 35, 50, 65, 75, 90, 100};

// I N D U C T I V E   B R A K I N G   C A L I B R A T I O N
// We can customize this as needed
// {COAST, SOFT, LIGHT, LOW_MEDIUM, MEDIUM, HIGH_MEDIUM, HARD, LOCK};
// guide to index        0   1   2   3    4    5    6    7
int      BRAKE_PWM[8] = {0, 10, 30, 60, 100, 160, 200, 255};


/* **************************************************************** *
 * P R I V A T E   F U N C T I O N S
 * We have set of functions we call motion primitives and you can
 * expand this list BUT! only primitive functions should call the
 * following functions that are documented as PRIVATE.
 * Don't call these directly in your scripts or as commands over
 * the serial line it WILL cause havoc.
 * **************************************************************** */

/* **************************************************************** 
 * PRIVATE!
 * ONLY call this function from a primitive function that 
 * is using motor_set_wires() directly for a caller that passed a 
 * throttle value of (0 <= throttle <= 100).
 *
 * Uses integer division to convert 0% to 100% to our 0-10 range
 * for use with motor_set_wires()
 * NOTE: This only gives you full power at a power setting of 100;
 * so 99% to 90% is only 90% power et cetera.
 */
int motor_percentage(int perc_in) {
   if (perc_in < 0) perc_in = 0;
   if (perc_in > 100) perc_in = 100;
   return (perc_in / 10);
}


/* **************************************************************** 
 * PRIVATE!
 * ONLY use this in motion primitives (like forward()).
 * To cut power to all pins by command or in a script use motor_relax(-1).
 * Adding the -1 forces the function to return immediately after 
 * motor_zero_wires() gets called in a correct context.
 * 
 * Sets all 4 motor pins to zero in 1/4 microsecond on a Mega 2560.
 */
inline void motor_zero_wires() {
   analogWrite(LEFT_FWD_MOTOR_PIN, OFF);
   analogWrite(RIGHT_FWD_MOTOR_PIN, OFF);
   analogWrite(LEFT_REV_MOTOR_PIN, OFF);
   analogWrite(RIGHT_REV_MOTOR_PIN, OFF);
}


/* **************************************************************** 
 * PRIVATE! 
 * ONLY call this function from a primitive function that 
 * is using motor_set_wires() directly. This ensures that your
 * motor lock is inductively locking your wheels.
 *
 * if passed DIASABLED it LOCKS the wheels.
 * if passed ENABLED if UNLOCKS the wheels
 */
int motor_unlock(int state) {
if (ENABLED == state) {
  digitalWrite(LEFT_FWD_LOCK_PIN, HIGH);  digitalWrite(LEFT_REV_LOCK_PIN, HIGH); 
  digitalWrite(RIGHT_FWD_LOCK_PIN, HIGH); digitalWrite(RIGHT_REV_LOCK_PIN, HIGH);
  return ENABLED;}
else {
  digitalWrite(LEFT_FWD_LOCK_PIN, LOW);  digitalWrite(LEFT_REV_LOCK_PIN, LOW); 
  digitalWrite(RIGHT_FWD_LOCK_PIN, LOW); digitalWrite(RIGHT_REV_LOCK_PIN, LOW);
  return DISABLED;}
}


/* **************************************************************** *
 * PRIVATE!
 * ONLY call this function from the Arduino setup() function.
 *
 * Put anything here that needs to run once and only once to 
 * initialize our motor system.
 * 
 * First we turn off motor control pins, and lock motor relays.
 *
 */
int motors_init() {
   motor_zero_wires();
   motor_state.unlock = motor_unlock( DISABLED );
   // EXTRA INITIALIZATIONS HERE.
   return 0;
}


/* ****************************************************************
 * PRIVATE!
 * This should only be called from a motion primitive function. 
 * This function requires the coordinated use of the other private 
 * functions for correct and safe operation.
 * 
 * Set the pwm values on the motor drive output pins and block
 * for milli_delay ms, with a max of 10 seconds. The values 
 * passed into this function are the calibration arrays indexed
 * by our 0 to 10 throttle settings. OR pass 0-255 directly in
 * LF, RF, LR, RR...
 * see forward(), reverse(), and spin() for usage of this and
 * the other private functions.
 */
int motor_set_wires(int LF, int RF, int LR, int RR, int milli_delay) {
   // We are getting values from an index so we don't sanitize.
   analogWrite(LEFT_FWD_MOTOR_PIN, LF);
   analogWrite(RIGHT_FWD_MOTOR_PIN, RF);
   analogWrite(LEFT_REV_MOTOR_PIN, LR);
   analogWrite(RIGHT_REV_MOTOR_PIN, RR);
   if (milli_delay == (-1)) 
      {return -1;} // keep running...
   if (milli_delay < 0) 
      {delay(0); motor_zero_wires(); return 0;}
   if (milli_delay > 10000) 
      {delay(10000); motor_zero_wires(); return 10000;}
   else 
      {delay(milli_delay); motor_zero_wires(); return milli_delay;}
}


/* These are the public API functions. 
 *
 * Call these functions from the loop() section of the Arduino sketch.
*/

/* ****************************************************************
 * This is a delay() with the motors OFF. It is to soft stop after
 * a motion command has completed and specifically to relax the
 * torque on a gear train for 10ms to 30ms when changing directions.
 *
 * Sets all motor lines to off for milli_delay milliseconds.
 * NOTE:
 * Use motor_relax(-1) to immediaely kill power to the motors.
 * Use the motor_brake(LOCK) function to generate maximum braking.
 */
int motor_relax(int milli_delay) {
   motor_zero_wires();
   if (milli_delay < 0) 
      {delay(0); return 0;}
   if (milli_delay > 200) 
      {delay(200); return 200;}
   else 
      {delay(milli_delay); return milli_delay;}
}


/* **************************************************************** */
int motor_forward(int throttle, int milli_duration) {
   throttle = motor_percentage(throttle);
   motor_zero_wires();
   motor_state.unlock = motor_unlock( ENABLED );
   motor_set_wires(
      LEFT_FWD_PWM[throttle], RIGHT_FWD_PWM[throttle],
      LEFT_REV_PWM[OFF],            RIGHT_REV_PWM[OFF],
      milli_duration);
   motor_state.unlock = motor_unlock( DISABLED );
   return 0;
}


/* **************************************************************** */
int motor_reverse(int throttle, int milli_duration) {
   throttle = motor_percentage(throttle);
   motor_zero_wires();
   motor_state.unlock = motor_unlock( ENABLED );
   motor_set_wires(
      LEFT_FWD_PWM[OFF],            RIGHT_FWD_PWM[OFF], 
      LEFT_REV_PWM[throttle], RIGHT_REV_PWM[throttle],
      milli_duration);
   motor_state.unlock = motor_unlock( DISABLED );
   return 0;
}


/* **************************************************************** */
// Sets the throttle and returns immediately with the motors running
// you MUST call motor_brake(...); or they will run till dead battery.
int motor_cruise(int throttle) {
   throttle = motor_percentage(throttle);
   motor_zero_wires();
   motor_state.unlock = motor_unlock( ENABLED );
   motor_set_wires(
      LEFT_FWD_PWM[throttle], RIGHT_FWD_PWM[throttle], 
      LEFT_REV_PWM[OFF],      RIGHT_REV_PWM[OFF],
      (-1));
   return throttle;
}


/* **************************************************************** */
int motor_brake(int strength, int milli_duration) {
   motor_state.unlock = motor_unlock( ENABLED );
   switch (strength) {
     case LOCK: 
        motor_set_wires(
           LEFT_FWD_PWM[BRAKE_PWM[LOCK]], RIGHT_FWD_PWM[BRAKE_PWM[LOCK]],
           LEFT_REV_PWM[BRAKE_PWM[LOCK]], RIGHT_REV_PWM[BRAKE_PWM[LOCK]],
           milli_duration);
        break;
     case COAST:
        motor_set_wires(
           LEFT_FWD_PWM[BRAKE_PWM[COAST]], RIGHT_FWD_PWM[BRAKE_PWM[COAST]],
           LEFT_REV_PWM[BRAKE_PWM[COAST]], RIGHT_REV_PWM[BRAKE_PWM[COAST]],
           milli_duration);
        break;
     case SOFT: 
        motor_set_wires(
           LEFT_FWD_PWM[BRAKE_PWM[SOFT]], RIGHT_FWD_PWM[BRAKE_PWM[SOFT]],
           LEFT_REV_PWM[BRAKE_PWM[SOFT]], RIGHT_REV_PWM[BRAKE_PWM[SOFT]],
           milli_duration);
        break;
     case LIGHT: 
        motor_set_wires(
           LEFT_FWD_PWM[BRAKE_PWM[LIGHT]], RIGHT_FWD_PWM[BRAKE_PWM[LIGHT]],
           LEFT_REV_PWM[BRAKE_PWM[LIGHT]], RIGHT_REV_PWM[BRAKE_PWM[LIGHT]],
           milli_duration);
        break;
     case LOW_MEDIUM: 
        motor_set_wires(
           LEFT_FWD_PWM[BRAKE_PWM[LOW_MEDIUM]], RIGHT_FWD_PWM[BRAKE_PWM[LOW_MEDIUM]],
           LEFT_REV_PWM[BRAKE_PWM[LOW_MEDIUM]], RIGHT_REV_PWM[BRAKE_PWM[LOW_MEDIUM]],
           milli_duration);
        break;
     case MEDIUM:
        motor_set_wires(
           LEFT_FWD_PWM[BRAKE_PWM[MEDIUM]], RIGHT_FWD_PWM[BRAKE_PWM[MEDIUM]],
           LEFT_REV_PWM[BRAKE_PWM[MEDIUM]], RIGHT_REV_PWM[BRAKE_PWM[MEDIUM]],
           milli_duration);
        break;
     case HIGH_MEDIUM:
        motor_set_wires(
           LEFT_FWD_PWM[BRAKE_PWM[HIGH_MEDIUM]], RIGHT_FWD_PWM[BRAKE_PWM[HIGH_MEDIUM]],
           LEFT_REV_PWM[BRAKE_PWM[HIGH_MEDIUM]], RIGHT_REV_PWM[BRAKE_PWM[HIGH_MEDIUM]],
           milli_duration);
        break;
     case HARD: 
        motor_set_wires(
           LEFT_FWD_PWM[BRAKE_PWM[HARD]], RIGHT_FWD_PWM[BRAKE_PWM[HARD]],
           LEFT_REV_PWM[BRAKE_PWM[HARD]], RIGHT_REV_PWM[BRAKE_PWM[HARD]],
           milli_duration);
        break;
     case STOP: // This will be a dynamic stop later, but we can call it now...
        motor_set_wires(
           LEFT_FWD_PWM[BRAKE_PWM[HARD]], RIGHT_FWD_PWM[BRAKE_PWM[HARD]],
           LEFT_REV_PWM[BRAKE_PWM[HARD]], RIGHT_REV_PWM[BRAKE_PWM[HARD]],
           milli_duration);
        break;
   }
   // This also acts as our default case so we set-up inductive lock when we exit.
   motor_set_wires(
      LEFT_FWD_PWM[OFF], RIGHT_FWD_PWM[OFF],
      LEFT_REV_PWM[OFF], RIGHT_REV_PWM[OFF],
      30);
   motor_state.unlock = motor_unlock( DISABLED );
   return 0;
}


/* **************************************************************** */
int motor_spin(int turn_direction, int throttle, int milli_delay) {
   throttle = motor_percentage(throttle);
   motor_zero_wires();
   switch (turn_direction) {
     case LEFT:
        motor_state.unlock = motor_unlock( ENABLED );
        motor_set_wires(
           LEFT_FWD_PWM[OFF],        RIGHT_FWD_PWM[throttle],
           LEFT_REV_PWM[throttle], RIGHT_REV_PWM[OFF],
           milli_delay);
        motor_relax(10);
        motor_state.unlock = motor_unlock( DISABLED );
        break;
     case RIGHT:
        motor_state.unlock = motor_unlock( ENABLED );
        motor_set_wires(
           LEFT_FWD_PWM[throttle], RIGHT_FWD_PWM[OFF],
           LEFT_REV_PWM[OFF],        RIGHT_REV_PWM[throttle],
           milli_delay);
        motor_relax(10);
        motor_state.unlock = motor_unlock( DISABLED );
        break;
     default:
        return -1;
        break;
   }
   return 0;
}


/* **************************************************************** */
int motor_skid(int turn_direction, int mode, int throttle, int milli_delay) {
   throttle = motor_percentage(throttle);
   motor_zero_wires();
   motor_state.unlock = motor_unlock( ENABLED );
   switch (turn_direction) {

     case LEFT:
        switch (mode) 
           {
           case LEAD:
              motor_set_wires(
              LEFT_FWD_PWM[OFF], RIGHT_FWD_PWM[throttle],
              LEFT_REV_PWM[OFF], RIGHT_REV_PWM[OFF],
              milli_delay);
              break;
           case DRAG:
           motor_set_wires(
              LEFT_FWD_PWM[OFF],        RIGHT_FWD_PWM[OFF],
              LEFT_REV_PWM[throttle], RIGHT_REV_PWM[OFF],
              milli_delay);
              break;
           default: // switch (mode): ERROR
              return -1;
           }     
        break; // switch (turn_direction): case LEFT

     case RIGHT:
        switch (mode) 
           {
           case LEAD:
              motor_set_wires(
              LEFT_FWD_PWM[throttle], RIGHT_FWD_PWM[OFF],
              LEFT_REV_PWM[OFF],        RIGHT_REV_PWM[OFF],
              milli_delay);
              break;
           case DRAG:
           motor_set_wires(
              LEFT_FWD_PWM[OFF], RIGHT_FWD_PWM[OFF],
              LEFT_REV_PWM[OFF], RIGHT_REV_PWM[throttle],
              milli_delay);
              break;
           default: // switch (mode): ERROR 
              return -1;
           }     
        break; // switch (turn_direction): case RIGHT

     default: // switch (turn_direction) ERROR
        return -1;
        break;
     }
   return 0;
}


/* **************************************************************** */
int motor_scan(int start_direction, int throttle, int scan_milli_width, int scan_number) {
   int reverse_direction;
   int scan_counter;
   int scan_half_width;
   // return "invalid command" (-1) if we get any zeros
   if (start_direction == 0 || throttle == 0 || scan_milli_width == 0 || scan_number == 0)
      { return -1; }
   // Setup our reverse direction
   (start_direction == LEFT) ? reverse_direction = RIGHT : reverse_direction = LEFT;
   // Sanitize our scan_milli_width
   if (scan_milli_width < 10)   { scan_milli_width = 10; }
   if (scan_milli_width > 2000) { scan_milli_width = 2000; }
   scan_half_width = (scan_milli_width / 2);
  
   // TRICKY TRICKY, watch the even number control our direction switch...
   // Do our initial turn off-heading
   motor_spin(start_direction, throttle, scan_half_width);
   
   // Cycle through our full scans doing (scan_number - 1) passes
   // we do this by setting scan_counter = 1 for our 2*(1/2 scans).
   for (scan_counter = 1; scan_counter < scan_number; scan_counter++)
      {
      // if scan_counter is even...  
      if(!(scan_counter%2)) { motor_spin( start_direction, throttle, scan_milli_width);}
      else {motor_spin(reverse_direction, throttle, scan_milli_width);}
      }
   
   // bring us back to our start heading (HALF OF A SCAN)
   if(!(scan_counter%2)) { motor_spin( start_direction, throttle, scan_half_width);}
   else {motor_spin(reverse_direction, throttle, scan_half_width);}
   return 0;
}   


/* **************************************************************** */
// System Initializations
void setup() {
   motors_init();
   // Setup for serial output on Serial Monitor in Arduino IDE
   // Serial.begin(9600);
   // while (!Serial) {;} // Spin lock for port handle
   // Now we can call Serial.functions() write strings and variables
   // OR: setup a listener to accept commands from the terminal. 
}


void loop() 
{
// This little chunk will make your motors gently lurch forward to verify 
// that things are working as intended. Use the examples below to write
// your own motion scripts and use it as an API for automating motor 
// commands.
// 
// motor_relax(3000);
// forward(10, 150);

/*
forward(20, 150);
forward(30, 150);
forward(40, 150);
forward(50, 150);
forward(60, 150);
forward(70, 150);
forward(80, 150);
forward(90, 150);
forward(100, 1500);
motor_relax(1000);

reverse(10, 150);
reverse(20, 150);
reverse(30, 150);
reverse(40, 150);
reverse(50, 150);
reverse(60, 150);
reverse(70, 150);
reverse(80, 150);
reverse(90, 150);
reverse(100, 1500);
motor_relax(1000);

spin(LEFT, 30, 300);
spin(LEFT, 40, 300);
spin(LEFT, 50, 300);
spin(LEFT, 60, 300);
spin(LEFT, 70, 300);
spin(LEFT, 80, 300);
spin(LEFT, 90, 300);
spin(LEFT, 100, 1500);
motor_relax(30);

spin(RIGHT, 100, 1500);
spin(RIGHT, 90, 300);
spin(RIGHT, 80, 300);
spin(RIGHT, 70, 300);
spin(RIGHT, 60, 300);
spin(RIGHT, 50, 300);
spin(RIGHT, 40, 300);
spin(RIGHT, 30, 300);
spin(RIGHT, 20, 300);
spin(RIGHT, 10, 300);
motor_relax(2000);

scan(LEFT, 30, 2000, 4);
motor_relax(1000);
*/
delay(10000);
}

/* ***** D E T A I L E D   I N S T R U C T I O N S ********************************

Coming Soon!

Quick take: Use this library to drive the input pins on H-Bridge circuits.
You need one H-Bridge to drive two bipolar motors
You can buy H-Bridges on Amazon or Ebay, etc. They are cheap.

                            REMEMBER!! 

    Automatic machines can cause real damage and hurt people 

                         USE INTERLOCKS!!

 * ***************************************************************************** */
 



