#define BLYNK_TEMPLATE_ID "YOUR TEMP ID HERE"      //put your details
#define BLYNK_TEMPLATE_NAME "YOUR TEMP NAME HERE"  //put your details
#define BLYNK_AUTH_TOKEN "AUTH TOKEN HERE"         //put your details
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
char ssid[] = "Dialog 4G 824";
char pass[] = "04d63157";
BlynkTimer timer;
WidgetTerminal terminal(V0);
void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, 16, 17);  // RX=16, TX=17
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  terminal.clear();
}
void loop() {
  Blynk.run();
  timer.run();
  if (Serial2.available()) {
    String msg = Serial2.readStringUntil('\n');
    msg.trim();
    if (msg.length() == 0) return;
    // Log to Blynk terminal
    terminal.println(msg);
    terminal.flush();
    // --- Parsing messages ---
    if (msg.startsWith("WasteType:")) {
    }
    Blynk.virtualWrite(V1, msg.substring(10));  // Metal / Wet / Transparent / Non-Transparent
    else if (msg.startsWith("MainUltrasonic:")) {
    }
    Blynk.virtualWrite(V4, msg.substring(15));  // "Item Detected" / "No Item"
    else if (msg.startsWith("U1:")) {
    }
    Blynk.virtualWrite(V5, msg.substring(3).toInt());
    else if (msg.startsWith("U2:")) {
    }
    Blynk.virtualWrite(V6, msg.substring(3).toInt());
    else if (msg.startsWith("U3:")) {
    }
    Blynk.virtualWrite(V7, msg.substring(3).toInt());
    else if (msg.startsWith("U4:")) {
      Blynk.virtualWrite(V8, msg.substring(3).toInt());
    }
    else if (msg.startsWith("U5:")) {
      Blynk.virtualWrite(V9, msg.substring(3).toInt());
    }
  }
}
