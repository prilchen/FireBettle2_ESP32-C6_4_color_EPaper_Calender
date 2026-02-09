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

const String LAT = "50.91";
const String LON = "6.81";

#define MY_WIDTH  200
#define MY_HEIGHT 200

// Hilfsfunktion zur Zentrierung von Text
// FontWidth: Font8=5, Font12=7, Font16=11, Font20=14, Font24=17
int getCenterX(const char* text, int fontWidth) {
    int len = strlen(text);
    int x = (MY_WIDTH - (len * fontWidth)) / 2;
    return (x < 0) ? 0 : x;
}

String getDayName(int year, int month, int day) {
    const char* days[] = {"Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag"};
    struct tm timeinfo = {0};
    timeinfo.tm_year = year - 1900;
    timeinfo.tm_mon = month - 1;
    timeinfo.tm_mday = day;
    mktime(&timeinfo);
    return days[timeinfo.tm_wday];
}

int getISOWeek(int year, int month, int day) {
    struct tm timeinfo = {0};
    timeinfo.tm_year = year - 1900;
    timeinfo.tm_mon = month - 1;
    timeinfo.tm_mday = day;
    mktime(&timeinfo);
    return (timeinfo.tm_yday / 7) + 1;
}

void fetchWeatherAndDisplay() {
    HTTPClient http;
    String url = "https://api.open-meteo.com/v1/forecast?latitude=" + LAT + "&longitude=" + LON + 
                 "&current_weather=true&daily=temperature_2m_max,temperature_2m_min,precipitation_probability_max&timezone=Europe%2FBerlin";
    
    if (http.begin(url)) {
        if (http.GET() == HTTP_CODE_OK) {
            JsonDocument doc;
            deserializeJson(doc, http.getString());

            float tempNow    = doc["current_weather"]["temperature"];
            const char* time = doc["current_weather"]["time"]; 
            float tempMax    = doc["daily"]["temperature_2m_max"][0];
            float tempMin    = doc["daily"]["temperature_2m_min"][0];
            int rainProb     = doc["daily"]["precipitation_probability_max"][0];

            int y, m, d;
            sscanf(time, "%d-%d-%d", &y, &m, &d);
            String dayName = getDayName(y, m, d);
            int kw = getISOWeek(y, m, d);

            EPD_1IN54G_Init();
            UWORD Imagesize = ((MY_WIDTH % 4 == 0)? (MY_WIDTH / 4 ): (MY_WIDTH / 4 + 1)) * MY_HEIGHT;
            UBYTE *ImageBuffer = (UBYTE *)malloc(Imagesize);
            if(ImageBuffer == NULL) return;

            Paint_NewImage(ImageBuffer, MY_WIDTH, MY_HEIGHT, 0, EPD_1IN54G_WHITE);
            Paint_SetScale(4);
            Paint_SelectImage(ImageBuffer);
            Paint_Clear(EPD_1IN54G_WHITE);

            // --- ZENTRIERTES ZEICHNEN ---
            
            // 1. Wochentag (Zentriert, Font20)
            Paint_DrawString_EN(getCenterX(dayName.c_str(), 14), 5, dayName.c_str(), &Font20, EPD_1IN54G_WHITE, EPD_1IN54G_BLACK);
            
            // 2. Datum & KW (Zentriert, Font16 - jetzt größer)
            char dateBuf[20];
            sprintf(dateBuf, "%02d.%02d.%d (KW%d)", d, m, y, kw);
            Paint_DrawString_EN(getCenterX(dateBuf, 11), 30, dateBuf, &Font16, EPD_1IN54G_WHITE, EPD_1IN54G_BLACK);
            
            // 3. Aktuelle Temperatur (Zentriert, Font24, ROT)
            char tempBuf[15];
            sprintf(tempBuf, "%.1f C", tempNow);
            Paint_DrawString_EN(getCenterX(tempBuf, 17), 55, tempBuf, &Font24, EPD_1IN54G_WHITE, EPD_1IN54G_RED);

            // 4. Max/Min (Zentriert, Font12)
            char mmBuf[30];
            sprintf(mmBuf, "Max: %.1f C | Min: %.1f C", tempMax, tempMin);
            Paint_DrawString_EN(getCenterX(mmBuf, 7), 90, mmBuf, &Font12, EPD_1IN54G_WHITE, EPD_1IN54G_BLACK);
            
            // 5. Regenwahrscheinlichkeit (Zentriert, Font16)
            char rainBuf[20];
            sprintf(rainBuf, "Regen: %d%%", rainProb);
            Paint_DrawString_EN(getCenterX(rainBuf, 11), 110, rainBuf, &Font16, EPD_1IN54G_WHITE, EPD_1IN54G_BLACK);

  // 6. PRILCHEN.DE mit gelber Umrandung
const char* logo = "PRILCHEN.DE";
int logoWidth = strlen(logo) * 14; // Font20 ist 14 Pixel breit
int logoX = (MY_WIDTH - logoWidth) / 2;

// Gelber Balken als Hintergrund (etwas breiter als der Text)
Paint_DrawRectangle(logoX - 4, 153, logoX + logoWidth + 4, 182, EPD_1IN54G_YELLOW, DOT_PIXEL_1X1, DRAW_FILL_FULL);

// Schwarzer Text direkt auf das gelbe Rechteck setzen
Paint_DrawString_EN(logoX, 158, logo, &Font20, EPD_1IN54G_BLACK, EPD_1IN54G_YELLOW);

            EPD_1IN54G_Display(ImageBuffer);
            EPD_1IN54G_Sleep();
            free(ImageBuffer); 
        }
        http.end();
    }
}

void setup() {
    Serial.begin(115200);
    DEV_Module_Init();
    WiFi.begin(ssid, password);
    int timeout = 0;
    while (WiFi.status() != WL_CONNECTED && timeout < 20) { delay(500); timeout++; }
    if (WiFi.status() == WL_CONNECTED) fetchWeatherAndDisplay();
    esp_sleep_enable_timer_wakeup(3600ULL * 1000000ULL);
    esp_deep_sleep_start();
}

void loop() {}