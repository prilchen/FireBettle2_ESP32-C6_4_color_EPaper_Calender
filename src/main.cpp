#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <secrets.h>

// E-Paper Library Dateien (müssen im /lib Ordner liegen)
#include "DEV_Config.h"
#include "EPD_1in54g.h"
#include "GUI_Paint.h"
#include "fonts.h"

// WLAN Daten
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASS;

// Koordinaten für Köln (kann angepasst werden)
const String LAT = "50.91";
const String LON = "6.81";

// Display Dimensionen
#define MY_WIDTH  200
#define MY_HEIGHT 200

void fetchWeatherAndDisplay() {
    HTTPClient http;
    // URL mit aktuellen Daten, Tageswerten (Min/Max/Regen) und lokaler Zeitzone
    String url = "https://api.open-meteo.com/v1/forecast?latitude=" + LAT + "&longitude=" + LON + 
                 "&current_weather=true&daily=temperature_2m_max,temperature_2m_min,precipitation_probability_max&timezone=Europe%2FBerlin";
    
    Serial.println("Hole Wetterdaten...");
    
    if (http.begin(url)) {
        int httpCode = http.GET();
        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            JsonDocument doc;
            deserializeJson(doc, payload);

            // Daten extrahieren
            float tempNow    = doc["current_weather"]["temperature"];
            int weatherCode  = doc["current_weather"]["weathercode"];
            const char* time = doc["current_weather"]["time"]; // Format "2026-02-08T10:00"
            float tempMax    = doc["daily"]["temperature_2m_max"][0];
            float tempMin    = doc["daily"]["temperature_2m_min"][0];
            int rainProb     = doc["daily"]["precipitation_probability_max"][0];

            // --- Display Vorbereitung ---
            EPD_1IN54G_Init();
            
            // Speicher reservieren (10.000 Bytes für 200x200 @ 2-Bit)
            UWORD Imagesize = ((MY_WIDTH % 4 == 0)? (MY_WIDTH / 4 ): (MY_WIDTH / 4 + 1)) * MY_HEIGHT;
            UBYTE *ImageBuffer = (UBYTE *)malloc(Imagesize);

            if(ImageBuffer == NULL) {
                Serial.println("Fehler: RAM voll!");
                return;
            }

            // Neues Bild initialisieren
            Paint_NewImage(ImageBuffer, MY_WIDTH, MY_HEIGHT, 0, EPD_1IN54G_WHITE);
            Paint_SetScale(4); // 4-Farben Modus aktivieren
            Paint_SelectImage(ImageBuffer);
            Paint_Clear(EPD_1IN54G_WHITE);

            // --- Zeichnen ---
            
            // 1. Kopfzeile: Datum (Extrahiert aus "2026-02-08T10:00")
            char dateBuf[15];
            // Formatierung zu DD.MM.YYYY
            sprintf(dateBuf, "%.2s.%.2s.%.4s", &time[8], &time[5], &time[0]);
            Paint_DrawString_EN(10, 5, dateBuf, &Font16, EPD_1IN54G_WHITE, EPD_1IN54G_BLACK);
            
            // 2. Aktuelle Temperatur groß in Rot
            char tempBuf[10];
            sprintf(tempBuf, "%.1f C", tempNow);
            Paint_DrawString_EN(10, 30, tempBuf, &Font24, EPD_1IN54G_WHITE, EPD_1IN54G_RED);

            // 3. Wetter-Details (Min/Max/Regen)
            char detailBuf[25];
            sprintf(detailBuf, "Max:%.1f Min:%.1f", tempMax, tempMin);
            Paint_DrawString_EN(10, 65, detailBuf, &Font12, EPD_1IN54G_WHITE, EPD_1IN54G_BLACK);
            
            char rainBuf[20];
            sprintf(rainBuf, "Regen: %d%%", rainProb);
            Paint_DrawString_EN(10, 85, rainBuf, &Font16, EPD_1IN54G_WHITE, EPD_1IN54G_YELLOW);

            // 4. Trennlinie und Info-Bereich
            Paint_DrawLine(0, 110, 200, 110, EPD_1IN54G_BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
            Paint_DrawString_EN(10, 120, "PRILCHEN.DE", &Font16, EPD_1IN54G_WHITE, EPD_1IN54G_BLACK);
            Paint_DrawString_EN(10, 145, "ESP32-C6 DeepSleep", &Font12, EPD_1IN54G_WHITE, EPD_1IN54G_BLACK);

            // 5. Übertragung an das Panel
            Serial.println("Sende Daten an E-Paper...");
            EPD_1IN54G_Display(ImageBuffer);
            
            // 6. Aufräumen
            EPD_1IN54G_Sleep();
            free(ImageBuffer); 
            Serial.println("Display-Update fertig, Speicher freigegeben.");
        } else {
            Serial.printf("HTTP Fehler: %d\n", httpCode);
        }
        http.end();
    }
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("\n--- FireBeetle 2 ESP32-C6 Wetterstation ---");

    // Hardware-Initialisierung (Pins aus DEV_Config.h)
    if (DEV_Module_Init() != 0) {
        Serial.println("GPIO Init Fehler!");
    }

    // WLAN Verbindung aufbauen
    WiFi.begin(ssid, password);
    Serial.print("Verbinde mit WLAN");
    
    int timeout = 0;
    while (WiFi.status() != WL_CONNECTED && timeout < 20) {
        delay(500);
        Serial.print(".");
        timeout++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nVerbunden!");
        fetchWeatherAndDisplay();
    } else {
        Serial.println("\nWLAN Zeitüberschreitung!");
    }

    // Deep Sleep für 1 Stunde (3600 Sekunden)
    Serial.println("Schlafmodus für 60 Min gestartet...");
    esp_sleep_enable_timer_wakeup(3600ULL * 1000000ULL);
    esp_deep_sleep_start();
}

void loop() {
    // Bleibt leer, da das Board nach setup() schläft und neu startet.
}