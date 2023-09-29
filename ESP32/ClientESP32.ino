#include <WiFi.h>
#include <WiFiClientSecure.h>

const char* ssid = "";
const char* password = "";
const char* serverName = "Z800"; // needs to be hostname
const int serverPort = 12345;  

// Set your NTP settings here
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

// CA certificate in PEM format (replace with the content of ca.crt )
// It is important to note that the domain we connect to, should be the same(CN) as when generating certificates
// GOOD TO OBFUSCATE OR IDEAL TO DOWNLOAD FROM A HTTPS TO PREVENT UPDATING CODE WHEN CERTS RENEW
const char* rootCACertificate = R"EOF(
-----BEGIN CERTIFICATE-----
PUT_YOUR_CA.CRT_HERE
-----END CERTIFICATE-----
)EOF";


WiFiClientSecure client;

void setup() {

  // Connect to Wi-Fi
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  tm local_tm;
    if (!getLocalTime(&local_tm)) {
      Serial.println("Failed to obtain time");
      return;
    }
    // Get epoch
    time_t timeSinceEpoch = mktime(&local_tm);
    
  //client.setInsecure();
  client.setCACert(rootCACertificate);
}

void loop() {
  if (!client.connected()) {
    Serial.println("Connecting to server...");

    if (client.connect(serverName, serverPort)) {
      Serial.println("Connected to server");

      // Send data to the server
      client.println("Hello, server!");
      client.println();

      // Read response from the server
      while (client.available()) {
        String line = client.readStringUntil('\n');
        Serial.println("GOT:"+line);
      }
      client.stop();
    } else {
      Serial.println("Connection failed.");
    }

    delay(5000);  // Wait for a while before reconnecting
  }
}
