# ESP-IDF Introduction

- ESP datasheet: https://www.alldatasheet.com/html-pdf/1148023/ESPRESSIF/ESP32/1706/3/ESP32.html
- ESP32 reference manual: https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf
- ESP32 WROOM datasheet: https://documentation.espressif.com/esp32-wroom-32_datasheet_en.pdf

PS C:\Espressif\frameworks\esp-idf-v5.5.2> cd C:\Embedded\ESP\ESP32_Workspace\blink
PS C:\Embedded\ESP\ESP32_Workspace\blink> idf.py menuconfig
PS C:\Embedded\ESP\ESP32_Workspace\blink> idf.py build
PS C:\Embedded\ESP\ESP32_Workspace\blink> idf.py -p COM8 flash monitor
press boot button on the board while flashing