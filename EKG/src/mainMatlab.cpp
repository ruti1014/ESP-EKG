/*
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

static char * ssid = "";
static char * pass = ""; 

static int localPort = 4069;

//udp of matlab
static int matlabPort = 5042;


WiFiUDP udp;

void setup() {

  Serial.begin(9600);


  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  //Wait for Wifi
  Serial.print("Waiting for wifi ");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }
  Serial.println("\nConnected!");
  Serial.print("ESP32-IP: "); Serial.println(WiFi.localIP());
  
  Serial.print("Waiting for udp!");
  udp.begin(WiFi.localIP(), localPort);
}


char * buffer;
void loop() {

  if(udp.read(buffer, 50) > 0){
    Serial.print("Server to client: ");
    Serial.println((char *)buffer);
  }
/*
  udp.beginPacket("192.168.0.54", 5042);
  udp.printf(("New phone who dis?"));
  udp.endPacket();
  delay(5000);
  clc;
clear;


espPort = 5042;
espIP = "192.168.0.54";

localPort = 25236;
localHost = "192.168.0.54";
udp = udpport('LocalPort', localPort,'LocalHost',localHost);


%write(u, 1:10, "uint8", espIP, espPort);

while (true)
    write(udp, "new Phone who dis", "String", espIP, espPort)
    pause(5)
    
end

} 



*/