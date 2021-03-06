#include <Arduino.h>
#include <U8x8lib.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif

U8X8_SSD1306_64X48_ER_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);   // EastRising 0.66" OLED breakout board, Uno: A4=SDA, A5=SCL, 5V powered

const float EUR_spent = ;
const float BTC_owned = ;

const char* ssid = ".......";
const char* password = ".......";
const int httpsPort = 443;
const char* host = "api.kraken.com";
const char * url = "/0/public/Ticker";
const char* post_data = "pair=XXBTZEUR\r\n";

// Use web browser to view and copy SHA1 fingerprint of the certificate.
const char* fingerprint = "E6:DD:9B:E8:9E:2A:29:80:B0:E8:67:28:46:1C:90:08:3D:2A:AE:FF";

const int sleepTimeS = 60;

void setup(void)
{ 
  /* U8g2 Project: KS0108 Test Board */
  //pinMode(16, OUTPUT);
  //digitalWrite(16, 0);	
   
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

  if (client.verify(fingerprint, host))
    Serial.println("certificate matches");
  else
    Serial.println("certificate doesn't match");

  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: BTC_EUR_Ticker_ESP8266\r\n" +
               "Content-Type: application/x-www-form-urlencoded\r\n" + 
               "Content-Length: 13\r\n" + 
               "\r\n" +
               post_data + 
               "Connection: close\r\n\r\n");

  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  
  String contentLength = client.readStringUntil('\n');
  String payload = client.readStringUntil('\n');
  
  Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(payload);
  Serial.println("==========");

  DynamicJsonBuffer jsonBuffer(contentLength.toInt());
  JsonObject& root = jsonBuffer.parseObject(payload);

  if (!root.success())
    Serial.println("Payload parsing failed");
  else
    Serial.println("Correctly parsed the payload");

  const char * price_str = root["result"]["XXBTZEUR"]["c"][0];
  const char * volume_weighted_average_str = root["result"]["XXBTZEUR"]["p"][0];
  const char * opening_price_str = root["result"]["XXBTZEUR"]["o"];

  float price = ((String)price_str).toFloat();
  float opening_price = ((String)opening_price_str).toFloat();
  float daily_change = 100*(price - opening_price)/opening_price;

  char daily_change_str[9];
  dtostrf(abs(daily_change), 4, 1, daily_change_str);

  Serial.println(price);
  Serial.println(opening_price);
  Serial.println(daily_change_str);
  
  u8x8.begin();
  //u8x8.initDisplay();
  //u8x8.setPowerSave(0);
  u8x8.setContrast(30);

  u8x8.setFont(u8x8_font_chroma48medium8_r);
  
  u8x8.drawString(0,0,"  BTC");
  u8x8.drawString(0,1,price_str); u8x8.drawString(7,1," ");
  u8x8.drawString(1,2,daily_change_str); 
  // This is to force a plus sign and to always have them at the beggining 
  // (dtostrf only adds minus signs and their position changes with str width)
  if(daily_change < 0) 
    u8x8.drawString(0,2, "-");
  else 
    u8x8.drawString(0,2, "+");
  u8x8.drawString(5,2, "%");

  char profits_str[9];
  float profits = BTC_owned*price - EUR_spent;  
  dtostrf(profits, 6, 1, profits_str);

  Serial.println("Profits (EUR)");
  Serial.println(profits);

  u8x8.drawString(0,4, "Profit:"); 
  u8x8.drawUTF8(0,5, profits_str); u8x8.drawString(6,5, "E");
  
  Serial.println("ESP8266 going to sleep mode");
  // Must connect D0 to RST. When it wakes up, the device resets and memory is lost.
  ESP.deepSleep(sleepTimeS * 1000000);
}

void loop(void)
{
  
}
