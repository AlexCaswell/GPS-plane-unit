/*
 * Electronics Setup
 * A0-A3 : X-axis steppers
 * D2-D5 : Z-axis steppers
 * D10-D13 : SD reader (CS, MOSI, MISO, SCK)
 * D6 : Line button
 * A5 : Tool servo
 * D7 : Thermistor
 * D8 : Hotend MOSFET
 * D9 : End stop
 */

 /*
  * SD input format
  * The intput file should be named input.txt.
  * 
  * input.txt contains the consecutive points for the hotend to travel and the velocity for each coresponding distance, as well as wait-for-next-cut and set-temperature commands:
  * Example of point/speed command (units are mm and mm/s):
  * <x-coordinate>:<y-coordinate>:<speed>\n
  * Example of wait-for-next-cut command:
  * w\n
  * Example of set-temperature command:
  * t:<celsius temperature>\ n
  */


#include <CheapStepper.h>
#include <SD.h>
#include <SPI.h>
#include <Bounce2.h>
#include <Servo.h>
#include <math.h>

//FUNCTIONS

String readNextFileLine(File file) {
  char fileLine[50];
  int i = 0;
  if(file.available()) {
    for(; file.available(); i++) {
      fileLine[i] = char(file.read());
      if(char(file.peek()) == '\n') {
        fileLine[++i] = char(file.read());
        break;
      }
    }
    fileLine[++i] = '\0';
  }else {
    String empty = String('\0');
    return empty;
  }
  String out = String(fileLine);
  return out;
}

int mmPerSecToRpm(int mmPerSec) {
   
}


double calcPID(double current, double *previousMeasurements, int numPreviousMeasurements, double target, double kp, double ki, double kd) {
  
}

//GLOBAL VARIABLES
int kp;
int ki;
int kd;
File input;
Servo toolServo;
Bounce lineButton = Bounce();
Bounce endStop = Bounce();
CheapStepper xAxis(A0, A1, A2, A3);
CheapStepper zAxis(2, 3, 4, 5);

int current_move_delay = 0;
int move_started = 0;
double x = 0;
double y = 0;
int target_temperature = 0;
bool wait = false;




void setup() {

Serial.begin(9600);

xAxis.set4076StepMode();
zAxis.set4076StepMode();

toolServo.attach(A5);

pinMode(9, INPUT_PULLUP);
pinMode(7, INPUT);
pinMode(8, OUTPUT);

lineButton.attach(6, INPUT_PULLUP);
lineButton.interval(10);
endStop.attach(9);
endStop.interval(10);

if(!SD.begin(10)) {
  Serial.println("SD card initialization failure"); while(1);
}
input = SD.open("input.txt", FILE_READ);

}

void loop() {


  lineButton.update();
  endStop.update();

  if(lineButton.fell()) {
    wait = false;
  }
  
  if(!wait && (millis() - move_started) > current_move_delay) {
    String fileLine = readNextFileLine(input);
    if(fileLine[0] == 'w') wait = true;
    else if(fileLine[0] == 't') {
      String temperature = fileLine.substring(2, fileLine.length() - 1);
      target_temperature = temperature.toInt();
      Serial.println(target_temperature);
    }
    else if(fileLine[0] != '\0') {
      //parse command
      int ind1 = fileLine.indexOf(':');
      int ind2 = fileLine.indexOf(':', (ind1 + 1));
      
      double x2 = fileLine.substring(0, ind1).toFloat();
      double y2 = fileLine.substring(ind1 + 1, ind2).toFloat();
      double speed2 = fileLine.substring((ind2 + 1), (fileLine.length() - 1)).toFloat();
     
      //calculate distance between current and target(just parsed) coordinates in millimeters
      double x_dist = x2 - x;
      double y_dist = y2 - y;
      double mm_distance = pow((square(x_dist) + square(y_dist)), 0.5);
      //calculate the duration of move in seconds
      double sec_duration = mm_distance/speed2;
      //calculate rpm
      // 1 revolution = 4096 steps = 60 mm
      // because there are 60mm per revolution and 60 seconds per minute the conversion factor between rpm and mm/s is 1;
      double x_speed = (x_dist/sec_duration);
      double y_speed = (y_dist/sec_duration);
      Serial.print("Speed: "); Serial.print(speed2); Serial.print(", Duration: "); Serial.print(sec_duration); Serial.print(", x-y-speed: (");
      Serial.print(x_speed); Serial.print(", "); Serial.print(y_speed); Serial.print(") points: ("); Serial.print(x); Serial.print(", ");
      Serial.print(y); Serial.print("), ("); Serial.print(x2); Serial.print(", "); Serial.print(y2); Serial.print(")"); Serial.println();
      //update coordinates
      x = x2;
      y = y2;
      //set steppers
      xAxis.setRpm(abs(x_speed));
      zAxis.setRpm(abs(y_speed));
      //steps per mm = 68.2
      xAxis.newMove(((x_speed < 0) ? true : false), abs(x_dist*68.2));
      zAxis.newMove(((y_speed < 0) ? true : false), abs(y_dist*68.2));
      //set duration
      move_started = millis();
      current_move_delay = sec_duration * 1000;
      
    }
    else {
      wait = true;
      input.seek(0);
      y = 0;
      x = 0;
      Serial.println("end");
      xAxis.stop();
      zAxis.stop();
    }
  }

  xAxis.run();
  zAxis.run();
  
  
  
}
















  
