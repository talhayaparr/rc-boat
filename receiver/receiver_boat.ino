#include "SPI.h" 
#include "RF24.h" 
#include "nRF24L01.h" 
#include <Servo.h>

#define CE_PIN 9 
#define CSN_PIN 10 
#define INTERVAL_MS_SIGNAL_LOST 1000 
#define INTERVAL_MS_SIGNAL_RETRY 250

Servo esc;
Servo servo_steering;

//Define pins
const int pin_esc_speed = A1;
const int pin_servo_steering = A2;
const int pin_button_cutFishingLine = 2;
const int pin_button_openHeadLight = 3;

RF24 radio(CE_PIN, CSN_PIN); 
const byte address[6] = {'T','T','1','0','0','0'}; 
//NRF24L01 buffer limit is 32 bytes (max struct size) 
struct payload
{ 
  int speed_boat;
  int steering_boat;
  bool cutFishingLine_boat;
  bool openHeadLight_boat;  
};
payload data2send_rf; 
unsigned long lastSignalMillis = 0;
 
void setup() 
{ 
  //Attach the ESC and Servo pins and its min max pulse widths;
  esc.attach(9,1000,2000); // (pin, min pulse width, max pulse width in microseconds) 
  servo_steering.attach(10,1000,2000); // (pin, min pulse width, max pulse width in microseconds) 
  
  //Define pins mode
  pinMode (pin_esc_speed, OUTPUT);
  pinMode (pin_servo_steering, OUTPUT);
  pinMode (pin_button_cutFishingLine, OUTPUT);
  pinMode (pin_button_openHeadLight, OUTPUT);
  
  Serial.begin(115200);
  delay(10);
  initializeCommunicationAsReceiver();
}
void loop() 
{ 
  unsigned long currentMillis = millis(); 
  if (radio.available() > 0)
  { 
    radio.read(&data2send_rf, sizeof(data2send_rf));

    esc.write(data2send_rf.speed_boat);
    servo_steering.write(data2send_rf.steering_boat);
    digitalWrite(pin_button_cutFishingLine, data2send_rf.cutFishingLine_boat);
    digitalWrite(pin_button_openHeadLight, data2send_rf.openHeadLight_boat);
    
    Serial.println("Received"); 
    Serial.print("speed_boat:"); 
    Serial.println(data2send_rf.speed_boat); 
    Serial.print("steering_boat:"); 
    Serial.println(data2send_rf.steering_boat);
    Serial.print("cutFishingLine_boat:"); 
    Serial.println(data2send_rf.cutFishingLine_boat); 
    Serial.print("openHeadLight_boat:"); 
    Serial.println(data2send_rf.openHeadLight_boat); 
    lastSignalMillis = currentMillis; 
  } 
  if (currentMillis - lastSignalMillis > INTERVAL_MS_SIGNAL_LOST)
  { 
    lostConnection(); 
  } 
} 
void lostConnection() 
{ 
  Serial.println("We have lost connection, preventing unwanted behavior"); 
  
  esc.write(0);
  servo_steering.write(90);
  digitalWrite(pin_button_cutFishingLine, HIGH);
  digitalWrite(pin_button_openHeadLight, HIGH);

  if (radio.failureDetected = true)
  {
    initializeCommunicationAsReceiver();
    radio.failureDetected = 0;           // Reset the detection value
    delay(100);
  }
  delay(INTERVAL_MS_SIGNAL_RETRY);
}
void initializeCommunicationAsReceiver()
{
  radio.begin(); 
  //Append ACK packet from the receiving radio back to the transmitting radio 
  radio.setAutoAck(false); //(true|false) 
  //Set the transmission datarate 
  radio.setDataRate(RF24_250KBPS); //(RF24_250KBPS|RF24_1MBPS|RF24_2MBPS) 
  //Greater level = more consumption = longer distance 
  radio.setPALevel(RF24_PA_MAX); //(RF24_PA_MIN|RF24_PA_LOW|RF24_PA_HIGH|RF24_PA_MAX) 
  //Default value is the maximum 32 bytes1 
  radio.setPayloadSize(sizeof(data2send_rf)); 
  //Act as receiver 
  radio.openReadingPipe(0, address); 
  radio.startListening();
}
