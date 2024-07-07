#define RANGE_SLOPE 0.010777778 // Calibrate Slope of the HC-SR04
#define T_TOTAL 20000 // Time division for servo
#define ONE_SECOND 1000 //For time

const int ServoPin = 10;
const int PulsePin = 5;
const int EchoPin = 6;


typedef enum{
Invalid = 0,
Manual,
Auto,
OFF
} MODE_E;


String Input_Str = "";

//Data for Manual
typedef struct{
  int pos; //Angle we want to scan in degree
  int pulse_rate; //Rate of the scan in millisecond
} MANUAL_T;

typedef struct{
  int increment; //Angle increment in degree
  int timedelay; //Time delay between each increment
  int numpulse;  //Number of Pulse per angle
  int minangle;  //Minimum Angle in degree
  int maxangle; //Maximum Angle in degree
} AUTO_T;


AUTO_T autodata;
MANUAL_T manualdata;
MODE_E Mode = OFF;

MODE_E Compare_Mode(String Input);

void setup() {
  // Initialise Pin and Serial
  pinMode(ServoPin, OUTPUT);
  pinMode(PulsePin, OUTPUT);
  pinMode(EchoPin, INPUT);
  Serial.begin(9600);

  //Initialise Manual Setting
  manualdata.pos = 90;
  manualdata.pulse_rate = ONE_SECOND;

  // Initialise Auto Setting
  autodata.increment = 5;
  autodata.timedelay = ONE_SECOND;
  autodata.numpulse = 1;
  autodata.minangle = 45;
  autodata.maxangle = 145;
  
  //Moving Servo to 90 degree
  servo_move(90);
}

void loop() {
  // put your main code here, to run repeatedly:

  
  
  while(Mode == OFF){
    Input_Str = Serial.readStringUntil('\n');
    if(Compare_Mode(Input_Str)) Mode = Compare_Mode(Input_Str);
    delay(500);
  }

  if(Mode == Auto){
    Auto_Mode();
  } else if(Mode == Manual){
    Manual_Mode();
  }

}


//Moving to specific angle in degree
void servo_move(int angle){
  int t_1= (int) ((0.425 + angle*RANGE_SLOPE) * 1000);
  int t_2 = T_TOTAL - t_1;
  digitalWrite(ServoPin, HIGH);
  delayMicroseconds(t_1);
  digitalWrite(ServoPin, LOW);
  delayMicroseconds(t_2);
}

void Auto_Mode(){

  while(true){
    if(Mode == Manual || Mode == OFF) return;
    servo_increment();
    servo_decrement();
  }
}



// Manual Operation Mode
void Manual_Mode(){

  while (true) {
    
    // Checking if mode has changed
    Input_Str = Serial.readStringUntil('\n');
    if (Compare_Mode(Input_Str)) Mode = Compare_Mode(Input_Str);
    if (Mode == Auto || Mode == OFF) return;

    // Checking if Angle or Position has changed
    if (extractValue(Input_Str, "Manual Angle") != -1){
      manualdata.pos = extractValue(Input_Str, "Manual Angle");
      return;
    }
    if (extractValue(Input_Str, "Pulse Rate") != -1) {
        manualdata.pulse_rate = 1000 * extractValue(Input_Str, "Pulse Rate");
        return;
    }
    
    servo_move(manualdata.pos);
    Echo_duration(manualdata.pos);
    delay(manualdata.pulse_rate);
  }

}



void change_auto(){

  Input_Str = Serial.readStringUntil('\n');
  if(Compare_Mode(Input_Str)) Mode = Compare_Mode(Input_Str);

  //If Mode changes from Auto exit
  if(Mode == Manual || Mode == OFF) return;

  //Checking to see if any variable hase change the variables have changed
      if (extractValue(Input_Str, "Increment") != -1) {
        autodata.increment = extractValue(Input_Str, "Increment");
        return;
      }

      if (extractValue(Input_Str, "Move Delay") != -1) {
        autodata.timedelay = ONE_SECOND * extractValue(Input_Str, "Move Delay");
        return;
      }

      if (extractValue(Input_Str, "Min Angle") != -1){
        if(extractValue(Input_Str, "Min Angle") > autodata.maxangle){
          Serial.print("Please make sure the min angle entered is less than or equal to the current Max Angle of ");
          Serial.println(autodata.maxangle);
        } else {
         autodata.minangle = extractValue(Input_Str, "Min Angle");
        }
        return;
      }

      if (extractValue(Input_Str, "Max Angle") != -1){
        if(extractValue(Input_Str, "Max Angle") < autodata.minangle){
          Serial.print("Please make sure the max angle entered is greater than or equal to the current Min Angle of ");
          Serial.println(autodata.minangle);
        } else{
          autodata.maxangle = extractValue(Input_Str, "Max Angle");
        }
        return;
      }
          
      if (extractValue(Input_Str, "Pulse Number") != -1) {
        autodata.numpulse = extractValue(Input_Str, "Pulse Number");
        return;
      }
  return;
}

// Extracting Value
int extractValue(String text, String StringVal){
  int ans = -1;
  
  // Find the position of "StringVal" in the text
  int anglePosition = text.indexOf(StringVal);
  
  // Extracting "StringVal" Value
  if (anglePosition != -1) {
    int equalsPosition = text.indexOf('=', anglePosition);
    if (equalsPosition != -1) {
      String valueString = text.substring(equalsPosition + 1);
      ans = valueString.toInt();
    }
  }
  
  return ans;
}

MODE_E Compare_Mode(String Input) {
    Input.toLowerCase();
    
    if (Input == "manual") return Manual;
    if (Input == "auto") return Auto;
    if (Input == "off") return OFF;
    return Invalid;
}


// Sweeping Increment at auto mode
void servo_increment(){
  for(int angle = autodata.minangle; angle < autodata.maxangle; angle += autodata.increment){
    angle = min(angle, autodata.maxangle);
    change_auto();
    if (Mode == Manual || Mode == OFF) return;
    servo_move(angle);
    delay(1); 
    Echo_amount(angle, autodata.numpulse);
    delay(autodata.timedelay);
  }
  return;
}

// Sweeping decrement at auto mode
void servo_decrement(){
  for(int angle = autodata.maxangle; angle > autodata.minangle ; angle -= autodata.increment){
    angle = max(angle, autodata.minangle);
    change_auto();
    if (Mode == Manual || Mode == OFF) return; 
    servo_move(angle);
    delay(1);
    Echo_amount(angle, autodata.numpulse);
    delay(autodata.timedelay);
  }
  return;
}


void Echo_amount(int angle, int pulse_num){
  for(int i = 0; i < pulse_num; i++){
    Echo_duration(angle);
    delay(autodata.timedelay);
  }
  return;
}

// Echo Location
void Echo_duration(int angle){
  double distance = 0, duration;
  for(int i = 0; i < 20; i++){
    pulseOutward();
    duration = pulseIn(EchoPin, HIGH);
    distance += (duration * 0.01737) / 20;
    delay(10);
  }

  Serial.print(". Distance in cm: ");
  Serial.print(distance);
  Serial.print(", at angle: ");
  Serial.println(angle);
}

// Generate Outward Pulse
void pulseOutward(){
  // Clear the trigger pin
  digitalWrite(PulsePin, LOW);
  delayMicroseconds(2);

  // Generate a 10us pulse on the trigger pin
  digitalWrite(PulsePin, HIGH);
  delayMicroseconds(10);
  digitalWrite(PulsePin, LOW);
  return;
}
