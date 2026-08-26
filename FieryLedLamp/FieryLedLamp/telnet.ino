// ***************************************************************************** telnet.ino *************************************************************
#if TELNET_LOG

WiFiServer telnetServer(23);
WiFiClient telnetClient;
bool telnetGreetingShown = false;

void initTelnet() {
  telnetServer.begin();
  telnetServer.setNoDelay(true);
  SYSLOG.add("Telnet server started on port 23");
}

void handleTelnetClient() {
  if (telnetServer.hasClient()) {
    if (!telnetClient || !telnetClient.connected()) {
      if (telnetClient) {
        telnetClient.stop();
        telnetGreetingShown = false;
      }
      telnetClient = telnetServer.available();
      if (telnetClient && telnetClient.connected()) {
        telnetClient.println("\n=========================================");
        telnetClient.println("Connected to FieryLedLamp");
        telnetClient.println("IP: " + Wifi::instance().localIP().toString());
        telnetClient.println("=========================================\n");
        telnetGreetingShown = true;

        SYSLOG.add("Telnet client connected");
      }
    } else {
      telnetServer.available().stop();
    }
  } // if (telnetServer.hasClient())

  if (telnetClient && telnetClient.connected() && telnetClient.available()) {
    String cmd = telnetClient.readStringUntil('\n');
    cmd.trim();
    if (cmd.length() > 0) {
      SYSLOG.add("Telnet command: %s", cmd.c_str());

      if (cmd == "help" || cmd == "?") {
        telnetClient.println("\nAvailable commands:");
        telnetClient.println(" help - show this help");
        telnetClient.println(" status - show device status");
        telnetClient.println(" reboot - restart device");
        telnetClient.println(" ota - activate OTA mode");
        telnetClient.println(" quit - disconnect");
        telnetClient.println();
      }
      else if (cmd == "status") {
        auto& wifi = Wifi::instance();
        telnetClient.printf("\nDevice: %s\n", LAMP_NAME.c_str());
        telnetClient.printf("IP: %s\n", wifi.localIP().toString().c_str());
        telnetClient.printf("WiFi: %s\n", wifi.isConnected() ? "Connected" : "Disconnected");
        telnetClient.printf("RSSI: %d dBm\n", wifi.getRSSI());
        telnetClient.printf("Free heap: %u bytes\n", ESP.getFreeHeap());
        telnetClient.printf("Uptime: %lu seconds\n", millis() / 1000);
        telnetClient.println();
      }
      else if (cmd == "reboot") {
        telnetClient.println("Rebooting in 2 seconds...");
        delay(2000);
        ESP.restart();
      }
      else if (cmd == "ota") {
        telnetClient.println("Activating OTA mode...");
        if (Ota.RequestOtaUpdate()) {
          telnetClient.println("OTA mode activated successfully!");
        } else {
          telnetClient.println("Failed to activate OTA mode");
        }
      }
      else if (cmd == "quit" || cmd == "exit") {
        telnetClient.println("Disconnecting...");
        telnetClient.stop();
        telnetGreetingShown = false;
      }
      else if (cmd.length() > 0) {
        telnetClient.printf("Unknown command: %s. Type 'help' for available commands.\n", cmd.c_str());
      }
    }
  }
} // void handleTelnetClient()

void telnetPrint(const String& msg) {
  if (telnetClient && telnetClient.connected()) {
    telnetClient.print(msg);
  }
  Serial.print(msg);
}

void telnetPrintln(const String& msg) {
  if (telnetClient && telnetClient.connected()) {
    telnetClient.println(msg);
  }
  Serial.println(msg);
}

#endif // TELNET_LOG

// ******************************************************************************************************************************************************
