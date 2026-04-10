#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// ===== TUJHE WiFi DETAILS ITHE TAAK =====
const char* ssid = "Redmi";
const char* password = "0099009900";

// ===== MQTT Broker (Free Public Broker - HiveMQ) =====
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* mqtt_topic_humidity = "bp8266/sensor/humidity";
const char* mqtt_topic_temperature = "bp8266/sensor/temperature";

// ===== DHT Sensor Setup =====
#define DHTPIN D4       // DHT sensor DATA pin -> D4
#define DHTTYPE DHT11   // DHT11 use karat aslya tar DHT11, DHT22 aslya tar DHT22 lih

// ===== Built-in LED (Data aala tar blink hoil) =====
#define LED_PIN LED_BUILTIN  // NodeMCU built-in LED (D0/GPIO16)

DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastReadTime = 0;
const long readInterval = 5000; // 5 second madhe ek reading

void connectWiFi() {
    Serial.print("WiFi la connect hot aahe: ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    Serial.print("WiFi connected! IP: ");
    Serial.println(WiFi.localIP());
}

void connectMQTT() {
    while (!client.connected()) {
        Serial.print("MQTT la connect hot aahe...");
        // Random client ID generate kar
        String clientId = "ESP8266-" + String(random(0xffff), HEX);

        if (client.connect(clientId.c_str())) {
            Serial.println("MQTT connected!");
        } else {
            Serial.print("Failed, rc=");
            Serial.print(client.state());
            Serial.println(" 5 sec madhe retry karto...");
            delay(5000);
        }
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println("=== Humidity Sensor + MQTT Project ===");

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH); // LED off (NodeMCU LED is active LOW)

    dht.begin();
    connectWiFi();
    client.setServer(mqtt_server, mqtt_port);
}

void loop() {
    // MQTT connection check
    if (!client.connected()) {
        connectMQTT();
    }
    client.loop();

    // Sensor reading every 5 seconds
    unsigned long now = millis();
    if (now - lastReadTime >= readInterval) {
        lastReadTime = now;

        float humidity = dht.readHumidity();
        float temperature = dht.readTemperature();

        // Check reading valid aahe ka
        if (isnan(humidity) || isnan(temperature)) {
            Serial.println("ERROR: Sensor varun reading aali nahi! Wiring check kar.");
            return;
        }

        // Data aala - LED blink kar!
        digitalWrite(LED_PIN, LOW);   // LED ON
        delay(200);
        digitalWrite(LED_PIN, HIGH);  // LED OFF

        // Serial Monitor var print kar
        Serial.print("Humidity: ");
        Serial.print(humidity);
        Serial.print("% | Temperature: ");
        Serial.print(temperature);
        Serial.println("°C");

        // MQTT var data pathav
        char humStr[8];
        char tempStr[8];
        dtostrf(humidity, 4, 2, humStr);
        dtostrf(temperature, 4, 2, tempStr);

        client.publish(mqtt_topic_humidity, humStr);
        client.publish(mqtt_topic_temperature, tempStr);

        Serial.println("MQTT var data pathavla!");
    }
}
