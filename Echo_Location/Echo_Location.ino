#define SLOPEONE 0.010777778
#define pinServo 10
#define t_total (20 * 1000)
#define trigPin 5
#define echoPin 6
#define ONE_SECOND 1000

// Initialize Value
String Manual = "Manual";
String Auto = "Auto";
String OFF = "Off";
String temp_str;

typedef struct{
  int pos, pulserate;
} MANUAL_T;

typedef struct{
  int increment, timedelay, numpulse, minangle, maxangle;
} AUTO_T;

AUTO_T autodata;
MANUAL_T manual;

// Initialize Start Mode
String Mode = "Off";

void setup() {
    pinMode(pinServo, OUTPUT);
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    Serial.begin(9600);

    // Initialize Default Manual Setting 
    manual.pos = 90; manual.pulserate = ONE_SECOND;

    // Initialize Default Auto Setting;
    autodata.increment = 5; autodata.timedelay = ONE_SECOND;
    autodata.numpulse = 1; autodata.minangle = 45; autodata.maxangle = 145;
    change_auto();
}

void loop() {
  servo_move(90);
  while(Mode == OFF){
    temp_str = Serial.readStringUntil('\n');
    if(Compare_Mode(temp_str)) Mode = temp_str;
  }

  if(Mode == Auto){
    Auto_Mode();
  } else if(Mode == Manual){
    Manual_Mode();
  }
}

// Moving Servo to correct Angle
void servo_move(int angle){
    int t_1 = (int)((0.425 + angle * SLOPEONE) * 1000);
    int t_2 = t_total - t_1;
    digitalWrite(pinServo, HIGH);
    delayMicroseconds(t_1);
    digitalWrite(pinServo, LOW);
    delayMicroseconds(t_2);
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

// Generate Outward Pulse
void pulseOutward(){
  // Clear the trigger pin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Generate a 10us pulse on the trigger pin
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  return;
}

// Echo Location
void Echo_duration(int angle){
  double distance = 0, duration;
  for(int i = 0; i < 20; i++){
    pulseOutward();
    duration = pulseIn(echoPin, HIGH);
    distance += (duration * 0.01737) / 20;
    delay(10);
  }

  Serial.print(". Distance in cm: ");
  Serial.print(distance);
  Serial.print(", at angle: ");
  Serial.println(angle);
}

// Auto Mode of Operation
void Auto_Mode(){

    while(true){
      change_auto();
      if (Mode == Manual || Mode == OFF) return; 
      servo_increment();
      servo_decrement();
    }
}

// Manual Operation Mode
void Manual_Mode(){

  while (true) {
    
    // Checking if mode has changed
    temp_str = Serial.readStringUntil('\n');
    if (Compare_Mode(temp_str)) Mode = temp_str;
    if (Mode == Auto || Mode == OFF) return;

    // Checking if Angle or Position has changed
    if (extractValue(temp_str, "Manual Angle") != -1) manual.pos = extractValue(temp_str, "Manual Angle");
    if (extractValue(temp_str, "Pulse Rate") != -1) manual.pulserate = 1000 * extractValue(temp_str, "Pulse Rate");
    
    servo_move(manual.pos);
    Echo_duration(manual.pos);
    delay(manual.pulserate);
  }
}

// Checking if any of variable have changed for Auto Mode
void change_auto(){
      // Checking if the mode has changed
      temp_str = Serial.readStringUntil('\n');
      if (Compare_Mode(temp_str)) Mode = temp_str;
      if (Mode == Manual || Mode == OFF) return; 

      // Checking if any of the variables have changed
      if (extractValue(temp_str, "Increment") != -1) autodata.increment = extractValue(temp_str, "Increment");
      if (extractValue(temp_str, "Move Delay") != -1) autodata.timedelay = ONE_SECOND * extractValue(temp_str, "Move Delay");

      if (extractValue(temp_str, "Min Angle") != -1){
        if(extractValue(temp_str, "Min Angle") > autodata.maxangle){
          Serial.print("Please make sure the min angle entered is less than or equal to the current Max Angle of ");
          Serial.println(autodata.maxangle);
        } else {
         autodata.minangle = extractValue(temp_str, "Min Angle");
        }
      }

      if (extractValue(temp_str, "Max Angle") != -1){
        if(extractValue(temp_str, "Max Angle") < autodata.minangle){
          Serial.print("Please make sure the max angle entered is greater than or equal to the current Min Angle of ");
          Serial.println(autodata.minangle);
        } else{
          autodata.maxangle = extractValue(temp_str, "Max Angle");
        }
      }
          
      if (extractValue(temp_str, "Pulse Number") != -1) autodata.numpulse = extractValue(temp_str, "Pulse Number");
  
  return;
}

// Checking for Mode
int Compare_Mode(String input){
  return (input == Manual) || (input == Auto) || (input == OFF);
}

// Extracting Value
int extractValue(String text, String StringVal){
  int angleValue = -1;
  
  // Find the position of "StringVal" in the text
  int anglePosition = text.indexOf(StringVal);
  
  // Extracting "StringVal" Value
  if (anglePosition != -1) {
    int equalsPosition = text.indexOf('=', anglePosition);
    if (equalsPosition != -1) {
      String valueString = text.substring(equalsPosition + 1);
      angleValue = valueString.toInt();
    }
  }
  
  return angleValue;
}

void Echo_amount(int angle, int pulse_num){
  for(int i = 0; i < pulse_num; i++){
    Echo_duration(angle);
    delay(autodata.timedelay);
  }
  return;
}
