#include "Servo.h"
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_BMP085.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h>


/*********************************************
 * ELECTRONICS SETUP
 *********************************************
 * 
 * RECEIVER
 * channel1 = digital 2
 * channel2 = digital 3
 * channel3 = digital 4
 * channel4 = digital 5
 * channel5 = digital 9
 * 
 * ACTUATORS
 * esc = analog 1
 * ailerion servo = digital 8
 * elevator servo = digital 7
 * 
 * I2C DEVICES
 * SCL = analog 5
 * SDA = analog 4
 * 
 * SPI SD READER
 * CS = digital 10
 * MOSI = digital 11
 * MISO = digital 12
 * SCK = digital 13
*/


/*********************************************
CONSTANTS
*********************************************/

//receiver channel input pins
const int CHANNEL_1 = 2; //aileron
const int CHANNEL_2 = 3; //elevator
const int CHANNEL_3 = 4; //throttle
const int CHANNEL_4 = 5; 
const int CHANNEL_5 = 9; //gps-on switch

//output pins (to plane)
const int AILERON_SERVO_PIN = 8;
const int ELEVATOR_SERVO_PIN = 7;
//throttle is A1

const int GPS_ON_MICROSECONDS_VALUE = 2000;



/*********************************************
MODELS
*********************************************/

class Coordinates {
  public:
    char lng[20];
    char lat[20];

    Coordinates() {}
    
    Coordinates(char* la, char* ln) {
      strcpy(lat, la);
      strcpy(lng, ln);
    }
};

class Waypoint {
  public:
    Coordinates location;
    int ft_altitude;
    bool drop_on_arrival;

    Waypoint() {}

    Waypoint(Coordinates loc, int alt, bool drop) {
      location = loc;
      ft_altitude = alt;
      drop_on_arrival = drop;
    }

    Waypoint& operator=(Waypoint const& obj) {
      location = obj.location;
      ft_altitude = obj.ft_altitude;
      drop_on_arrival = obj.drop_on_arrival;
      return *this;
    }
};

class Controller {
  public:
    int pwm_elevator;
    int pwm_aileron;
    int pwm_throttle;
    int pwm_gps;

  Controller() {
    pwm_elevator = ELEVATOR_SERVO_MICROSECONDS_ZERO;
    pwm_aileron = AILERON_SERVO_MICROSECONDS_ZERO;
    pwm_throttle = THROTTLE_ESC_MICROSECONDS_ZERO;
    pwm_gps = 0; //off
  }
};


/*********************************************
VARIABLES
*********************************************/

//Sensors
Adafruit_BMP085 bmp;

//Actuators
Servo esc;
Servo elevator;
Servo aileron;

//Flight
int ALTITUDE; //define once in setup
int AILERON_TARGET = AILERON_SERVO_MICROSECONDS_ZERO;
Controller REMOTE_INPUT;
Waypoint CURRENT_WAYPOINT;
int WP_INDEX;
int WP_LENGTH;
bool REPEAT_FLIGHT;
bool RETURN_HOME;

/*********************************************
FUNCTIONS
*********************************************/

//converts null-terminated charArray to integer
int charArrayToInt(char* charArr) {
  String string = String(charArr);
  return int(string.toInt());
}

//gets line of text from open file object
void getNextFileLine(File file, char* string) {
  int line_len = 0;
  for(int i = 0; file.available(); i++) {
    if(char(file.peek()) == '\n') {
      file.seek(file.position() + 1);
      break;
    }
    line_len++;
    string[i] = char(file.read());
  }
  
  string[line_len] = '\0'; //null terminate
}

//returns a populated Waypoint object from a flight file and index
Waypoint loadWaypoint(int index) {
    File flight = SD.open("flight.txt");
    flight.seek(0); //reset file position
    char string[20];
    getNextFileLine(flight, string); getNextFileLine(flight, string); getNextFileLine(flight, string); //remove flight settings and newline

    for(int i = 0; i < index * 5; i++) getNextFileLine(flight, string); //each waypoint is five lines

    getNextFileLine(flight, string);
    bool drop = charArrayToInt(string);

    getNextFileLine(flight, string);
    int ft_altitude = charArrayToInt(string);
   
    getNextFileLine(flight, string);
    Coordinates coords(string, new char);
    getNextFileLine(flight, string);
    strcpy(coords.lng, string);
    flight.close();
    
    Waypoint wp_out(coords, ft_altitude, drop);
    
    return wp_out;
}

//truncates value in both directions by factor
int symetricCap(int value, int zero, double factor) {
    double cap_size = zero/factor;
    if(value > (zero + cap_size)) value = int(zero + cap_size);
    if(value < (zero - cap_size)) value = int(zero - cap_size);
    return value;
}

//calculates the degrees-from-north heading between two Coordinates objects
double getTargetHeading(Coordinates c1, Coordinates c2) {
  
}

//reads current degrees-from-north heading from magnetometer
double getHeading() {
  
}

//returns the ft difference in current altitude and the ALTITUDE global varible
double getAltitude(Adafruit_BMP085 b_sensor) {
  double ft_altitude = (b_sensor.readAltitude() - ALTITUDE)*3.28084; //get difference and convert from meters to feet
  return ft_altitude;
}

//returns current coordinates
Coordinates getCurrentCoordinates() {
  
}


//determines if waypoint has been reached
bool hasArrived(Waypoint waypoint) {
  return false;
}

//drops plane payload
void dropPayload() {
  
}


void setup() {
  Serial.begin(9600);

  //Initialize SD card
  if (!SD.begin(10)) {
    Serial.println("SD card initialization failed!");
    while(1) {}
  }
  pinMode(10, OUTPUT); //cs must be output

  //Controller inputs
  pinMode(CHANNEL_1, INPUT);
  pinMode(CHANNEL_2, INPUT);
  pinMode(CHANNEL_3, INPUT);
  pinMode(CHANNEL_4, INPUT);
  pinMode(CHANNEL_5, INPUT);

  //Actuators
  esc.attach(A1);
  aileron.attach(AILERON_SERVO_PIN);
  elevator.attach(ELEVATOR_SERVO_PIN);

  //Sensors
  if(!bmp.begin()) {
   Serial.println("Unable to find BMP180 device.");
    while (1) {}
  }
  //set global altitude zero-variable
  ALTITUDE = bmp.readAltitude();

  //Load flight file
  File FLIGHT = SD.open("flight.txt");
  char string[20]; 
  
  getNextFileLine(FLIGHT, string);
  REPEAT_FLIGHT = bool(charArrayToInt(string));
  getNextFileLine(FLIGHT, string);
  RETURN_HOME = charArrayToInt(string);

  int wpt_count = 0;
  while(FLIGHT.available()) {
    getNextFileLine(FLIGHT, string);
    if(string[0] == '\0') wpt_count++;
  }
  FLIGHT.close();

  WP_LENGTH = wpt_count;
  WP_INDEX = 0;

  CURRENT_WAYPOINT = loadWaypoint(WP_INDEX);

  SD.remove("datalog.txt");
}

double feet_target = 0;
int count = 0;

void loop() {
  
//  REMOTE_INPUT.pwm_aileron = pulseIn(CHANNEL_1, HIGH);
//  REMOTE_INPUT.pwm_elevator = pulseIn(CHANNEL_2, HIGH);
//  REMOTE_INPUT.pwm_throttle = pulseIn(CHANNEL_3, HIGH);
  REMOTE_INPUT.pwm_gps = pulseIn(CHANNEL_5, HIGH);
  
//  Serial.println("AUTOPILOT: " + String(REMOTE_INPUT.pwm_gps) + ", " + "AILERON: " + String(REMOTE_INPUT.pwm_aileron) + ", " + "ELEVATOR: " + String(REMOTE_INPUT.pwm_elevator) + ", " + "THROTTLE: " + String(REMOTE_INPUT.pwm_throttle) + ", ");

  if(REMOTE_INPUT.pwm_gps > GPS_ON_MICROSECONDS_VALUE) {
    if(count == 0) feet_target = 100;
    
    //CHECK WAYPOINT ARRIVAL
    if(hasArrived(CURRENT_WAYPOINT)) {
      if(CURRENT_WAYPOINT.drop_on_arrival) {
        dropPayload();
      }
      if(WP_INDEX == WP_LENGTH - 1) { //is last waypoint
        if(RETURN_HOME || REPEAT_FLIGHT) WP_INDEX = -1;
        else WP_INDEX--;
      }
      WP_INDEX++; //either 0 or last waypoint when final wp is reached
      CURRENT_WAYPOINT = loadWaypoint(WP_INDEX);
    }

    //control
    REMOTE_INPUT.pwm_aileron = pulseIn(CHANNEL_1, HIGH);
//    REMOTE_INPUT.pwm_elevator = ELEVATOR_SERVO_MICROSECONDS_ZERO;
    REMOTE_INPUT.pwm_throttle = pulseIn(CHANNEL_3, HIGH);
    
    //HEADING CORRECTION

  
    //ELEVATOR CORRECTION
    double alt = getAltitude(bmp);
    double alt_error = (feet_target - alt);
    double out = ELEVATOR_SERVO_MICROSECONDS_ZERO + (alt_error*P_HEIGHT);
    out = symetricCap(out, ELEVATOR_SERVO_MICROSECONDS_ZERO, 10);
    REMOTE_INPUT.pwm_elevator = int(out);

    //optional throttle controller
    double t_out = THROTTLE_ESC_MICROSECONDS_ZERO + (alt_error*P_HEIGHT_T);
    t_out = symetricCap(t_out, THROTTLE_ESC_MICROSECONDS_ZERO, 9);
    REMOTE_INPUT.pwm_throttle = int(t_out);

    File data_log = SD.open("datalog.txt", FILE_WRITE);
    //data_log to sd card
    if(data_log) {
      data_log.println("-----------------------------------------------------");
      data_log.println("Altitude: " + String(alt));
      data_log.println("Correction: " + String(alt_error*P_HEIGHT) + "  +  " +  ELEVATOR_SERVO_MICROSECONDS_ZERO + " = " + String(REMOTE_INPUT.pwm_elevator));
      data_log.println("Throttle Correction: " + String(alt_error*P_HEIGHT_T) + "  +  " +  THROTTLE_ESC_MICROSECONDS_ZERO + " = " + String(REMOTE_INPUT.pwm_throttle));
      data_log.println("-----------------------------------------------------");
      data_log.close();
    }else {
      Serial.println("unable to open data_log");
    }

    count++;
    //AILERON CORRECTION
    
  } else {
    count = 0;
      //READ HUMAN CONTROLLER INPUT
      REMOTE_INPUT.pwm_elevator = pulseIn(CHANNEL_2, HIGH);
      REMOTE_INPUT.pwm_aileron = pulseIn(CHANNEL_1, HIGH);
      REMOTE_INPUT.pwm_throttle = pulseIn(CHANNEL_3, HIGH);
  }

  
  //APPLY VALUES
  aileron.writeMicroseconds(REMOTE_INPUT.pwm_aileron);
  elevator.writeMicroseconds(REMOTE_INPUT.pwm_elevator);
  esc.writeMicroseconds(REMOTE_INPUT.pwm_throttle);
  
  
}

















