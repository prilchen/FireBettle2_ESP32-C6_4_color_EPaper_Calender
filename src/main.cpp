#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "EPD_1in54g.h" 
#include "GUI_Paint.h"
#include "DEV_Config.h"
#include <secrets.h>

// WLAN Daten
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASS;

// Konstante Werte für das Display
#define MY_WIDTH  200
#define MY_HEIGHT 200

void fetchWeatherAndDisplay() {
    // 1. Wetterdaten holen (Open-Meteo)
    HTTPClient http;
    String url = "https://api.open-meteo.com/v1/forecast?latitude=50.91&longitude=6.81&current_weather=true";
    float temp = 0;

    if (http.begin(url)) {
        if (http.GET() == HTTP_CODE_OK) {
            JsonDocument doc;
            deserializeJson(doc, http.getString());
            temp = doc["current_weather"]["temperature"];
        }
        http.end();
    }

    // 2. Display vorbereiten
    EPD_1IN54G_Init();
    
    // Budget berechnen: 10000 Bytes (200 * 200 / 4)
    UWORD Imagesize = ((MY_WIDTH % 4 == 0)? (MY_WIDTH / 4 ): (MY_WIDTH / 4 + 1)) * MY_HEIGHT;
    UBYTE *ImageBuffer = (UBYTE *)malloc(Imagesize);

    if(ImageBuffer == NULL) {
        Serial.println("Fehler: RAM voll!");
        return;
    }

    // 3. Zeichnen wie im funktionierenden Sketch
    Paint_NewImage(ImageBuffer, MY_WIDTH, MY_HEIGHT, 0, EPD_1IN54G_WHITE);
    Paint_SetScale(4); // DER ENTSCHEIDENDE BEFEHL
    Paint_SelectImage(ImageBuffer);
    Paint_Clear(EPD_1IN54G_WHITE);

    // Wetter & Kalender Layout
    Paint_DrawString_EN(10, 10, "FRECHEN", &Font20, 1, 0); // Schwarz auf Weiß
    
    char temp_buf[10];
    sprintf(temp_buf, "%.1f C", temp);
    Paint_DrawString_EN(10, 50, temp_buf, &Font24, 1, 3); // Rot (Farbe 3)
    
    Paint_DrawRectangle(10, 100, 190, 150, 2, DOT_PIXEL_1X1, DRAW_FILL_FULL); // Gelber Balken (Farbe 2)
    Paint_DrawString_EN(20, 115, "Kalender-Info", &Font16, 2, 0);

    // 4. Senden und schlafen
    Serial.println("Update Display...");
    EPD_1IN54G_Display(ImageBuffer);
    EPD_1IN54G_Sleep();
    
    free(ImageBuffer); // Speicher wieder freigeben!
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    // Hardware initialisieren (Achtung: Prüfe deine DEV_Config.h Pins!)
    DEV_Module_Init();
    
    // WLAN Verbindung
    WiFi.begin(ssid, password);
Serial.println("Verbinde mit WLAN...");
int counter = 0;
while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    counter++;
    if(counter > 20) { // Nach 10 Sekunden abbrechen
        Serial.println("\nWLAN Fehler: Zeitüberschreitung!");
        break; 
    }
}
if(WiFi.status() == WL_CONNECTED) {
    Serial.println("\nErfolgreich verbunden!");
    fetchWeatherAndDisplay();
}

    fetchWeatherAndDisplay();

    // Deep Sleep für 1 Stunde
    esp_sleep_enable_timer_wakeup(3600ULL * 1000000ULL);
    esp_deep_sleep_start();
}

void loop() {
    // Bleibt leer wegen Deep Sleep
}