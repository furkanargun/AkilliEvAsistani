#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#define lamba D0
#define priz D1
#define bluetooth D2
#define sensor D3
#define fan D5
#define ledler D6
#define sicaklikSensor A0

float sicaklikdegeri;
float olculendeger;
unsigned long eskiZaman=0;
unsigned long yeniZaman;
uint32_t otoFan = 0;
uint32_t x = 0;

#define WLAN_SSID       "Your_SSID"                     // Your SSID
#define WLAN_PASS       "Your_password"                 // Your password
/************************* Adafruit.io Setup *********************************/
#define AIO_SERVER      "io.adafruit.com"               //Adafruit Server
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "Username"                      // Username
#define AIO_KEY         "Auth_Key"                      // Auth Key

//WIFI CLIENT
WiFiClient client;

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

Adafruit_MQTT_Subscribe lambaK = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME"/feeds/lamba");
Adafruit_MQTT_Subscribe prizK = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/priz");
Adafruit_MQTT_Subscribe bluetoothK = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/bluetooth");
Adafruit_MQTT_Subscribe fanK = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/fan");
Adafruit_MQTT_Subscribe sensorK = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/sensor");
Adafruit_MQTT_Subscribe ledlerK = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/ledler");
Adafruit_MQTT_Publish feed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/sicaklik");
Adafruit_MQTT_Publish fanOtoK = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/fan");

void MQTT_connect();

void setup() {
  Serial.begin(115200); // Seri haberleşmeyi başlat
  // Kontrol pinlerini çıkış olarak ayarla 
  pinMode(lamba, OUTPUT);
  pinMode(priz, OUTPUT);
  pinMode(bluetooth, OUTPUT);
  pinMode(fan, OUTPUT);
  pinMode(sensor, OUTPUT);
  pinMode(ledler, OUTPUT);
  pinMode(A0, INPUT);  // Analog pin olarak ayarla
  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);
  
  // WiFi bağlantısını kur
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  // WiFi bağlantısı tamamlanana kadar bekleyin
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println();
  Serial.println("WiFi bağlandı");
  Serial.println("IP adresi: ");
  Serial.println(WiFi.localIP());

// MQTT feed'lerini subscribe et
  mqtt.subscribe(&lambaK);
  mqtt.subscribe(&prizK);
  mqtt.subscribe(&bluetoothK);
  mqtt.subscribe(&fanK);
  mqtt.subscribe(&sensorK);
  mqtt.subscribe(&ledlerK);
}

void loop() {
  MQTT_connect();  // MQTT bağlantısını kontrol et ve varsa yeniden bağlan
  Adafruit_MQTT_Subscribe *subscription;  // MQTT abonelik nesnesi

  // Sensör okuma sıklığını kontrol etmek için zamanlama işlemi
  yeniZaman = millis(); 
  /* bir önceki turdan itibaren 10 milisaniye geçmiş mi
  yani yeniZaman ile eskiZaman farkı 10 den büyük mü */
    if(yeniZaman-eskiZaman > 10 && otoFan == 1){
    // Sıcaklık sensöründen veri oku
      olculendeger = analogRead(sicaklikSensor);
      olculendeger = (olculendeger/1024)*3300;
      sicaklikdegeri = olculendeger / 10.0;
    // Sıcaklık verisini Adafruit.IO'ya gönder
      feed.publish(sicaklikdegeri);
      Serial.print(x++);
    
    // Sıcaklık 32°C'den yüksekse, fanı aç ve durumu Adafruit.IO'ya bildir
      if (sicaklikdegeri > 32) {
         fanOtoK.publish(1);
         digitalWrite(fan, 1);
         Serial.print(sicaklikdegeri);
      }
      else{
         fanOtoK.publish(0);
         digitalWrite(fan, 0);
         Serial.print(sicaklikdegeri);
        }
    } 
     /* Eski zaman değeri yeni zaman değeri ile güncelleniyor */
     eskiZaman = yeniZaman;
  
  // MQTT abonelikleri oku
  while ((subscription = mqtt.readSubscription(5000))) {
  // Lamba feed'i için gelen değeri oku ve lambayı kontrol et
    if (subscription == &lambaK) {
      Serial.print(F("Got: "));
      Serial.println((char *)lambaK.lastread);
      int lambaK_State = atoi((char *)lambaK.lastread);
      digitalWrite(lamba, lambaK_State);
    }
  // Priz feed'i için gelen değeri oku ve prizi kontrol et
    if (subscription == &prizK) {
      Serial.print(F("Got: "));
      Serial.println((char *)prizK.lastread);
      int prizK_State = atoi((char *)prizK.lastread);
      digitalWrite(priz, prizK_State);
    }
  // Bluetooth feed'i için gelen değeri oku ve bluetooth'u kontrol et
    if (subscription == &bluetoothK) {
      Serial.print(F("Got: "));
      Serial.println((char *)bluetoothK.lastread);
      int bluetoothK_State = atoi((char *)bluetoothK.lastread);
      digitalWrite(bluetooth, bluetoothK_State);
    }
  // Fan feed'i için gelen değeri oku ve fanı kontrol et
    if (subscription == &fanK) {
      Serial.print(F("Got: "));
      Serial.println((char *)fanK.lastread);
      int fanK_State = atoi((char *)fanK.lastread);
      digitalWrite(fan, fanK_State);
    }
  // Sensor feed'i için gelen değeri oku ve otomatik fan kontrolünü ayarla
    if (subscription == &sensorK) {
      Serial.print(F("Got: "));
      Serial.println((char *)sensorK.lastread);
      int sensorK_State = atoi((char *)sensorK.lastread);
      otoFan = sensorK_State;
      digitalWrite(sensor, sensorK_State);
    }
  // Ledler feed'i için gelen değeri oku ve ledleri kontrol et
    if (subscription == &ledlerK) {
      Serial.print(F("Got: "));
      Serial.println((char *)ledlerK.lastread);
      int ledlerK_State = atoi((char *)ledlerK.lastread);
      digitalWrite(ledler, ledlerK_State);
    }
  }
}

void MQTT_connect() {
  int8_t ret;
  
  // MQTT bağlantısı zaten varsa fonksiyonu sonlandır
  if (mqtt.connected()) {
    return;
  }
  Serial.print("MQTT'ye bağlanılıyor... ");
  
  // MQTT bağlantısını yap
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) {
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("MQTT bağlantısı 5 saniye içinde tekrar deneniyor...");
    mqtt.disconnect();
    delay(5000);
    retries--;
  // Bağlantı deneme sayısı sıfır olduğunda sonsuz döngüye gir
    if (retries == 0) {
      while (1);
    }
  }
  Serial.println("MQTT Bağlandı!");
}
