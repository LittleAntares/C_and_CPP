
//Digital Filter at with sample rate of 1000 Hz and cut off frequency of 50 Hz
void butter_filter(DATA_T sys[MAX_DATA], float data[MAX_DATA]) {
  for (int row = 0; row < MAX_DATA; row++) {
    for (int index = MAX_DATA - 1; index > 0; index--) {
      sys[row].yval[index] = sys[row].yval[index - 1];
      sys[row].uval[index] = sys[row].uval[index - 1];
    }
    sys[row].uval[0] = data[row];
    sys[row].yval[0] = TF_BF(sys[row]);
  }
}

//Butterworth Transfer function
float TF_BF(DATA_T sys) {
  float ans = 1.5610 * sys.yval[1] - 0.6414 * sys.yval[2] 
              + 0.0201 * sys.uval[0] + 0.0402 * sys.uval[1] 
              + 0.0201 * sys.uval[2];
  return ans;
}

//Magnitute Function
float MagComp(float v1, float v2, float v3){
  return sqrt(v1*v1+v2*v2+v3*v3);
}

//Computing angle
float Aslope(float v1, float v2){
  return (180/PI)*asin(v1/v2);
}

//Return a string for bluetooth or serial print
String textout(){
  String outputString = "Acceleration Xa: ";
  outputString += String((accel_data[X].yval[0] - accel_offset[X]) / ONE_G);
  outputString += ", Ya: ";
  outputString += String((accel_data[Y].yval[0] - accel_offset[Y]) / ONE_G);
  outputString += ", Za: ";
  outputString += String((-accel_data[Z].yval[0] - accel_offset[Z]) / ONE_G);
  outputString += " g";
  outputString += " Rotation Xg: ";
  outputString += String((gyro_data[X].yval[0] - gyro_offset[X]) * RAD2DEG);
  outputString += ", Yg: ";
  outputString += String((gyro_data[Y].yval[0] - gyro_offset[Y]) * RAD2DEG);
  outputString += ", Zg: ";
  outputString += String((gyro_data[Z].yval[0] - gyro_offset[Z]) * RAD2DEG);
  outputString += " degree/s";
  outputString += "\n";
  return outputString;
}


void SendHelp(){
    StartUpWifi();
    // make a HTTP request:
    // send HTTP header
    client.println("GET " + PATH_NAME + queryString + " HTTP/1.1");
    client.println("Host: " + String(HOST_NAME));
    client.println("Connection: close");
    client.println(); // end HTTP header


    while (client.connected()) {
      if (client.available()) {
        // read an incoming byte from the server and print it to serial monitor:
        char c = client.read();
        Serial.print(c);
      }
    }

    // the server's disconnected, stop the client:
    client.stop();
    Serial.println();
    Serial.println("disconnected");
    return;
}

//Check has the SOS been stopped
int DistSOS() {
  if (buttonIsPressed && (millis() - buttonPressTimestamp >= THRESHOLD_DURATION)) {
    // Call your stopTrigger() function here
    Serial.println("5-second button press detected! System deactivated and triggering stopped.");
    buttonIsPressed = 0;
    fall = 0;
    return 1;
  }

  return 0;
}

void buttonISR() {
  if (digitalRead(buttonPin) == LOW) { // Button pressed
    buttonIsPressed = 1;
    buttonPressTimestamp = millis();
  }
}

void startTrigger() {
  isTriggering = true; 
  tone(ledPin,500);// Place your code for the triggering action here 
  Serial.println("Action triggered!"); } 

  
void stopTrigger() { 
  isTriggering = false;
  noTone(ledPin); // Place any necessary code to stop the triggered action here 
  Serial.println("Triggering stopped!"); 
}
