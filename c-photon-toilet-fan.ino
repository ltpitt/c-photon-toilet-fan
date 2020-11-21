// This settings will allow the product to work even without Particle Cloud connection
//SYSTEM_THREAD(ENABLED)
//SYSTEM_MODE(SEMI_AUTOMATIC)

unsigned long previousMillis = 0;        // Will store last time fan was turned on
long interval = 300000;                 // Interval at which the fan will stop (milliseconds)
bool isFanOn = false;
bool isFanEnabled = true;
int buttonPin = D0;
int fanPin = D1;
int ledPin = D7;


int cloudCommand(String command) {
    // Getting index of all the commas in the command
    int firstCommaIndex = command.indexOf(',');
    int secondCommaIndex = command.indexOf(',', firstCommaIndex+1);
    // Using that indexes to split the command subunits
    String function = command.substring(0, firstCommaIndex);
    String parameter = command.substring(firstCommaIndex+1, secondCommaIndex);
    // Printing to serial the received data
    Serial.print("function: " + function);  
    Serial.println();
    Serial.print("parameter: " + parameter);  
    Serial.println();
    // Execute the right function according to the one requested in the http call    
    if (function == "turnOn") {
        turnOnFan();
        return 1;
    } else if (function == "turnOff") {
        turnOffFan();
        return 0;
    } else if (function == "disable") {
        turnOffFan();
        isFanEnabled = false;
        Particle.publish("Fan DISABLED", PRIVATE);
        return 0;
    } else if (function == "enable") {
        Particle.publish("Fan ENABLED", PRIVATE);
        isFanEnabled = true;
        return 0;
    } else if (function == "setInterval") {
        interval = parameter.toInt();
        return interval;
    }
    return -1;
}


void turnOnFan(){
    if (isFanEnabled) {
        isFanOn = true;
        previousMillis = millis();
        digitalWrite(fanPin, LOW);
        digitalWrite(ledPin, HIGH);
        Particle.publish("Fan ON", PRIVATE);
        Serial.println("Fan ON");
        delay(500);
    } else {
        //Particle.publish("Fan ON attempt but currently isFanEnabled = false", PRIVATE);
        Serial.println("Turning fan ON attempt but currently isFanEnabled = false");
    }
}

void turnOffFan(){
    isFanOn = false;
    previousMillis = millis();
    digitalWrite(fanPin, HIGH);
    digitalWrite(ledPin, LOW);
    Particle.publish("Fan OFF", PRIVATE);    
    Serial.println("Fan OFF");
    delay(500);
}

void checkTimeout() {
    unsigned long currentMillis = millis();
    if (isFanOn && currentMillis - previousMillis >= interval) {
        turnOffFan();
    }
}

void startupBlinkSequence(){
   for (int i=0; i < 3; i++){
        digitalWrite(ledPin, HIGH);
        delay(1000);
        digitalWrite(ledPin, LOW);
        delay(1000);
   }

   for (int i=0; i < 20; i++){
        digitalWrite(ledPin, HIGH);
        delay(50);
        digitalWrite(ledPin, LOW);
        delay(50);
   }
   Particle.publish("Fan READY","123");
}

void setup() {
//    Particle.connect();
    Particle.publish("Fan INITIALIZING","123");
    Serial.println("Fan INITIALIZING");
    Particle.function("cloudCommand", cloudCommand);
    Particle.variable("isFanOn", isFanOn);
    Particle.variable("isFanEnabled", isFanEnabled);
    pinMode(buttonPin, INPUT_PULLUP);
    pinMode(fanPin, OUTPUT);
    pinMode(ledPin, OUTPUT);    
    digitalWrite(fanPin, HIGH);
    startupBlinkSequence();
}

void loop(){
    
    if (!Particle.connected()) {
      if (!WiFi.ready()) {
        if (!WiFi.connecting()) {
          //Particle.publish("Wifi down, REBOOT", PRIVATE);
          Serial.println("Wifi is down, rebooting...");
          delay(60000);
          System.reset();
        }
      }
    }
    
    checkTimeout();
    
    if (digitalRead(buttonPin) == LOW){ 
        if (!isFanOn) {
            turnOnFan();
        }
    } else {
        if (!isFanOn) {
            digitalWrite(ledPin, LOW);
        }
    }
    
}
