
unsigned long startTime = millis();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial.println("MHZ test");
}

void loop() {
  Serial.println("------------------------------");
  Serial.print("Time from start: ");
  Serial.print((millis() - startTime) / 1000);
  Serial.println(" s");
  int ppm_uart = readCO2UART();
  
}

int readCO2UART(){
  byte cmd[9] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79};
  byte response[9]; // for answer

  Serial.println("Sending CO2 request...");
  Serial1.write(cmd, 9); //request PPM CO2

  // clear the buffer
  memset(response, 0, 9);
  int i = 0;
  while (Serial1.available() == 0) {
//    Serial.print("Waiting for response ");
//    Serial.print(i);
//    Serial.println(" s");
    delay(1000);
    i++;
  }
  if (Serial1.available() > 0) {
      Serial1.readBytes(response, 9);
  }
  // print out the response in hexa
  for (int i = 0; i < 9; i++) {
    Serial.print(String(response[i], HEX));
    Serial.print("   ");
  }
  Serial.println("");

  // checksum
  //byte check = getCheckSum(response);
  //if (response[8] != check) {
  //  Serial.println("Checksum not OK!");
  //  Serial.print("Received: ");
  //  Serial.println(response[8]);
  //  Serial.print("Should be: ");
  //  Serial.println(check);
  //}
 
  // ppm
  int ppm_uart = 256 * (int)response[2] + response[3];
  Serial.print("PPM UART: ");
  Serial.println(ppm_uart);

  // temp
  byte temp = response[4] - 40;
  Serial.print("Temperature? ");
  Serial.println(temp);

  // status
  byte status = response[5];
  Serial.print("Status? ");
  Serial.println(status);
  if (status == 0x40) {
    Serial.println("Status OK");
  }
 
  return ppm_uart;
}
