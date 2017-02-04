#include <Arduino.h>
#include <U8x8lib.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif

U8X8_SSD1306_64X48_ER_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);   // EastRising 0.66" OLED breakout board, Uno: A4=SDA, A5=SCL, 5V powered


const char* ssid = ".......";
const char* password = ".......";

const char* host = "api.kraken.com";
const int httpsPort = 443;

// Use web browser to view and copy
// SHA1 fingerprint of the certificate
const char* fingerprint = "E6:DD:9B:E8:9E:2A:29:80:B0:E8:67:28:46:1C:90:08:3D:2A:AE:FF";

void setup(void)
{ 
  /* U8g2 Project: SSD1306 Test Board */
  //pinMode(10, OUTPUT);
  //pinMode(9, OUTPUT);
  //digitalWrite(10, 0);
  //digitalWrite(9, 0);		
  
  /* U8g2 Project: KS0108 Test Board */
  pinMode(16, OUTPUT);
  digitalWrite(16, 0);	
  
  u8x8.begin();
  u8x8.setPowerSave(0);
  
  /* WiFi */
  Serial.begin(115200);
  Serial.println();
  Serial.print("connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Use WiFiClientSecure class to create TLS connection
  WiFiClientSecure client;
  Serial.print("connecting to ");
  Serial.println(host);
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }

  if (client.verify(fingerprint, host)) {
    Serial.println("certificate matches");
  } else {
    Serial.println("certificate doesn't match");
  }

  String url = "/0/public/Ticker";
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: BTC_EUR_Ticker_ESP8266\r\n" +
               "Content-Type: application/x-www-form-urlencoded\r\n" + 
               "Content-Length: 13\r\n" + 
               "\r\n" +
               "pair=XXBTZEUR\r\n" + 
               "Connection: close\r\n\r\n");

  Serial.println("request sent");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  
  String contentLength = client.readStringUntil('\n');
  DynamicJsonBuffer jsonBuffer(300);
  String payload = client.readStringUntil('\n');
  JsonObject& root = jsonBuffer.parseObject(payload);

  if (!root.success())
    Serial.println("parseObject() failed");
  const char* price = root["result"]["XXBTZEUR"]["c"][0];
  
  Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(price);
  Serial.println("==========");
  Serial.println("closing connection");

  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.drawString(0,1,"BTC/EUR");
  u8x8.drawString(0,2,price);
  u8x8.refreshDisplay();
}

void loop(void)
{
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.drawString(0,1,"BTC/EUR");
  //u8x8.drawString(0,2,price);
  u8x8.refreshDisplay();		// for SSD1606  
  delay(1000);
  
  /*
  delay(1000);
  u8x8.setPowerSave(1);
  delay(1000);
  u8x8.setPowerSave(0);
  delay(1000);
  */
}
