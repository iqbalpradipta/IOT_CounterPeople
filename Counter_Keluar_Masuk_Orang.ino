#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <time.h>
#include <ThingSpeak.h>

const char* ssid = "YOUR WIFI";
const char* password = "YOUR PASSWORD";

unsigned long myChannelNumber = "CHANNEL NUMBER";
const char * myWriteAPIKey = "API KEY";

const int sensorPin1 = 32;
const int sensorPin2 = 33;

volatile int peopleIn = 0;
volatile int peopleOut = 0;
int peopleInside = 0;

int lastPeopleIn = 0;   
int lastPeopleOut = 0;  

int lastSensorState1 = HIGH;
int lastSensorState2 = HIGH;

unsigned long lastSensor1TriggerTime = 0;
unsigned long lastSensor2TriggerTime = 0;
const unsigned long debounceDelay = 200;

unsigned long lastThingSpeakUpdateTime = 0;
const unsigned long thingSpeakMinInterval = 18000; 

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 7 * 3600);

char formattedDate[12];
char formattedTime[9];

WiFiClient client;

void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.println("===================================");
  Serial.println("   IoT People Counter - ESP32    ");
  Serial.println("   (Mode Independent Counter)    ");
  Serial.println("   (ThingSpeak Update on Change) "); 
  Serial.println("===================================");

  Serial.print("Menghubungkan ke WiFi ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  int connectAttempts = 0;
  while (WiFi.status() != WL_CONNECTED && connectAttempts < 30) {
    delay(500);
    Serial.print(".");
    connectAttempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Terhubung!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    timeClient.begin();
    Serial.println("Mendapatkan waktu dari NTP...");
    while (!timeClient.update()) {
      timeClient.forceUpdate();
      delay(500);
      Serial.print(".");
    }
    Serial.println("\nWaktu berhasil disinkronkan!");
    configTime(7 * 3600, 0, "pool.ntp.org", "time.nist.gov");

    ThingSpeak.begin(client);
  } else {
    Serial.println("\nGagal terhubung ke WiFi. Waktu tidak akan realtime dan data tidak terkirim ke ThingSpeak.");
    Serial.println("Pastikan SSID dan Password WiFi benar, dan ESP32 dalam jangkauan.");
  }

  Serial.println("Waiting for people to pass...");

  pinMode(sensorPin1, INPUT);
  pinMode(sensorPin2, INPUT);
}

void loop() {
  timeClient.update();

  time_t epochTime = timeClient.getEpochTime();
  struct tm *timeinfo;
  timeinfo = localtime(&epochTime);

  strftime(formattedDate, sizeof(formattedDate), "%d/%m/%Y", timeinfo);
  strftime(formattedTime, sizeof(formattedTime), "%H:%M:%S", timeinfo);

  int currentSensorState1 = digitalRead(sensorPin1);
  int currentSensorState2 = digitalRead(sensorPin2);

  unsigned long currentTime = millis();

  bool dataChanged = false; // Flag untuk menandai apakah ada perubahan data yang perlu dikirim

  if (currentSensorState1 == LOW && lastSensorState1 == HIGH &&
      (currentTime - lastSensor1TriggerTime > debounceDelay)) {

    peopleIn++;
    peopleInside++;
    dataChanged = true; 
    
    Serial.print("MASUK - ");
    Serial.print(formattedDate);
    Serial.print(" ");
    Serial.print(formattedTime);
    Serial.print(" | Total Masuk: ");
    Serial.print(peopleIn);
    Serial.print(" | Total Keluar: ");
    Serial.print(peopleOut);
    Serial.print(" | TOTAL ORANG DI DALAM: ");
    Serial.println(peopleInside);
    lastSensor1TriggerTime = currentTime;
    delay(50);
  }

  if (currentSensorState2 == LOW && lastSensorState2 == HIGH &&
      (currentTime - lastSensor2TriggerTime > debounceDelay)) {

    peopleOut++;
    peopleInside--;
    if (peopleInside < 0) {
      peopleInside = 0;
    }
    dataChanged = true;

    Serial.print("KELUAR - ");
    Serial.print(formattedDate);
    Serial.print(" ");
    Serial.print(formattedTime);
    Serial.print(" | Total Masuk: ");
    Serial.print(peopleIn);
    Serial.print(" | Total Keluar: ");
    Serial.print(peopleOut);
    Serial.print(" | TOTAL ORANG DI DALAM: ");
    Serial.println(peopleInside);
    lastSensor2TriggerTime = currentTime;
    delay(50);
  }

  lastSensorState1 = currentSensorState1;
  lastSensorState2 = currentSensorState2;

  if (WiFi.status() == WL_CONNECTED && 
      (peopleIn != lastPeopleIn || peopleOut != lastPeopleOut) && 
      (currentTime - lastThingSpeakUpdateTime > thingSpeakMinInterval)) {
    
    Serial.println("Mengirim data ke ThingSpeak (perubahan terdeteksi)...");

    ThingSpeak.setField(1, peopleIn);
    ThingSpeak.setField(2, peopleOut);
    ThingSpeak.setField(3, peopleInside);

    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    if (x == 200) {
      Serial.println("Data berhasil terkirim ke ThingSpeak.");
      lastPeopleIn = peopleIn;   
      lastPeopleOut = peopleOut;
    } else {
      Serial.print("Gagal mengirim data ke ThingSpeak, HTTP error code ");
      Serial.println(x);
    }
    lastThingSpeakUpdateTime = currentTime;
  }

  delay(10);
}