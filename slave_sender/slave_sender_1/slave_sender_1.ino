//importing libaries
#include <Wire.h>
#include <Rtttl.h>

// CONFIG VALUES
int configPanicButton = 1;

//setting pin variables
const int firstButtonPin = 3;
const int ledPin = 13;
const int piezo = 10;
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
const int OUTPUTSIZE = 100;
int endPointer = 0; // Tracks last value in output array.
Message output[OUTPUTSIZE];


void setup() {
  Rtttl.play(sex_bomb);
  Wire.begin(8);                // join i2c bus with address #8
  Wire.onRequest(requestEvent);// register event
  if (configPanicButton == 1) {
    setupPanicButton();
  }

  // Add 'OR',||, if components needs LED.
  if (configPanicButton == 1) {
    setupLedPin();
  }
}

// Setup Panic Button Input; Requires pin 2 for button 1 (as this is an interrupt pin).
void setupPanicButton() {
  attachInterrupt(0, buttonPress, FALLING);
  pinMode(firstButtonPin, INPUT);
}

// Setup LED pin
void setupLedPin() {
  pinMode(ledPin, OUTPUT);
}



void loop() {
  Rtttl.updateMelody();
  if (configPanicButton == 1) {
    // Run panic button logic for main loop.
    LoopPanicButton();
  }
  //delay(1000);
}

// function that executes whenever data is requested by master
// this function is registered as an event, see setup()
void requestEvent() {
  
  int messagePosition = getMessagePosition(output, endPointer);
  Wire.write(getMessage(output, messagePosition)); // respond with message of 6 bytes
  // as expected by master
}




// Contain panic button logic required in main loop.
void LoopPanicButton() {
  // IF panic button led is on, turn it off and add message to array.
  if (digitalRead(ledPin) == HIGH) {
    digitalWrite(ledPin, LOW);
    Message object;
    object.priority = 1;
    // Issue: might just copy variable, not set.
    strcpy(object.message, "IP0001"); // Sets string to message.
    outputAppend(output, object);
  }
}

// Interrupt routine: Only attatched if config values allow.
void buttonPress() {
  // Turns LED on - signifying panic button press has worked.
  digitalWrite(ledPin, HIGH);
  
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
    if (output[i].priority < minimum)
      minimum = output[i].priority;
    objectPosition = i;
  }
  // Should remove message from output here (POP).
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
  if (endPointer == 0) {
    // Prevents array underflow.
    return "000000"; // Default message to send when array is empty.
  }
  char* stringOutput = output[messagePosition].message;
  // replace the message with message at the end of the array.
  output[messagePosition] = output[endPointer - 1];
  // Decrement the pointer
  endPointer -= 1;
  return stringOutput;
}

/*
  Appends Message object to output array
*/
void outputAppend(Message* output, Message obj) {
  // Prevents array overflow, stops appending message after array is full.
  // Fix this, as messages are lost.
  if (endPointer != OUTPUTSIZE) {
    output[endPointer] = obj;
    endPointer += 1;
  }
}

void song(){
  
}
