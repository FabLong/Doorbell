// RFID code inspired by https://wiki.seeedstudio.com/Grove-125KHz_RFID_Reader/
// Accesed 3/12/21

#include <Wire.h>
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
#include <Servo.h>


# define I2C_SLAVE1_ADDRESS 8
# define I2C_SLAVE2_ADDRESS 9
# define I2C_SLAVE3_ADDRESS 10 // Pretend slave 3 POC

// Config variables
String modesList[4] = {"Off  ", "Fab  " , "Solly" , "Dan  "}; // Spaces in mode so LCD removes character when updating screen.
int numberModes = 4; // Length of the number of modes we have set
int sleepLocations[1] = {1}; //ignore these rooms when we are sleeping, e.g {1,5} would ignore slave arduion's 1 and 5
int numberSleepLocations = 1;

// Create constants for Pin Names
const int rs = 12, en = 11, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
int const piezo = 10; //pin 8 is now being used by LCD to set contrast.
int const ButtonOnePin = 2;
int const ButtonTwoPin = 3;
int const servoPin = 13;
//int const SpeakerPin = 0; //need to change this 

// NEW variables
int Contrast = 60; // Sets LCD contrast value.
int pressedBasic = 0; // Boolean to track whether the basic button has been pressed during interrupt.
int pressedSecret = 0; // Boolean to track whether the secret button has been presssed during interrupt.
int servoPosition = 90;

int requestSlave1 = 0; // Boolean to track whether to Slave 1 needs to make a request
int requestSlave2 = 0; // Boolean to track whether to Slave 1 needs to make a request
int requestDoorbell = 0; // Boolean to track whether to call doorbell ringtone code.


//setup libarys
SoftwareSerial SoftSerial(10, 9);
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
Servo modeServo;


//definining global variables
char message[7];
int mode = 0;
int lastMode = mode;

//creating pixart
byte distance[8] {
  B00111,
  B00011,
  B00101,
  B01000,
  B00000,
  B00000,
  B00000,
};
byte temperature[8] {
  B01010,
  B00010,
  B01010,
  B00010,
  B01010,
  B10001,
  B10001,
  B01110,
};

byte light[8] {
  B00000,
  B01110,
  B10001,
  B10101,
  B10101,
  B01010,
  B00100,
  B00000,
};
byte modePixel[8] {
  B10000,
  B00000,
  B11000,
  B00000,
  B11100,
  B00000,
  B11110,
  B00000,
};

// Defining our lists
// Reduce use of global variables.
String allowsValues[] = {"3B00D56D9516", "310098A80100"};
int numberAllowed = 2; // Length of the number of allowed values we have in our list
int authenticated = 0; // Set authenticated value to 0, indicating unauthenticated status.
// Screen dims in 'Off' mode.
int servoLocation[] = {90, 0, 0, 180};

void setup() {
  Wire.begin();        // Join i2c bus (address optional for master)
  analogWrite(8, Contrast);
  lcd.begin(16, 2);    // Set up LCD's num columns and rows.
  lcd.clear();         // Clear LCD Buffer

  //Creating our pixart values so we can display them.
  lcd.createChar(0, distance);
  lcd.createChar(1, temperature);
  lcd.createChar(2, light);
  lcd.createChar(3, modePixel);

  //Serial.begin(9600);  // Start serial for output // do we want this.
  //SoftSerial.begin(9600);     // RFID (the SoftSerial baud rate)
  attachInterrupt(1, doorbell, FALLING); // Attaching interrupt so button changes the mode of system.
  attachInterrupt(0, setMode, FALLING);
  pinMode(ButtonOnePin, INPUT_PULLUP);
  pinMode(ButtonTwoPin, INPUT_PULLUP);
  modeServo.attach(servoPin);

  // Set Initial mode on LCD and Servo position.
  mode = 0;
  String modeName = modesList[mode];
  lcd.setCursor(10, 1);
  lcd.write(byte(3));
  lcd.print(modeName);
  modeServo.write(servoPosition); // set servo initial position.
}

void loop() {
  String modeName = modesList[mode];
  lcd.setCursor(10, 1);
  lcd.write(byte(3));
  lcd.print(modeName);
  
  
  int potValue = analogRead(A0); // Poteniometer value
  potValue = map(potValue, 0, 1023, 0, 3); // Map the analogue value: modes 0, 1, 2, 3
  modeName = modesList[potValue];
  lcd.setCursor(0, 1);
  lcd.print("SELECT:   ");
  lcd.write(byte(3));
  //lcd.print(" ");
  lcd.print(modeName);

  
  lastMode = mode;



  if(requestDoorbell == 1){
    doorbell_sound(); //CHANGE ME TO NICE SOUND
    requestDoorbell = 0;
  }
  checkModeChange();
  lastMode = mode;

 
  checkModeChange();
  lastMode = mode;
  delay(200);
  // This code is in the loop; can't run lcd code in interrupt.
  checkModeChange();
  lastMode = mode;
  
  if(servoPosition != 90){
    delay(1000);
    servoPosition = 90;
    modeServo.write(90); // reset servo position.
  }

  lcd.setCursor(0,0);
  lcd.print("               ");
}

void checkModeChange(){
  if (mode != lastMode) { // If our mode has changed within the loop we update this on the LCD
    String modeName = modesList[mode];
    int adress;
    if (mode == 1){
      adress = I2C_SLAVE1_ADDRESS;
    }
    else{
      adress = I2C_SLAVE2_ADDRESS;
    }
    // If mode is 'off' we want to clear the data on top row of LCD.
    if (mode == 0){
      lcd.setCursor(0,0);
      lcd.print("               ");
    }
    lcd.setCursor(10, 1);
    lcd.write(byte(3));
    lcd.print(modeName);
    //Serial.write("11");
    getMessage(adress); // Gets the message from slave 2
    checkMessage(adress); // Shows the message on our LCD and handles it if nesssecary
    delay(1000);
    getMessage(adress); // Gets the message from slave 2
    checkMessage(adress);

    if(mode == 3){
      lcd.setCursor(0, 0);
      lcd.write(byte(3));
      lcd.print("Really?");
    } else{
      lcd.setCursor(0, 0);
      lcd.write(byte(3));
      lcd.print("Sexy ;)");
    }
    
    mode = 0;
    lastMode = 0;
  }
}
/*
   Function to check what message has been sent frmo our node, and display this on the LCD and handle it if nessecary
   Parameters: slaveAdress
*/
void checkMessage(int slaveAdress) {
  String toPrint = "" ;
  char type;
  char sensor;
  int skip = 0;
  type = message[0];
  //Serial.print(type);
  //Serial.write("1");
  //Serial.write(type);
  if (1) { //if we have an input, and our mode is not off
    //Serial.write("3");
    sensor = message[1];
        toPrint.concat("PANIC");
        toPrint.concat(':');
        toPrint.concat(message[2]);
        lcd.setCursor(0, 1);
        lcd.print(toPrint);
        //setAlarm(slaveAdress); 
        lcd.setCursor(0, 1); //reset our cursor location
        lcd.write("        "); //clear our display once the panic alarm has stopped running 
    }
}
/*
   Requests a mesasge from slave at adress specified in the paramater and adds it to our message variable
   Paramater: slaveAdress
*/
void getMessage(int slaveAdress) {
  int counter = 0;
  Wire.requestFrom(slaveAdress, 6);    // request 6 bytes from slave device 1
  while (Wire.available()) { // slave may send less than requested these will be replaced with non-values
    char thisCharacter = Wire.read();
    //Serial.print(Wire.available());
    message[counter] = thisCharacter;// receive a byte as character
    counter ++;
  }
  message[counter] = 0;
}

void doorbell_sound(){
  detachInterrupt(0); //using software serial interferes with interupt procces, so we detach the interupt while running our function
  detachInterrupt(1);
  for (int x = 0; x < 10; x++){
    tone(piezo, 500, 400);
    delay(100);
    tone(piezo,200, 400);
    delay(100);
  }
  attachInterrupt(1, doorbell, FALLING); //reattaching intterupt for the rest of the program
  attachInterrupt(0, setMode, FALLING);
}


/*
   This function will run to check if we have a recognised rfid tag being sensed, and returns 1 if this is true, and 0 if false
*/
int checkRfid() { //checks whether an rfid tag is valid
  unsigned char rfidCode [14];
  // if date is coming from software serial port ==> data is coming from SoftSerial shield
  int count = 0;
  if (SoftSerial.available()) //if there is an rfid tag to read
  {
    while (SoftSerial.available())              //reading data into array
    {
      rfidCode[count++] = SoftSerial.read();
      if (count == 16)break; //if our array is full
    }
    for (int x = 0; x < numberAllowed; x ++) { //for values in our allowed list
      unsigned char rfidChar[14];
      allowsValues[x].toCharArray(rfidChar, 14); //turns our string value to a character array so we can compare it to the read value
      int same = 1; //assume values are the same until we find a value that is different
      for (int y = 1; y < 13; y ++) { //for value in arrays, skipping 0 and 14 as these are null characters.
        if (rfidCode[y] != rfidChar[y]) {
          same = 0;
        }
      }
      if (same == 1) {
        return 1;
      }
    }
  }
  return 0;

}
/*
   Interrupt when we we press button, allows us to cycle through modes and will run the setServo() method to reflect this
*/
void setMode() {
  int potValue = analogRead(A0); // Poteniometer value
  potValue = map(potValue, 0, 1023, 0, 3); // Map the analogue value: modes 0, 1, 2, 3
  // We need to display changing of the modes, although we cant do this in the interrupt.
  mode = potValue;

  /*
   * Old code
  if (mode == (numberModes - 1)) { //this means we are trying to add one to a full list (-1 becuase zero indexed), so we resest mode to the start
    mode = 0;
  }
  else {
    mode ++;
  }
  */
  setServo();
  // Serial.print(mode);
}

void doorbell() {
  requestDoorbell = 1;
}

/*
   Sets the servo to show the current mode
*/
void setServo() {
  servoPosition = servoLocation[mode];
  modeServo.write(servoLocation[mode]);
}
