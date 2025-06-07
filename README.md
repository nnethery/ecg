┌───────── ADS1292R Board ─────────┐         
| (3.5 mm jack) ◄─── Patient Cable |
│                                  │         
│  [CS]───10      [DRDY]───7       │         
│  [MOSI]─16      [MISO]──14       │         
│  [SCLK]─15      [START]──8       │         
│  [RESET]─9      VCC/GND          │         
└──────────────────────────────────┘         
        │      │        │                      
        ▼      ▼        ▼                      
Pro Micro 3 V3  SPI Bus + GPIOs              
        ▲      ▲        ▲                      
┌──────────────────────────────────┐         
│          HM‑10 Module            │         
│  RXD◄────1      TXD────0         │         
│  VCC/GND                         │         
└──────────────────────────────────┘         
