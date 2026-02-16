# FireBeetle2 E-Paper Weather & Calendar Display

Ein smartes Informationsdisplay auf Basis eines **FireBeetle2**, das Wetterdaten der Open-Meteo API mit einem lokalen Kalender kombiniert und auf einem 4.2" E-Paper Display anzeigt.

![Wetter Kalender Projekt](https://prilchen.de/wp-content/uploads/2026/01/image-26-768x526.png)

## Features

* **Live Wetterdaten**: Stündliche Abfrage von Temperatur (Aktuell, Min/Max) und Regenwahrscheinlichkeit via Open-Meteo API (ohne API-Key, erweiterbar)
* **Präzise Zeit**: Synchronisation über NTP (Network Time Protocol) zur Anzeige von Datum, Wochentag und Kalenderwoche.
* **E-Paper Technologie**: Stromsparende Anzeige, die das Bild auch ohne Stromversorgung behält.

## Hardware

* **Mikrocontroller**: FireBeetle2 ESP32-C6
* **Display**: Waveshare 1,54 Zoll großes E-Paper (E-Ink) Modul
* **LiPo Akku**: Optional Lithium Polymer Akku 3,7 V 2000mAh
* **Entwicklungsumgebung**: Visual Studio Code mit PlatformIO

### Pin-Belegung (ESP32 WROOM-32)

| E-Paper Pin | ESP32 Pin |
| --- | --- |
| **VCC** | **3,3V** |
| **GND** | **GND** |
| **DIN** (MOSI) | **GPIO 22** |
| **CLK** (SCK) | **GPIO 21** |
| **CS** | **GPIO 8** |
| **DC** | **GPIO 14** |
| **RST** | **GPIO 1** |
| **BUSY** | **GPIO 18** |

## Software-Setup

1. **Bibliotheken**: Folgende Libraries müssen in der `platformio.ini` vorhanden sein:
* `bblanchon/ArduinoJson`
*  Die lokale Hersteller EPD-Library wird automatisch gefunden!

2. **Konfiguration**:
* Trage deine WLAN-Zugangsdaten in der `main.cpp` ein.
* Passe die Koordinaten (`latitude` & `longitude`) in der `weatherUrl` an deinen Standort an.

## Über prilchen.de

Dieses Projekt ist Teil meiner Serie für technisch interessierte Bastler und Tüftler.

* **Webseite**: [prilchen.de](https://prilchen.de/arduino-projekt-ein-4-farben-e-paper-am-arduino-r4-wifi/)
* **YouTube**: [@prilchen](https://www.youtube.com/@prilchen)
* **Standort**: Köln, Deutschland

---

### Lizenz

Dieses Projekt ist unter der MIT-Lizenz veröffentlicht.
