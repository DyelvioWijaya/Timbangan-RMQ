/* 
 *  PLEASE DONT DELETE AND EDIT 
 *  Develop by          : Nurman Hariyanto
 *  Email               : nurman.hariyanto13@gmail.com
 *  Project             : Workshop Soil Moisture
 *  Version             : 1.0
 *  Description         : This code for get data from soil moisture and send data soil to MQTT/RMQ
 *  Microcontroller     : Wemos Mini ESP8266
 *                        NodeMCU ESP8266
 *  Device              : Soil Moisture Sensor Module                   
 */




/*
 * YOU CAN EDIT NOW
 */

/*
 * Include library 
 */
 #include "HX711.h" //import Library
 #include <PubSubClient.h> // import Library
 #include <ESP8266WiFi.h> // import Library
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * Connection parameter
 * -Wifi -> internet connection foor data communication
 * -RabbitMQ/MQTT -> protoocol data communication
 */
const char* wifiSsid              = "LSKK_HomeAuto"; // Deklarasi nama Wifi
const char* wifiPassword          = "1234567890"; // Deklarasi untuk Password Wifi
const char* mqttHost              = "rmq2.pptik.id"; // Deklarasi untuk link yang akan dituju
const char* mqttUserName          = "/workshopmitra:workshopmitra"; // Deklarasi untuk nama UserName DI RMQ
const char* mqttPassword          = "passwordnyaqwerty"; // Deklarasi untuk Passwordnya di RMQ
//const char* mqttClient            = "IOT-Water-Pumpp"; 
const char* mqttQueueTimbangan       = "Timbangan"; // Deklarasi untuk nama Queue di RMQ
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*7
 * Device parameter
 * -Guid -> Id Device (unique) you can generate from here (https://www.uuidgenerator.net/version1) 
 * -status device -> save last state from the pump is on or off (1 = on , 0 = off) 
 * -pin microcontroller 
 * -mac device
 * 
 */
int deviceDout                   = 4; // Deklarasi Pin 
int deviceSck                    = 12; // Deklarasi Pin


HX711 scale(deviceDout, deviceSck); 
float calibration_factor = 109525; // Memasukan Nilai "109525" ke Varibel calibration_factor dalam tipe data Float


/*
 * Wifi setup WiFi client and mac address 
 */
WiFiClient espClient;
PubSubClient client(espClient);
byte mac[6]; //array temp mac address
String MACAddress;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * Set up WiFi connection
 */
 void setup_wifi(){
  delay(10); //Delay waktu 0,01 detik
  //We start by connecting to a WiFi network
  Serial.println(); // Tampilkan Di serial monitor
  Serial.print("Connecting to :"); //Tampilkan di serial monitor "Connecting to :"
  Serial.println(wifiSsid); // Tampilkan wifiSsid
  WiFi.begin(wifiSsid, wifiPassword); // Masukaan wifi dengan nama dan passwordnya
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); // delay 0,5 detik
    Serial.print("."); // tampilkan " . "
  }
  Serial.println(""); //"tampilkan ""
  Serial.println("WiFi connected");// Tampilkan Wifi Connected
  Serial.println("IP address: "); // Tampilkan Ip Adrees :
  Serial.println(WiFi.localIP()); // Menampilkan Ip 
 }
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * Function for Get Mac Address from microcontroller
 */
 
String mac2String(byte ar[]) {
  String s;
  for (byte i = 0; i < 6; ++i)
  {
    char buf[3];
    sprintf(buf, "%2X", ar[i]);
    s += buf;
    if (i < 5) s += ':';
  }
  return s;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * Function for Print Mac Address 
 */
 void printMACAddress() {
  WiFi.macAddress(mac);
  MACAddress = mac2String(mac);
  Serial.println(MACAddress); // Tampilkan MacAddress
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



/*
 * Function for Get message payload from MQTT rabbit mq
 */
void callback(char* topic, byte* payload, unsigned int length){
  char message[5]; //variable for temp payload message
  Serial.print("Message arrived in topic: "); // Tampilkan Message arrived in topic
  Serial.println(topic); // tampilkan topic
  Serial.print("Messagge :"); // tampilkan Message :
  for(int i = 0;i < length;i++){
    Serial.print((char)payload[i]);
    message[i] = (char)payload[i]; //initiate value from payload to message variable
    
  }
  
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * Function for Reconnecting to MQTT/RabbitMQ 
 */
void reconnect() {
  // Loop until we're reconnected
  printMACAddress();
  const char* CL;
  CL = MACAddress.c_str();
  Serial.println(CL);
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(CL, mqttUserName, mqttPassword)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      ESP.restart();
      delay(5000);

    }
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/*
 * Function for Setup Pin Mode,wifi,mqtt,and serial
 */
void setup()
{
  pinMode(deviceDout, INPUT);
  Serial.begin(115200);
  setup_wifi();
  printMACAddress();
  client.setServer(mqttHost, 1883);
  client.setCallback(callback);
  scale.set_scale(80900);
  scale.tare(); //Reset the scale to 0
 
  long zero_factor = scale.read_average(); //Get a baseline reading
  Serial.print("Zero factor: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
  Serial.println(zero_factor);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



/*
 * This functioon for loop your program
 */
void loop() {
  //if you disconnected from wifi and mqtt
  if (!client.connected()) {
    reconnect();
  }
scale.set_scale(calibration_factor); //Adjust to this calibration factor
// Serial.println( scale.set_scale(calibration_factor)); 
float dataWeight;

  //Serial.print("Reading: ");
  // Serial.println(scale.get_units(), 2);
 dataWeight = ((scale.get_units()));
  //Serial.print(dataWeight);
 String convertDataWeight = String(dataWeight);
 char dataToMQTT[50];
 convertDataWeight.toCharArray(dataToMQTT, sizeof(dataToMQTT));
 Serial.print("Ini Data untuk ke MQTT: ");
 Serial.print(dataToMQTT);
 Serial.println(" Kg");

 client.publish(mqttQueueTimbangan,dataToMQTT);
  client.loop();
  delay(1000);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
