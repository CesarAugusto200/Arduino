#include "DHT.h" // Incluye la biblioteca DHT para leer sensores de humedad y temperatura DHT.
#include <Wire.h> // Incluye la biblioteca Wire para comunicaciones I2C.
#include <LiquidCrystal_I2C.h> // Incluye la biblioteca LiquidCrystal_I2C para controlar pantallas LCD.
#include <WiFi.h> // Incluye la biblioteca WiFi para conectarse a una red WiFi.
#include <HTTPClient.h> // Incluye la biblioteca HTTPClient para hacer solicitudes HTTP.
#include <ArduinoJson.h> // Incluye la biblioteca ArduinoJson para trabajar con datos JSON.

LiquidCrystal_I2C lcd(0x27, 20, 4); // Inicializa el LCD en la dirección I2C 0x27 y especifica que es de 20 columnas por 4 filas.

#define DHTPIN 4 
#define DHTTYPE DHT11 
#define LDRPIN A0 

const char* ssid = "Redmi Note 8"; 
const char* password = "hola12345678910"; 
const char* serverUrl = "http://192.168.155.141:8080/data-readings";



DHT dht(DHTPIN, DHTTYPE); 

void setup() {
  Serial.begin(9600); // Inicia la comunicación serial a 9600 baudios.
  lcd.init(); // Inicia el LCD.
  lcd.backlight(); // Enciende la luz de fondo del LCD.
  dht.begin(); // Inicia el sensor DHT.
  pinMode(LDRPIN, INPUT); // Configura el pin del sensor de luz como entrada.

  WiFi.begin(ssid, password); 

  while (WiFi.status() != WL_CONNECTED) { // Espera hasta que el módulo se conecte a la red WiFi.
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi"); 
}

void loop() {
  float h = dht.readHumidity(); // Lee la humedad desde el sensor DHT.
  float t = dht.readTemperature(); // Lee la temperatura desde el sensor DHT.
  int ldrStatus = analogRead(LDRPIN); // Lee el valor desde el sensor de luz.
  
  // Aquí se actualiza el LCD con la temperatura, la humedad y el estado de la luz.
  lcd.print("Temperature: ");
  lcd.print(t);
  lcd.print((char)223);
  lcd.setCursor(0, 1);
  lcd.print("humidity: ");
  lcd.print(h);
  lcd.setCursor(0, 2);
  lcd.print("Light: ");
  if(ldrStatus < 500) {
    lcd.print("Day");
  } else {
    lcd.print("Night");
  }

 
  if(WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");

   
    DynamicJsonDocument doc(1024);
    doc["humidity"] = h;
    doc["temperature"] = t;
    doc["dayNight"] = (ldrStatus < 500) ? "Night" : "Day";
    doc["isAptoForFarming"] = (h > 0.6 && t > 20);
    doc["location"] = "Chiapas"; 

    String httpRequestData;
    serializeJson(doc, httpRequestData);

    int httpResponseCode = http.POST(httpRequestData); 

    
    if(httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(httpResponseCode);
      Serial.println(response);
    } else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }

    http.end(); 
  }

  delay(2000); 
  lcd.clear(); 
}
