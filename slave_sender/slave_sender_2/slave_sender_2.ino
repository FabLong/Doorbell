/*
 * Make variable names consistent (not pythonic)
 * Can output array overflow? How to fix this?
 */
//importing libaries
#include <Wire.h>
#include <Rtttl.h>

// CONFIG VALUES
int configPanicButton = 1;


//setting pin variables
const int FIRSTBUTTONPIN = 2;
const int LEDPIN = 13;
const int piezo = 10;
int playing =0;

Rtttl Rtttl(piezo);
FLASH_STRING(sex_bomb,"sexbomb:d=4,o=5,b=125:b,g#,b,g#,8p,c#6,8b,d#6,b,p,8b,8b,8b,8g#,8b,8b,8b,8b,8a#,8a#,8a#,8g#,8a#,8p,b,g#,b,g#,8p,c#6,8b,d#6,b,8p,8d#,8b,8b,8b,8g#,g,8g,8g#,p,b,g#,b,g#,8p,c#6,8b,d#6,b,p,8b,8b,8b,8g#,8b,8b,8b,8b,8a#,8a#,8a#,8g#,8a#,8p,b,g#,b,g#,8p,c#6,8b,d#6,b,8p,8d#,8b,8b,8b,8g#,g,8g,8g#");





/*
 Fields:
        message: char array
        priority: int, smaller value means higher priority
 */
class Message {       // The class
  
  public:             // Access specifier
    int priority;        // Attribute (int variable)
    char message[7];  // Attribute (string variable)
};

// Array of Message objects. Acts like priority output.
// Fix: Static array size of Q_size constant.
const int OUTPUTSIZE = 100;
int endPointer = 0; // Tracks last value in output array.
Message output[OUTPUTSIZE];

void setup() {
  Rtttl.play(sex_bomb);
  Wire.begin(9);                // join i2c bus with address #8
  Wire.onRequest(requestEvent); // register event

  if (configPanicButton == 1) {
    setupPanicButton();
  }

  // Add 'OR',||, if components needs LED.
  if (configPanicButton == 1) {
    setupLEDPIN();
  }

  
  //Serial.begin(9600);  // start serial for output
}

// Setup Panic Button Input; Requires pin 2 for button 1 (as this is an interrupt pin).
void setupPanicButton() {
  attachInterrupt(0, buttonPress, FALLING);
  pinMode(FIRSTBUTTONPIN, INPUT);
}

// Setup LED pin
void setupLEDPIN() {
  pinMode(LEDPIN, OUTPUT);
}


void loop() {
  if (playing ==1){
    Rtttl.updateMelody();
 }
 if (playing ==0){
    Rtttl.play(sex_bomb);
  }
  if (configPanicButton == 1) {
    // Run panic button logic for main loop.
    loopPanicButton();
  }
}


// function that executes whenever data is requested by master
// this function is registered as an event, see setup()
void requestEvent() {
  int message_position = getMessagePosition(output, endPointer);
  Wire.write(getMessage(output, message_position)); // respond with message of 6 bytes
  // as expected by master
  if (playing == 1){
    playing = 0;
  }
  else{
    playing = 1;
  }
}


// Contain panic button logic required in main loop.
void loopPanicButton() {
  // IF panic button led is on, turn it off and add message to array.
  if (digitalRead(LEDPIN) == HIGH) {
    digitalWrite(LEDPIN, LOW);
    Message object;
    object.priority = 1;
    // Issue: might just copy variable, not set.
    strcpy(object.message, "IP0001"); // Sets string to message.
    outputAppend(output, object);
  }
}

// Interrupt routine: Only attatched if config values allow.
void buttonPress() {
  //Serial.write("hello");
  // Turns LED on - signifying panic button press has worked.
  digitalWrite(LEDPIN, HIGH);
  /*
    Ideas:
    Start Alarm
    Send message to master OR add to array of messages.
  */
}


String paddingInt(int x) {
  /*
    Converts an integer to 3 digits long for message.
    Also checks for -ve integers, and integers 4 digits long.
    Returns it as String
  */

  // ERR if -ve integer
  if (x < 0) {
    return "ERR";
  }

  // 4 digits or more, cap at 999.
  if (x > 999) {
    return "999";
  }

  String output;
  if (x < 10) {
    output = "00";
  }
  else if (x < 100) {
    output = "0";
  }

  output += String(x);
  return output;
}

// https://forum.arduino.cc/t/get-minimum-value-from-array-solved/322128/5
/*
 Return message with highest priority from output array.
 Linear approach can be improved upon.
 */
int getMessagePosition(Message* output, int size)
{
  int minimum = output[0].priority;
  int objectPosition = 0;
  for (int i = 0; i < size; i++)
  {  
     if (output[i].priority < minimum && output[i].priority != -1)
        minimum = output[i].priority;
        objectPosition = i;
  }
  // SHould remove message from output here (POP).
  return objectPosition;
  //return output[objectPosition].message;
}

/*
 Gets message from array with index: messagePosition
 Updates the array so the item at the end replaces message output from array.
 Decrements the end pointer.
 
 return: char* message
 */
char* getMessage(Message* output, int messagePosition) {
  // Store message.
  if (endPointer == 0){
    // Prevents array underflow.
    return "000000"; // CHECK this works
  }
  char* string_output = output[messagePosition].message;
  // replace the message with message at the end of the array.
  output[messagePosition] = output[endPointer-1];
  // Decrement the pointer
  endPointer -= 1;
  return string_output;
}

/*
 Appends Message object to output array
 */
void outputAppend(Message* output, Message obj) {
  // Prevents array overflow, stops appending message after array is full.
  // Fix this, as messages are lost.
  if(endPointer != OUTPUTSIZE){
    output[endPointer] = obj;
    endPointer += 1;
  }
}
