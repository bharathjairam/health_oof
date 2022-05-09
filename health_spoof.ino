#include <ESP8266WiFi.h>
#include <Wire.h>

#include "Ubidots.h"


//WiFiClient client;

const char* UBIDOTS_TOKEN = "BBFF-LSAMfo2d3QO7ecxZODofei2q73m8U5";
const char* ssid     = "MAJOR PROJECT"; // Your ssid
const char* password = "12345678"; // Your Password
Ubidots ubidots(UBIDOTS_TOKEN, UBI_HTTP);


//GAS sensor
const int gasPin = A0; //GAS sensor output pin to Arduino analog A0 pin
float gas;

const int ProxSensor = D7;
int irsensor();
const int IroxSensor = D2;

//Pulse sensor
long BPM;
float B_Temp;



void setup() {

  // Initialize Serial port
  Serial.begin(115200);
  delay(100);

  ubidots.wifiConnect(ssid, password);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi is connected");


  pinMode(gasPin, INPUT);
  pinMode(ProxSensor, INPUT);
  pinMode(IroxSensor, INPUT);

}


void loop() {

  //pulse sensor
  if (irsensor() == 1)
  {
    pulse rate = int BPM();
    Serial.print("Pulse rate: ");
    Serial.println(BPM);

    B_Temp = B_temp();
    Serial.print("Body temperature: ");
    Serial.println(B_Temp);



  }

  int ECG();

  int Motion = digitalRead(IroxSensor);

  //gas sensor
  gas = (analogRead(gasPin));


  ubidots.add("Temperature", B_Temp);// Change for your variable name
  ubidots.add("Pulse Rate", BPM);
  ubidots.add("MOTION", ECG);
  ubidots.add("Gas Quality", gas);


  bool bufferSent = false;
  bufferSent = ubidots.send(); // Will send data to a device label that matches the device Id
  if (bufferSent) {
    Serial.println("Values sent by the device");
  }
  delay(3000);


}



int irsensor()

{
  int state = digitalRead(ProxSensor);
  if (state == 0)

  {
    Serial.println();
    delay(1000);
    state;
    return (1);
  }
  else
  {
    Serial.println("Place your hand on Pulse Meter");
    delay(1000);
    return (0);
  }
}

int ECG() {

  int mynum[] = { -1, 0, -1, 0, 1, -1, 6, 0, 1, 2, 3, 0, 0, 0, 0};

  for (int x = 0; x < 15; x++) {
    int ecg = (mynum[x]);
    //erial.println(ecg);
    return ecg;

  }

}

void BPM() {                        // triggered when flipper fires....
  cli();                               // disable interrupts while we do this
  Signal = analogRead(A0);              // read the Pulse Sensor
  sampleCounter += 2;                         // keep track of the time in mS with this variable
  int N = sampleCounter - lastBeatTime;       // monitor the time since the last beat to avoid noise

  //  find the peak and trough of the pulse wave
  if (Signal < thresh && N > (IBI / 5) * 3) { // avoid dichrotic noise by waiting 3/5 of last IBI
    if (Signal < T) {                       // T is the trough
      T = Signal;                         // keep track of lowest point in pulse wave
    }
  }

  if (Signal > thresh && Signal > P) {        // thresh condition helps avoid noise
    P = Signal;                             // P is the peak
  }                                        // keep track of highest point in pulse wave

  //  NOW IT'S TIME TO LOOK FOR THE HEART BEAT
  // signal surges up in value every time there is a pulse
  if (N > 250) {                                  // avoid high frequency noise
    if ( (Signal > thresh) && (Pulse == false) && (N > (IBI / 5) * 3) ) {
      Pulse = true;                               // set the Pulse flag when we think there is a pulse
      digitalWrite(blinkPin, HIGH);               // turn on pin 13 LED
      IBI = sampleCounter - lastBeatTime;         // measure time between beats in mS
      lastBeatTime = sampleCounter;               // keep track of time for next pulse

      if (secondBeat) {                      // if this is the second beat, if secondBeat == TRUE
        secondBeat = false;                  // clear secondBeat flag
        for (int i = 0; i <= 9; i++) {       // seed the running total to get a realisitic BPM at startup
          rate[i] = IBI;
        }
      }

      if (firstBeat) {                       // if it's the first time we found a beat, if firstBeat == TRUE
        firstBeat = false;                   // clear firstBeat flag
        secondBeat = true;                   // set the second beat flag
        sei();                               // enable interrupts again
        return;                              // IBI value is unreliable so discard it
      }


      // keep a running total of the last 10 IBI values
      word runningTotal = 0;                  // clear the runningTotal variable

      for (int i = 0; i <= 8; i++) {          // shift data in the rate array
        rate[i] = rate[i + 1];                // and drop the oldest IBI value
        runningTotal += rate[i];              // add up the 9 oldest IBI values
      }

      rate[9] = IBI;                          // add the latest IBI to the rate array
      runningTotal += rate[9];                // add the latest IBI to runningTotal
      runningTotal /= 10;                     // average the last 10 IBI values
      BPM = 60000 / runningTotal;             // how many beats can fit into a minute? that's BPM!
      QS = true;                              // set Quantified Self flag
      // QS FLAG IS NOT CLEARED INSIDE THIS ISR
    }
  }

  if (Signal < thresh && Pulse == true) {  // when the values are going down, the beat is over
    digitalWrite(blinkPin, LOW);           // turn off pin 13 LED
    Pulse = false;                         // reset the Pulse flag so we can do it again
    amp = P - T;                           // get amplitude of the pulse wave
    thresh = amp / 2 + T;                  // set thresh at 50% of the amplitude
    P = thresh;                            // reset these for next time
    T = thresh;
  }

  if (N > 2500) {                          // if 2.5 seconds go by without a beat
    thresh = 512;                          // set thresh default
    P = 512;                               // set P default
    T = 512;                               // set T default
    lastBeatTime = sampleCounter;          // bring the lastBeatTime up to date
    firstBeat = true;                      // set these to avoid noise
    secondBeat = false;                    // when we get the heartbeat back
  }

  sei();                                   // enable interrupts when youre done!
}// end isr

B_temp()
{
  sensors.requestTemperatures();                // Send the command to get temperatures
  Serial.println("Temperature is: ");
  Serial.println(sensors.getTempCByIndex(0));   // Why "byIndex"? You can have more than one IC on the same bus. 0 refers to the first IC on the wire
  delay(500);
}
