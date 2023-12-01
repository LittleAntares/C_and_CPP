//Function for calibration
void MPU_Calibration(){

  int burn = MAX_DATA+10;
  Serial.println("Calibration Started");

    float ax_offset = 0;
    float ay_offset = 0;
    float az_offset = 0;
    float gx_offset = 0;
    float gy_offset = 0;
    float gz_offset = 0;

  delay(1000);
  for(int step = 0; step < NUM_STEP; step++){
    
      /* Get new sensor events with the readings */
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);


    CurrValaccel[X] = a.acceleration.x-ONE_G;
    CurrValaccel[Y] = a.acceleration.y;
    CurrValaccel[Z] = -a.acceleration.z-ONE_G;

    CurrValgyro[X] = g.gyro.x;
    CurrValgyro[Y] = g.gyro.y;
    CurrValgyro[Z] = g.gyro.z;

    butter_filter(accel_data, CurrValaccel);
    butter_filter(gyro_data, CurrValgyro);
      
    //Acceleration offset
    if(burn > 0){
      --burn;
      --step;
      continue;
    } else {
      ax_offset += accel_data[X].yval[0]/NUM_STEP;
      ay_offset += accel_data[Y].yval[0]/NUM_STEP;
      az_offset += accel_data[Z].yval[0]/NUM_STEP;

    //Gyro Offset
      gx_offset +=  gyro_data[X].yval[0]/NUM_STEP;
      gy_offset += gyro_data[Y].yval[0]/NUM_STEP;
      gz_offset += gyro_data[Z].yval[0]/NUM_STEP;
    }

    delay(10);

  }
  Serial.print("Acceleration Offset for X = "); Serial.print(ax_offset,PRECISION); 
  Serial.print(", Y = "); Serial.print(ay_offset,PRECISION); 
  Serial.print(", Z = "); Serial.println(az_offset,PRECISION);

  Serial.print("Gyro Offset for X = "); Serial.print(gx_offset,PRECISION); 
  Serial.print(", Y = "); Serial.print(gy_offset,PRECISION); 
  Serial.print(", Z = "); Serial.println(gz_offset,PRECISION);
}

//Display Only Serila reading
void DispReadingPureSerial(){

  getData();
  if(burn > 0){
    --burn;
  } else{
    /* Print out the values */
  String outputString = textout();

  Serial.print(outputString);

  Serial.print(outputString);
  }

  delay(1000);
}

//Bluetooth Reading
void BLEReadingLoop(){
  
  BLE.poll();
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());

    digitalWrite(LED_BUILTIN, HIGH);
    while (central.connected() && Mode == COLLECTMODE) {
      ModeCheck();
      long currentMillis = millis();
      // If 1000ms (1 second) have passed, update and send the count as a string:
      if (currentMillis - previousMillis >= 50) {
        previousMillis = currentMillis;
        DispReadingBLE();
      }
    }
    digitalWrite(LED_BUILTIN, LOW);
    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }
}


void DispReadingBLE(){
  getData();
  //Assign Value
  if(burn > 0){
    --burn;
  } else{
    /* Print out the values */
  String outputString = textout();
  Serial.print(outputString);
  matlabCharacteristic.writeValue(outputString);
  }
}

