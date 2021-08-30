#include "WiFiEsp.h"
// Emulate Serial1 on pins 6/7 if not present
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(6, 7); // RX, TX
#endif
int field_data;
char ssid[] = "Bazzinga";    // your network SSID (name)
char pass[] = "12345678999"; // your network password
int status = WL_IDLE_STATUS; // the Wifi radio's status

char server[] = "api.thingspeak.com";
int counter = 0;

WiFiEspServer server1(80);
WiFiEspClient client;
boolean lock = false;
boolean lock1 = false;
boolean lock2 = true;
void setup()
{
  pinMode(10, INPUT); //PIR sensor1
  pinMode(11, INPUT); //PIR sensor2
  pinMode(3, OUTPUT); //Light through Relay
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(12, OUTPUT);

  digitalWrite(3, 1);
  digitalWrite(8, 1);
  digitalWrite(9, 1);
  digitalWrite(12, 1);

  Serial.begin(9600);

  Serial1.begin(9600);

  WiFi.init(&Serial1);
  WiFi.config(IPAddress(192, 168, 43, 230));

  if (WiFi.status() == WL_NO_SHIELD)
  {
    Serial.println("WiFi shield not present");

    while (true)
      ;
  }

  while (status != WL_CONNECTED)
  {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);

    status = WiFi.begin(ssid, pass);
  }

  // you're connected now, so print out the data
  Serial.println("You're connected to the network");

  sendData("AT+CIPMUX=1\r\n", 1000, true); // configure for multiple connections

  server1.begin();
}

void loop()
{
  serverconnect();
  if (lock == true)
  {
    clientconnect(counter, field_data);
    lock = false;
  }
  int data = digitalRead(10);  //PIR outside room
  int data1 = digitalRead(11); //PIR inside room

  if (data == HIGH && lock2 == true)
  {
    lock2 = false;
    Serial.println("Sensor 1(IN)");
    counter++;
    if (counter > 0)
    {
      digitalWrite(8, 0);
      digitalWrite(3, 0);
      clientconnect(counter, 9);
    }
    delay(4000);
    lock2 = true;
  }

  if (data1 == HIGH && lock2 == true && counter > 0)
  {
    lock2 = false;
    Serial.println("Sensor 2(OUT)");
    counter--;
    clientconnect(counter, 8);

    if (counter == 0)
    {
      digitalWrite(8, 1);
      digitalWrite(3, 1);
      clientconnect(counter, 10);
    }
    delay(4000);
    lock2 = true;
  }
}
void clientconnect(int no, int field)
{
  lock1 = true;
  String n = String(no);
  String content;
  if (field == 8)
  {
    content = "&field1=" + n + "&field2=0";
    Serial.println(content);
  }
  if (field == 0)
  {
    content = "&field1=" + n + "&field2=0";
  }
  if (field == 1)
  {
    content = "&field1=" + n + "&field2=1";
  }
  if (field == 2)
  {
    content = "&field1=" + n + "&field3=0";
  }
  if (field == 3)
  {
    content = "&field1=" + n + "&field3=1";
  }
  if (field == 4)
  {
    content = "&field1=" + n + "&field4=0";
  }
  if (field == 5)
  {
    content = "&field1=" + n + "&field4=1";
  }
  if (field == 6)
  {
    content = "&field1=" + n + "&field5=0";
  }
  if (field == 7)
  {
    content = "&field1=" + n + "&field5=1";
  }
  if (field == 9)
  {
    content = "&field1=" + n + "&field2=1&field3=1";
  }
  if (field == 10)
  {
    content = "&field1=" + n + "&field2=0&field3=0";
  }
  Serial.println("Starting connection to server...");

  if (client.connect(server, 80))
  {
    Serial.println("Connected to server");

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: QYGJ1PANZQPITA5Z\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(content.length());
    client.print("\n\n");
    client.print(content);
  }
  client.stop();
  lock1 = false;
}
void serverconnect()
{
  WiFiEspClient client1 = server1.available();
  String c = "";
  if (lock1 == false)
  {
    if (client1)
    {
      Serial.println("New client");

      while (client1.connected())
      {
        if (client1.available())
        {
          c = client1.readStringUntil(0X0D);
          Serial.println(c);

          if (c.indexOf("/?r1=0") != -1)
          {
            field_data = 0;
            digitalWrite(3, 0);
            Serial.println("Room 1 bulb 1 ON");
          }
          if (c.indexOf("/?r1=1") != -1)
          {
            field_data = 1;
            digitalWrite(3, 1);
            Serial.println("Room 1 bulb 1 OFF");
          }
          if (c.indexOf("/?r1=2") != -1)
          {
            field_data = 2;
            digitalWrite(8, 0);
            Serial.println("Room 1 bulb 2 ON");
          }
          if (c.indexOf("/?r1=3") != -1)
          {
            field_data = 3;
            digitalWrite(8, 1);
            Serial.println("Room 1 bulb 2 OFF");
          }
          if (c.indexOf("/?r1=4") != -1)
          {
            field_data = 4;
            digitalWrite(9, 0);
            Serial.println("Room 2 bulb 1 ON");
          }
          if (c.indexOf("/?r1=5") != -1)
          {
            field_data = 5;
            digitalWrite(9, 1);
            Serial.println("Room 2 bulb 1 OFF");
          }
          if (c.indexOf("/?r1=6") != -1)
          {
            field_data = 6;
            digitalWrite(12, 0);
            Serial.println("Room 2 bulb 2 ON");
          }
          if (c.indexOf("/?r1=7") != -1)
          {
            field_data = 7;
            digitalWrite(12, 1);
            Serial.println("Room 2 bulb 2 OFF");
          }
          client1.print(
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: text/html\r\n"
              "Connection: close\r\n" // the connection will be closed after completion of the response
              "\r\n");
          break;
        }
      }

      delay(10);

      client1.stop();
      Serial.println("Client disconnected " + String(field_data));
      lock = true;
    }
  }
}
String sendData(String command, const int timeout, boolean debug)
{
  String response = "";

  Serial1.print(command); // send the read character to the esp8266

  long int time = millis();

  while ((time + timeout) > millis())
  {
    while (Serial1.available())
    {

      // The esp has data so display its output to the serial window
      char c = Serial1.read(); // read the next character.
      response += c;
    }
  }

  if (debug)
  {
    Serial.print(response);
  }

  return response;
}
