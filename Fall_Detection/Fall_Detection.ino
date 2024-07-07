#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <ArduinoBLE.h>
#include <math.h>
#include <WiFiNINA.h>


//Setting up Value
#define RAD2DEG 57.2958
#define NUM_STEP 2500
#define ONE_G 9.81
#define PRECISION 8
#define MAX_DATA 3
#define DELTAT 50
#define MINANGLE -90
#define MAXANGLE 90
#define DELTAG 0.1
#define DELTAANGVEL 60
#define THRESHOLD 5000
#define ONESEC 1000
#define THRESHOLD_DURATION 5000  

#define X 0
#define Y 1
#define Z 2

#define FALLMODE 0
#define COLLECTMODE 1
#define CALIBRATIONMODE 2



int Mode = 0; 


//Start Library File
Adafruit_MPU6050 mpu;
WiFiClient client;
File myFile;

//Bluetooth ID
BLEService matlabService("EF680201-9B35-4933-9B10-52FFA9740042"); // create service
BLEStringCharacteristic matlabCharacteristic("EF680202-9B35-4933-9B10-52FFA9740042"
                                            , BLERead | BLEWrite, 125);

char   HOST_NAME[] = "maker.ifttt.com";
String PATH_NAME   = "/trigger/Fall_Happened/json/with/key/dZFmPGzvq6Wy7bpxNUCb3g";
String queryString = "?value1=57&value2=25";



int burn = MAX_DATA+15;

//Setting up struct
typedef struct{
  float yval[MAX_DATA];
  float uval[MAX_DATA];
} DATA_T;

typedef struct{
  float Accel, AngVal;
  float CurrAng[3];
} PROCESS_T;

//Setting data for signal output
PROCESS_T CurrVal[DELTAT];
PROCESS_T MinMaxVal[2];

DATA_T accel_data[MAX_DATA];
DATA_T gyro_data[MAX_DATA];
float CurrValaccel[MAX_DATA];
float CurrValgyro[MAX_DATA];

//Start Value
float accel_offset[] = {0.17410634,-0.05022169,-0.21999046};
float gyro_offset[] = {0.01678776,0.01150871,0.01318305};



int previousMillis = 0;
int fall = 0;
int cancel = 0;
volatile unsigned long buttonPressTimestamp = 0;
volatile bool isTriggering = false; 
volatile bool buttonIsPressed = false;
const int buttonPin = 3;          // Button to check for the 5-second press
const int ledPin = 4;
const int modePin = 5;
const int modeLEDpin = 8;
const int fallLEDpin = 7;
int current_state = HIGH; // Initialize the current state as HIGH (unpressed)
int previous_state = HIGH; // Initialize the previous state as HIGH


void SetUpMPU();
void StartUpBLE();
float MagComp(float v1, float v2, float v3);
float Aslope(float v1, float v2);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(1000);
  Serial.println("Starting Up");
  

  pinMode(modeLEDpin,OUTPUT);
  pinMode(fallLEDpin,OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(modePin, INPUT_PULLUP);
  pinMode(buttonPin, INPUT_PULLUP);  
  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonISR, CHANGE);
  SetUpMPU();
  Mode_Change();


}

void loop() {
  // put your main code here, to run repeatedly:
  ModeCheck();

  if(Mode == FALLMODE){
    FALLDETECTMODE();
  }

  if(Mode == COLLECTMODE){
    BLEReadingLoop();
  }

  cancel = DistSOS();
  if(cancel){
    cancel = 0;
    STOPSOS();
  }


}

void ModeCheck(){
  current_state = digitalRead(modePin);

  if (current_state == LOW && previous_state == HIGH) {
    // Button was just pressed (edge detection)
    delay(50); // Debounce by waiting for 50ms

    if (digitalRead(modePin) == LOW) {
      // Button is still LOW, indicating a valid press
      Disconnect();
      Mode = (Mode + 1) % 2; // Change modes
      Serial.print("Mode: ");
      Serial.println(Mode);
      Mode_Change();
    }
  }

  previous_state = current_state;
}

void Disconnect(){
  if(Mode == FALLMODE){
    client.stop();
    Serial.println();
    Serial.println("Wifi disconnected");
  }

  if(Mode == COLLECTMODE){
    BLE.end();
    Serial.println();
    Serial.println("Bluetooth disconnected");
  }
}

void Mode_Change(){
  burn = MAX_DATA+15;
  digitalWrite(modeLEDpin, Mode);
  if(Mode == FALLMODE){
    FallModeStartup();
  }

  if(Mode == COLLECTMODE){
    DataModStartup();
  }
}

void FallModeStartup(){
  Serial.println("Starting up in Fall Dection Mode");
  delay(1000);
}

void DataModStartup(){
  StartUpBLE();
  Serial.println("Starting up in Data Collection Mode");
  delay(1000);
}


//Comeback Here
void FALLDETECTMODE(){
  
  ManageData();
  fall = Fallen(MinMaxVal);
  if(fall){
    digitalWrite(fallLEDpin, fall);
    Serial.println("Fall has Happened");
    int currentMillis = millis();
    int cancel = 0;

    while(millis() - currentMillis < THRESHOLD){
      Serial.println(millis() - currentMillis);
        cancel = DistSOS();
        if(cancel){
        Serial.println("SOS has been canceled");
        break;
    }
      ManageData();
    }
          
    if(!cancel){
      SendHelp();
      startTrigger();
      fall = 0;
    } else{
      STOPSOS();
    }
    delay(10*ONESEC);
  }
  return;
}

void STOPSOS(){
  stopTrigger();
  digitalWrite(fallLEDpin, 0);
}

//Getting Data From Sensor and Filtering it
void getData(){
  
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  CurrValaccel[X] = a.acceleration.x;
  CurrValaccel[Y] = a.acceleration.y;
  CurrValaccel[Z] = a.acceleration.z;

  CurrValgyro[X] = g.gyro.x;
  CurrValgyro[Y] = g.gyro.y;
  CurrValgyro[Z] = g.gyro.z;

  butter_filter(accel_data, CurrValaccel);
  butter_filter(gyro_data, CurrValgyro);
  
}


//Use the clean data to calculate magnitute of acceleration and angle. Also compute anagle inclination value.
void ManageData(){
  
  while(burn > 0){
    getData();
    delay(50);
    burn--;
    ShiftData(CurrVal);
    CurrVal[0].Accel =  MagComp((accel_data[X].yval[0] - accel_offset[X]) / ONE_G,(accel_data[Y].yval[0] - accel_offset[Y]) / ONE_G,
                              (-accel_data[Z].yval[0] - accel_offset[Z]) / ONE_G);
    CurrVal[0].AngVal = MagComp((gyro_data[X].yval[0] - gyro_offset[X]) * RAD2DEG,(gyro_data[Y].yval[0] - gyro_offset[Y]) * RAD2DEG,
                              (gyro_data[Z].yval[0] - gyro_offset[Z]) * RAD2DEG);

    for(int i = 0; i < 3; i++){
      CurrVal[0].CurrAng[i] = Aslope((accel_data[i].yval[0] - accel_offset[i]) / ONE_G,CurrVal[0].Accel);
    }
  }

  getData();

  //Shift Data
  ShiftData(CurrVal);
  CurrVal[0].Accel =  MagComp((accel_data[X].yval[0] - accel_offset[X]) / ONE_G,(accel_data[Y].yval[0] - accel_offset[Y]) / ONE_G,
                              (-accel_data[Z].yval[0] - accel_offset[Z]) / ONE_G);
  CurrVal[0].AngVal = MagComp((gyro_data[X].yval[0] - gyro_offset[X]) * RAD2DEG,(gyro_data[Y].yval[0] - gyro_offset[Y]) * RAD2DEG,
                              (gyro_data[Z].yval[0] - gyro_offset[Z]) * RAD2DEG);

  for(int i = 0; i < 3; i++){
    CurrVal[0].CurrAng[i] = Aslope((accel_data[i].yval[0] - accel_offset[i]) / ONE_G,CurrVal[0].Accel);
  }
  
  findMinMax(CurrVal,MinMaxVal);
  if(fall == 0){
    Serial.print("Acceleration: "); Serial.print(CurrVal[0].Accel);
    Serial.print(", Angle Velocity: "); Serial.println(CurrVal[0].AngVal);
    Serial.println("");
  }

  delay(50);

  return;

}

void ShiftData(PROCESS_T sys[DELTAT]){

  for(int i = 1; i < DELTAT; i++){
    sys[i].Accel = sys[i-1].Accel;
    sys[i].AngVal = sys[i-1].AngVal;

    for(int j = 0; j < 3; j++){
      sys[i].CurrAng[j] = sys[i-1].CurrAng[j];
    }
  }
  return;
}


void findMinMax(PROCESS_T sys[DELTAT], PROCESS_T sys2[2]){
    
  for(int i = 0; i<2; i++){
    sys2[i].Accel = sys[0].Accel;
    sys2[i].AngVal = sys[0].AngVal;
    for(int j = 0; j<3; j++){
      sys2[i].CurrAng[j] = sys[0].CurrAng[j];
    }
  }
  //Finding Min Value
  for(int i = 1; i<DELTAT;i++){

    sys2[0].Accel = min(sys2[0].Accel,sys[i].Accel);
    sys2[0].AngVal = min(sys2[0].AngVal,sys[i].AngVal);

    sys2[1].Accel = max(sys2[1].Accel,sys[i].Accel);
    sys2[1].AngVal = max(sys2[1].AngVal,sys[i].AngVal);

    for(int j=0; j<3;j++){
      sys2[0].CurrAng[j] = min(sys2[0].CurrAng[j],sys[i].CurrAng[j]);
      sys2[1].CurrAng[j] = max(sys2[1].CurrAng[j],sys[i].CurrAng[j]);
    }
  }


  return;
}

int Fallen(PROCESS_T sys[2]){
  if(fabs(sys[1].Accel-sys[0].Accel) < DELTAG){
    return 0;
  }

  //if(fabs(sys[1].AngVal-sys[0].AngVal) < DELTAANGVEL){
  //return 0;
  //}
  return 1;
}















