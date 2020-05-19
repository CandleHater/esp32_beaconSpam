// used Wi-Fi channels (available: 1-14)
const uint8_t channels[] = {random(1, 14)}; // {2, 7, 13}
const bool wpa2 = false;

/*
  - don't forget the \n at the end of each SSID!
  - max. 32 characters per SSID
  - don't add duplicates! You have to change one character at least
*/
const char ssids[] PROGMEM = {
  "🌮 🌮 🌮 🌮 🌮\n"
  "Satoshi Nakamoto\n"
  "To the Moon!\n"
  "21.000.000 BTC\n"
  "HODL\n"
  "Money without masters\n"
  "When Lambo, Sir?\n"
  "⚡ ⚡ ⚡ ⚡ ⚡\n"
  "Brrrrrrrrrrrrrrr..\n"
  "Citadel Intranet\n"
  "REKT\n"
  "Read the Whitepaper\n"
  "Craig Wright is a fraud\n"
  "Buy Bitcoin\n"
  "🌝 🌝 🌝 🌝 🌝\n"
  "FUCK BANKS!\n"
  "Alice and Bob's WLAN\n"
  "6.15 BTC\n"
  "Be a Chad\n"
  "Bitcoin Bitcoin Bitcoin Bitcoin\n"
  "🚀 🚀 🚀 🚀 🚀\n"
  "HTTP 402 Payment Required\n"
  "The Times 03/Jan/2009\n"
  "The People's Money\n"
  "Bitcoin fixes this\n"
  "1 BTC = 1 BTC\n"
  "Orange coin good, number go up\n"
  "📈 📈 📈 📈 📈\n"
  "Bitcoin is \"fuck you money\"\n"
  "FOMO\n"
  "bitcoin.org\n"
  "Don't trust. Verify.\n"
  "Run a Bitcoin node\n"
  "Stacking Sats\n"
  "🕯 🕯 🕯 🕯 🕯\n"
  "Unconfiscatable\n"
  "Buy the Dip\n"
  "Lightning Network ⚡\n"
  "Be your own Bank\n"
  "DYOR\n"
  "🐱‍💻\n"
  "Hey, Hey, Hey! BitConneeeeee..\n"
  "1337 h4x0r, j0 m4n!\n"
  "BTC BTC BTC BTC BTC BTC\n"
  "In it for the technology\n"
  "Long BTC, 100x leverage\n"
};

#include <WiFi.h>

extern "C" {
  #include "esp_wifi.h"

  // typedef void (*freedom_outside_cb_t)(uint8 status);
  // int wifi_register_send_pkt_freedom_cb(freedom_outside_cb_t cb);
  // void wifi_unregister_send_pkt_freedom_cb(void);
  // int wifi_send_pkt_freedom(uint8 *buf, int len, bool sys_seq);
  esp_err_t esp_wifi_set_channel(uint8_t primary, wifi_second_chan_t second);
  esp_err_t esp_wifi_80211_tx(wifi_interface_t ifx, const void *buffer, int len, bool en_sys_seq);
}

// run-time variables
char emptySSID[32];
uint8_t channelIndex = 0;
uint8_t macAddr[6];
uint8_t wifi_channel = 1;
uint32_t currentTime = 0;
uint32_t packetSize = 0;
uint32_t packetCounter = 0;
uint32_t attackTime = 0;
uint32_t packetRateTime = 0;

// beacon frame definition
uint8_t beaconPacket[109] = {
  /*  0 - 3  */ 0x80, 0x00, 0x00, 0x00,                         // Type/Subtype: managment beacon frame
  /*  4 - 9  */ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,             // Destination: broadcast
  /* 10 - 15 */ 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,             // Source
  /* 16 - 21 */ 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,             // Source

  // Fixed parameters
  /* 22 - 23 */ 0x00, 0x00,                                     // Fragment & sequence number (will be done by the SDK)
  /* 24 - 31 */ 0x83, 0x51, 0xf7, 0x8f, 0x0f, 0x00, 0x00, 0x00, // Timestamp
  /* 32 - 33 */ 0xe8, 0x03,                                     // Interval: 0x64, 0x00 => every 100ms - 0xe8, 0x03 => every 1s
  /* 34 - 35 */ 0x31, 0x00,                                     // capabilities Tnformation

  // Tagged parameters

  // SSID parameters
  /* 36 - 37 */ 0x00, 0x20,                                     // Tag: Set SSID length, Tag length: 32
  /* 38 - 69 */ 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20,                                       // SSID

  // Supported Rates
  /* 70 - 71 */ 0x01, 0x08,                                     // Tag: Supported Rates, Tag length: 8
  /* 72 */ 0x82,                                                // 1(B)
  /* 73 */ 0x84,                                                // 2(B)
  /* 74 */ 0x8b,                                                // 5.5(B)
  /* 75 */ 0x96,                                                // 11(B)
  /* 76 */ 0x24,                                                // 18
  /* 77 */ 0x30,                                                // 24
  /* 78 */ 0x48,                                                // 36
  /* 79 */ 0x6c,                                                // 54

  // Current Channel
  /* 80 - 81 */ 0x03, 0x01,                                     // Channel set, length
  /* 82 */      0x01,                                           // Current Channel

  // RSN information
  /*  83 -  84 */ 0x30, 0x18,
  /*  85 -  86 */ 0x01, 0x00,
  /*  87 -  90 */ 0x00, 0x0f, 0xac, 0x02,
  /*  91 -  92 */ 0x02, 0x00,

  // Fix: changed 0x02(TKIP) to 0x04(CCMP) is default. WPA2 with TKIP not supported by many devices
  /*  93 - 100 */ 0x00, 0x0f, 0xac, 0x04, 0x00, 0x0f, 0xac, 0x04,

  /* 101 - 102 */ 0x01, 0x00,
  /* 103 - 106 */ 0x00, 0x0f, 0xac, 0x02,
  /* 107 - 108 */ 0x00, 0x00
};

// goes to next channel
void nextChannel() {
  if (sizeof(channels) < 2) {
    return;
  }

  uint8_t ch = channels[channelIndex];

  channelIndex++;
  if (channelIndex > sizeof(channels)) {
    channelIndex = 0;
  }

  if (ch != wifi_channel && ch >= 1 && ch <= 14) {
    wifi_channel = ch;

    // wifi_set_channel(wifi_channel);
    esp_wifi_set_channel(wifi_channel, WIFI_SECOND_CHAN_NONE);
  }
}

// generates random MAC
void randomMac() {
  for (int i = 0; i < 6; i++) {
    macAddr[i] = random(256);
  }
}

void setup() {
  // create empty SSID
  for (int i = 0; i < 32; i++) {
    emptySSID[i] = ' ';
  }

  // for random generator
  randomSeed(1);

  // set packetSize
  packetSize = sizeof(beaconPacket);
  if (wpa2) {
    beaconPacket[34] = 0x31;

  } else {
    beaconPacket[34] = 0x21;
    packetSize -= 26;
  }

  // generate random mac address
  randomMac();

  // put your setup code here, to run once:
  Serial.begin(115200);

  // change WiFi mode
  WiFi.mode(WIFI_MODE_STA);

  // set channel
  esp_wifi_set_channel(channels[0], WIFI_SECOND_CHAN_NONE);

  // print out SSIDs
  Serial.println("\n\nSSIDs:");

  int i = 0;
  int len = sizeof(ssids);
  while (i < len) {
    Serial.print((char) pgm_read_byte(ssids + i));
    i++;
  }

  Serial.println("\nspamming..\n");
}

void loop() {
  currentTime = millis();

  // send out SSIDs
  if (currentTime - attackTime > 100) {
    attackTime = currentTime;

    // temp variables
    int i = 0;
    int j = 0;
    int ssidNum = 1;
    char tmp;
    int ssidsLen = strlen_P(ssids);
    bool sent = false;

    // go to next channel
    nextChannel();

    // read out next SSID
    while (i < ssidsLen) {
      j = 0;
      do {
        tmp = pgm_read_byte(ssids + i + j);
        j++;
      } while (tmp != '\n' && j <= 32 && i + j < ssidsLen);

      uint8_t ssidLen = j - 1;

      // set MAC address
      macAddr[5] = ssidNum;
      ssidNum++;

      // write MAC address into beacon frame
      memcpy(&beaconPacket[10], macAddr, 6);
      memcpy(&beaconPacket[16], macAddr, 6);

      // reset SSID
      memcpy(&beaconPacket[38], emptySSID, 32);

      // write new SSID into beacon frame
      memcpy_P(&beaconPacket[38], &ssids[i], ssidLen);

      // set channel for beacon frame
      beaconPacket[82] = wifi_channel;

      // send packet
      for (int k = 0; k < 3; k++) {
        packetCounter += esp_wifi_80211_tx(ESP_IF_WIFI_STA, beaconPacket, packetSize, 0) == 0;
        delay(2); // 1
      }

      i += j;
    }
  }

  // show packet-rate
  if (currentTime - packetRateTime > 3000) {
    packetRateTime = currentTime;

    Serial.print(packetCounter);
    Serial.println(" packets/s");

    packetCounter = 0;
  }
}
