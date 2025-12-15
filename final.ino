#include <ESP32Servo.h>
#include <WiFi.h>

const char* ssid = "Palang Pintu";
const char* password = "palang123";
IPAddress local_IP(192, 168, 10, 10); 
IPAddress gateway(192, 168, 10, 1);   
IPAddress subnet(255, 255, 255, 0);  

WiFiServer server(80);

Servo gateServo;

int trigPin1 = 23;
int echoPin1 = 22;

int trigPin2 = 19;
int echoPin2 = 18;

int servoPin = 21;
int ledPin = 25;
int ldrPin = 34;

int lightThreshold = 500;

long duration1, duration2;
int distance1, distance2;

void setup() {
  Serial.begin(115200);

   WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.begin();

  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);

  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);

  pinMode(ledPin, OUTPUT);

  gateServo.attach(servoPin);
  gateServo.write(0);
}

int readUltrasonic(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);
  int distance = duration * 0.034 / 2;

  return distance;
}

void loop() {
  int ldrValue = analogRead(ldrPin);
  digitalWrite(ledPin, ldrValue > lightThreshold ? HIGH : LOW);

  distance1 = readUltrasonic(trigPin1, echoPin1);
  distance2 = readUltrasonic(trigPin2, echoPin2);

  if ((distance1 > 0 && distance1 <= 50) || 
      (distance2 > 0 && distance2 <= 50)) 
  {
    gateServo.write(90);
  } else {
    gateServo.write(0);
  }

  delay(150); 

  WiFiClient client = server.available();
  if (client) {
    String req = client.readStringUntil('\r');
    client.flush();

    if (req.indexOf("/OPEN") != -1) {
      gateServo.write(90);
      client.print("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\n");
      client.print("Gate Opened");
    } 
    else if (req.indexOf("/CLOSE") != -1) {
      gateServo.write(0);
      client.print("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\n");
      client.print("Gate Closed");
    }
    else if (req.indexOf("GET / ") != -1) {
     
      client.print("HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n");
      client.print("{");
      client.print("\"sensor1\":");
      client.print(distance1);
      client.print(",");
      client.print("\"sensor2\":");
      client.print(distance2);
      client.print(",");
      client.print("\"ldr\":");
      client.print(analogRead(ldrPin));
      client.print("}");
    }
    
    else {
      client.print("HTTP/1.1 404 Not Found\r\nConnection: close\r\n\r\n");
    }

    delay(1);
    client.stop();
  }
}
