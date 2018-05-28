#include <SPI.h>
#include <SD.h>


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
 * SPI SD READER
 * CS = digital 10
 * MOSI = digital 11
 * MISO = digital 12
 * SCK = digital 13
*/


//receiver channel input pins
const int CHANNEL_1 = 2; //aileron
const int CHANNEL_2 = 3; //elevator
const int CHANNEL_3 = 4; //throttle
const int CHANNEL_4 = 5; 
const int CHANNEL_5 = 9; //gps-on switch

const int GPS_ON_MICROSECONDS_VALUE = 2000;

const int SAMPLES = 5;

class Controller {
  public:
    int pwm_elevator;
    int pwm_aileron;
    int pwm_throttle;
    int pwm_gps;

  Controller() {
    pwm_elevator = 0;
    pwm_aileron = 0;
    pwm_throttle = 0;
    pwm_gps = 0; //off
  }
};


Controller REMOTE_INPUT;

int elevator_samples[SAMPLES];
int throttle_samples[SAMPLES];
int aileron_samples[SAMPLES];



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

  SD.remove("offsets.txt");
}


int count = 0;
void loop() {
  
  REMOTE_INPUT.pwm_gps = pulseIn(CHANNEL_5, HIGH);
  

  if(REMOTE_INPUT.pwm_gps > GPS_ON_MICROSECONDS_VALUE) {
    if(count == 0) {
      count++;
      
      //record 5 samples over 1 second
      for(int i = 0; i < SAMPLES; i++) {
        REMOTE_INPUT.pwm_aileron = pulseIn(CHANNEL_1, HIGH);
        REMOTE_INPUT.pwm_elevator = pulseIn(CHANNEL_2, HIGH);
        REMOTE_INPUT.pwm_throttle = pulseIn(CHANNEL_3, HIGH);
        
        elevator_samples[i] = REMOTE_INPUT.pwm_elevator;
        throttle_samples[i] = REMOTE_INPUT.pwm_throttle;
        aileron_samples[i] = REMOTE_INPUT.pwm_aileron;
        
        delay(200);
      }
  
      //average samples
      for(int i = 0; i < SAMPLES; i++) {
        elevator_samples[0] += elevator_samples[i];
        throttle_samples[0] += throttle_samples[i]; 
        aileron_samples[0] += aileron_samples[i];
      }
      elevator_samples[0] = elevator_samples[0]/SAMPLES;
      throttle_samples[0] = throttle_samples[0]/SAMPLES;
      aileron_samples[0] = aileron_samples[0]/SAMPLES;
      
      //save offsets to sd card
      File offsets = SD.open("offsets.txt", FILE_WRITE);
      if(offsets) {
        offsets.println("-----------------------------------------------------");
        offsets.println("Elevator offset: " + String(elevator_samples[0]));
        offsets.println("Throttle offset: " + String(throttle_samples[0]));
        offsets.println("Aileron offset: " + String(aileron_samples[0]));
        offsets.println("-----------------------------------------------------");
        offsets.close();
      }else {
        Serial.println("unable to open file");
      }
      
    }
    
  }else {
    count = 0;
  }
  
  
}

















