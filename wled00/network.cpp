#include "wled.h"
#include "fcn_declare.h"
#include "wled_ethernet.h"


#ifdef WLED_USE_ETHERNET
// The following six pins are neither configurable nor
// can they be re-assigned through IOMUX / GPIO matrix.
// See https://docs.espressif.com/projects/esp-idf/en/latest/esp32/hw-reference/esp32/get-started-ethernet-kit-v1.1.html#ip101gri-phy-interface
const managed_pin_type esp32_nonconfigurable_ethernet_pins[WLED_ETH_RSVD_PINS_COUNT] = {
    { 21, true  }, // RMII EMAC TX EN  == When high, clocks the data on TXD0 and TXD1 to transmitter
    { 19, true  }, // RMII EMAC TXD0   == First bit of transmitted data
    { 22, true  }, // RMII EMAC TXD1   == Second bit of transmitted data
    { 25, false }, // RMII EMAC RXD0   == First bit of received data
    { 26, false }, // RMII EMAC RXD1   == Second bit of received data
    { 27, true  }, // RMII EMAC CRS_DV == Carrier Sense and RX Data Valid
};

const ethernet_settings ethernetBoards[] = {
  // None
  {
  },

  // WT32-EHT01
  // Please note, from my testing only these pins work for LED outputs:
  //   IO2, IO4, IO12, IO14, IO15
  // These pins do not appear to work from my testing:
  //   IO35, IO36, IO39
  {
    1,                    // eth_address,
    16,                   // eth_power,
    23,                   // eth_mdc,
    18,                   // eth_mdio,
    ETH_PHY_LAN8720,      // eth_type,
    ETH_CLOCK_GPIO0_IN    // eth_clk_mode
  },

  // ESP32-POE
  {
     0,                   // eth_address,
    12,                   // eth_power,
    23,                   // eth_mdc,
    18,                   // eth_mdio,
    ETH_PHY_LAN8720,      // eth_type,
    ETH_CLOCK_GPIO17_OUT  // eth_clk_mode
  },

   // WESP32
  {
    0,			              // eth_address,
    -1,			              // eth_power,
    16,			              // eth_mdc,
    17,			              // eth_mdio,
    ETH_PHY_LAN8720,      // eth_type,
    ETH_CLOCK_GPIO0_IN	  // eth_clk_mode
  },

  // QuinLed-ESP32-Ethernet
  {
    0,			              // eth_address,
    5,			              // eth_power,
    23,			              // eth_mdc,
    18,			              // eth_mdio,
    ETH_PHY_LAN8720,      // eth_type,
    ETH_CLOCK_GPIO17_OUT	// eth_clk_mode
  },

  // TwilightLord-ESP32 Ethernet Shield
  {
    0,			              // eth_address,
    5,			              // eth_power,
    23,			              // eth_mdc,
    18,			              // eth_mdio,
    ETH_PHY_LAN8720,      // eth_type,
    ETH_CLOCK_GPIO17_OUT	// eth_clk_mode
  },

  // ESP3DEUXQuattro
  {
    1,                    // eth_address,
    -1,                   // eth_power,
    23,                   // eth_mdc,
    18,                   // eth_mdio,
    ETH_PHY_LAN8720,      // eth_type,
    ETH_CLOCK_GPIO17_OUT  // eth_clk_mode
  },

  // ESP32-ETHERNET-KIT-VE
  {
    0,                    // eth_address,
    5,                    // eth_power,
    23,                   // eth_mdc,
    18,                   // eth_mdio,
    ETH_PHY_IP101,        // eth_type,
    ETH_CLOCK_GPIO0_IN    // eth_clk_mode
  },

  // QuinLed-Dig-Octa Brainboard-32-8L and LilyGO-T-ETH-POE
  {
    0,			              // eth_address,
    -1,			              // eth_power,
    23,			              // eth_mdc,
    18,			              // eth_mdio,
    ETH_PHY_LAN8720,      // eth_type,
    ETH_CLOCK_GPIO17_OUT	// eth_clk_mode
  },

  // ABC! WLED Controller V43 + Ethernet Shield & compatible
  {
    1,                    // eth_address, 
    5,                    // eth_power, 
    23,                   // eth_mdc, 
    33,                   // eth_mdio, 
    ETH_PHY_LAN8720,      // eth_type,
    ETH_CLOCK_GPIO17_OUT	// eth_clk_mode
  },

  // Serg74-ESP32 Ethernet Shield
  {
    1,                    // eth_address,
    5,                    // eth_power,
    23,                   // eth_mdc,
    18,                   // eth_mdio,
    ETH_PHY_LAN8720,      // eth_type,
    ETH_CLOCK_GPIO17_OUT  // eth_clk_mode
  },

  // ESP32-POE-WROVER
  {
    0,                    // eth_address,
    12,                   // eth_power,
    23,                   // eth_mdc,
    18,                   // eth_mdio,
    ETH_PHY_LAN8720,      // eth_type,
    ETH_CLOCK_GPIO0_OUT   // eth_clk_mode
  },
  
  // LILYGO T-POE Pro
  // https://github.com/Xinyuan-LilyGO/LilyGO-T-ETH-Series/blob/master/schematic/T-POE-PRO.pdf
  {
    0,			              // eth_address,
    5,			              // eth_power,
    23,			              // eth_mdc,
    18,			              // eth_mdio,
    ETH_PHY_LAN8720,      // eth_type,
    ETH_CLOCK_GPIO0_OUT	// eth_clk_mode
  }
};
#endif


//by https://github.com/tzapu/WiFiManager/blob/master/WiFiManager.cpp
int getSignalQuality(int rssi)
{
    int quality = 0;

    if (rssi <= -100)
    {
        quality = 0;
    }
    else if (rssi >= -50)
    {
        quality = 100;
    }
    else
    {
        quality = 2 * (rssi + 100);
    }
    return quality;
}


// performs asynchronous scan for available networks (which may take couple of seconds to finish)
// returns configured WiFi ID with the strongest signal (or default if no configured networks available)
int8_t findWiFi(bool doScan) {
  if (multiWiFi.size() <= 1) {
    DEBUG_PRINTLN(F("Defaulf WiFi used."));
    return 0;
  }

  if (doScan) WiFi.scanDelete();  // restart scan

  int status = WiFi.scanComplete(); // complete scan may take as much as several seconds (usually <3s with not very crowded air)

  if (status == WIFI_SCAN_FAILED) {
    DEBUG_PRINTLN(F("WiFi scan started."));
    WiFi.scanNetworks(true);  // start scanning in asynchronous mode
  } else if (status >= 0) {   // status contains number of found networks
    DEBUG_PRINT(F("WiFi scan completed: ")); DEBUG_PRINTLN(status);
    int rssi = -9999;
    unsigned selected = selectedWiFi;
    for (int o = 0; o < status; o++) {
      DEBUG_PRINT(F(" WiFi available: ")); DEBUG_PRINT(WiFi.SSID(o));
      DEBUG_PRINT(F(" RSSI: ")); DEBUG_PRINT(WiFi.RSSI(o)); DEBUG_PRINTLN(F("dB"));
      for (unsigned n = 0; n < multiWiFi.size(); n++)
        if (!strcmp(WiFi.SSID(o).c_str(), multiWiFi[n].clientSSID)) {
          // find the WiFi with the strongest signal (but keep priority of entry if signal difference is not big)
          if ((n < selected && WiFi.RSSI(o) > rssi-10) || WiFi.RSSI(o) > rssi) {
            rssi = WiFi.RSSI(o);
            selected = n;
          }
          break;
        }
    }
    DEBUG_PRINT(F("Selected: ")); DEBUG_PRINT(multiWiFi[selected].clientSSID);
    DEBUG_PRINT(F(" RSSI: ")); DEBUG_PRINT(rssi); DEBUG_PRINTLN(F("dB"));
    return selected;
  }
  //DEBUG_PRINT(F("WiFi scan running."));
  return status; // scan is still running or there was an error
}


bool isWiFiConfigured() {
  return multiWiFi.size() > 1 || (strlen(multiWiFi[0].clientSSID) >= 1 && strcmp_P(multiWiFi[0].clientSSID, PSTR(DEFAULT_CLIENT_SSID)) != 0);
}


//handle Ethernet connection event
void WiFiEvent(WiFiEvent_t event)
{
  switch (event) {
#ifdef ESP8266
    case WIFI_EVENT_STAMODE_GOT_IP:
      DEBUG_PRINTLN();
      DEBUG_PRINT(F("IP address: ")); DEBUG_PRINTLN(Network.localIP());
      break;
    case WIFI_EVENT_STAMODE_CONNECTED:
      DEBUG_PRINTLN(F("WiFi: Connected!"));
      wasConnected = true;
      break;
    case WIFI_EVENT_STAMODE_DISCONNECTED:
      // called quite often (when not connected to WiFi)
      if (wasConnected) {
        DEBUG_PRINTLN(F("WiFi: Disconnected"));
        if (interfacesInited && WiFi.scanComplete() >= 0) findWiFi(true); // reinit WiFi scan
        interfacesInited = false;
        forceReconnect = true;
      }
      break;
    case WIFI_EVENT_SOFTAPMODE_STACONNECTED:
      // AP client connected
      DEBUG_PRINTLN(F("WiFi: AP Client Connected"));
      apClients++;
      DEBUG_PRINTLN(apClients);
      break;
    case WIFI_EVENT_SOFTAPMODE_STADISCONNECTED:
      // AP client disconnected
      DEBUG_PRINTLN(F("WiFi: AP Client Disconnected"));
      if (--apClients == 0 && isWiFiConfigured()) forceReconnect = true; // no clients reconnect WiFi if awailable
      DEBUG_PRINTLN(apClients);
      break;
#else
    case SYSTEM_EVENT_AP_STADISCONNECTED:
      // AP client disconnected
      DEBUG_PRINTLN(F("WiFi: AP Client Disconnected"));
      if (--apClients == 0 && isWiFiConfigured()) forceReconnect = true; // no clients reconnect WiFi if awailable
      DEBUG_PRINTLN(apClients);
      break;
    case SYSTEM_EVENT_AP_STACONNECTED:
      // AP client connected
      DEBUG_PRINTLN(F("WiFi: AP Client Connected"));
      apClients++;
      DEBUG_PRINTLN(apClients);
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
      DEBUG_PRINTLN();
      DEBUG_PRINT(F("IP address: ")); DEBUG_PRINTLN(Network.localIP());
      break;
    case SYSTEM_EVENT_STA_CONNECTED:
      DEBUG_PRINTLN(F("WiFi: Connected!"));
      wasConnected = true;
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      if (wasConnected) {
        DEBUG_PRINTLN(F("WiFi: Disconnected"));
        if (interfacesInited && WiFi.scanComplete() >= 0) findWiFi(true); // reinit WiFi scan
        interfacesInited = false;
        forceReconnect = true;
      }
      break;
    case SYSTEM_EVENT_AP_START:
      DEBUG_PRINTLN(F("WiFi: AP Started"));
      break;
    case SYSTEM_EVENT_AP_STOP:
      DEBUG_PRINTLN(F("WiFi: AP Stopped"));
      break;
  #if defined(WLED_USE_ETHERNET)
    case SYSTEM_EVENT_ETH_START:
      DEBUG_PRINTLN(F("ETH Started"));
      break;
    case SYSTEM_EVENT_ETH_CONNECTED:
      {
      DEBUG_PRINTLN(F("ETH Connected"));
      if (!apActive) {
        if (useESPNowSync && statusESPNow == ESP_NOW_STATE_ON) WiFi.disconnect(); // if using ESP-NOW just disconnect from current SSID
        else WiFi.disconnect(true); // otherwise disable WiFi entirely
      }
      if (multiWiFi[0].staticIP != (uint32_t)0x00000000 && multiWiFi[0].staticGW != (uint32_t)0x00000000) {
        ETH.config(multiWiFi[0].staticIP, multiWiFi[0].staticGW, multiWiFi[0].staticSN, dnsAddress);
      } else {
        ETH.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
      }
      // convert the "serverDescription" into a valid DNS hostname (alphanumeric)
      char hostname[64];
      prepareHostname(hostname);
      ETH.setHostname(hostname);
      showWelcomePage = false;
      break;
      }
    case SYSTEM_EVENT_ETH_DISCONNECTED:
      DEBUG_PRINTLN(F("ETH Disconnected"));
      // This doesn't really affect ethernet per se,
      // as it's only configured once.  Rather, it
      // may be necessary to reconnect the WiFi when
      // ethernet disconnects, as a way to provide
      // alternative access to the device.
      if (interfacesInited && WiFi.scanComplete() >= 0) findWiFi(true); // reinit WiFi scan
      forceReconnect = true;
      break;
  #endif
#endif
    default:
      break;
  }
}

