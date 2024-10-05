#include "SPI.h" 
#include "RF24.h" 
#include "nRF24L01.h" 
#define CE_PIN 9 
#define CSN_PIN 10 
#define INTERVAL_MS_TRANSMISSION 250 
#define SURPASS_MAX_RPM 7300 //it is actually 8700 but we are using approx.%85 of cap of motor because if we use %100 esc or motor burn out.
#define MAX_STEERING_ANGLE 180
//Define pins
const int pin_joyStick_speed = A4;
const int pin_joyStick_steering = A5;
const int pin_button_cutFishingLine = 1;
const int pin_button_openHeadLight = 2;

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
void setup() 
{ 
  //Define pins mode
  pinMode (pin_joyStick_speed, INPUT);
  pinMode (pin_joyStick_steering, INPUT);
  pinMode (pin_button_cutFishingLine, INPUT);
  pinMode (pin_button_openHeadLight, INPUT);
  
  Serial.begin(115200);
  initializeCommunicationAsTransmitter();
} 
void loop() 
{
  //Prepare data to send
  data2send_rf.speed_boat = map(analogRead(pin_joyStick_speed), 0, 1023, 0, SURPASS_MAX_RPM);
  data2send_rf.steering_boat = map(analogRead(pin_joyStick_steering), 0, 1023, 0, MAX_STEERING_ANGLE);
  data2send_rf.cutFishingLine_boat = digitalRead(pin_button_cutFishingLine);
  data2send_rf.openHeadLight_boat = digitalRead(pin_button_openHeadLight);
  
  //Send data to boat
  radio.write(&data2send_rf, sizeof(data2send_rf)); 
  Serial.print("speed_boat:"); 
  Serial.println(data2send_rf.speed_boat); 
  Serial.print("steering_boat:"); 
  Serial.println(data2send_rf.steering_boat); 
  Serial.print("cutFishingLine_boat:"); 
  Serial.println(data2send_rf.cutFishingLine_boat); 
  Serial.print("openHeadLight_boat:"); 
  Serial.println(data2send_rf.openHeadLight_boat); 
  Serial.println("Sent"); 
  delay(INTERVAL_MS_TRANSMISSION/5);
}
void initializeCommunicationAsTransmitter()
{
  radio.begin(); 
  //Append ACK packet from the receiving radio back to the transmitting radio 
  radio.setAutoAck(false); //(true|false) 
  //Set the transmission datarate 
  radio.setDataRate(RF24_250KBPS); //(RF24_250KBPS|RF24_1MBPS|RF24_2MBPS) 
  //Greater level = more consumption = longer distance 
  radio.setPALevel(RF24_PA_MAX); //(RF24_PA_MIN|RF24_PA_LOW|RF24_PA_HIGH|RF24_PA_MAX) 
  //Default value is the maximum 32 bytes 
  radio.setPayloadSize(sizeof(data2send_rf)); 
  //Act as transmitter 
  radio.openWritingPipe(address); 
  radio.stopListening(); 
}
