 /* Fill-in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID "*****"
#define BLYNK_TEMPLATE_NAME "Quickstart Template"
#define BLYNK_AUTH_TOKEN "*********"

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <BlynkSimpleEthernet.h>
#include <EEPROM.h>
#include <WS2812FX.h>
#include <IRremote.hpp>

#define LED_COUNT 12
#define LED_PIN   28

WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

#include "TouchyTouch.h"
const int touch_threshold_adjust = 300;
const int touch_pins[] = {6,5,2,1};
const int touch_count = sizeof(touch_pins) / sizeof(int);
TouchyTouch touches[touch_count];
uint32_t time_now = 0;

BlynkTimer timer;

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(10, 0, 1, 226);

int r;
int g;
int b;  
int V_Value;

#include "PinDefinitions.h"

uint32_t Color_Device[12] = {
  0xFF0000,  // RED
  0x00FF00,  // GREEN
  0x0000FF,  // BLUE
  0xFFFFFF,  // WHITE
  0xFFFF00,  // YELLOW
  0x00FFFF,  // CYAN
  0xFF00FF,  // MAGENTA
  0x33E6CC,   //TYRQUOISE BLUE
  0x800080,  // PURPLE
  0xFFA500,  // ORANGE
  0xFFC0CB,  // PINK
  0xFF8C00   // DARK ORANGE
};

BLYNK_WRITE(V5) {   
  // Called when virtual pin V2 is updated from the Blynk.App
  // V2 is a datastream of data type String assigned to a   
  // Blynk.App ZeRGBa widget.
  r = param[0].asInt();
  g = param[1].asInt();
  b = param[2].asInt();
  Serial.print("V5: r = ");
  Serial.print(r);
  Serial.print("\t g=");
  Serial.print(g);
  Serial.print("\t b=");
  Serial.println(b);
  ws2812fx.setColor(((uint32_t)r << 16 | (uint32_t)g << 8 | (uint32_t)b));
}

BLYNK_WRITE(V4) {  
  V_Value= param[0].asInt();
  Serial.print("V4:");
  Serial.println(V_Value);  
  if(V_Value == 0 ||V_Value == 1 )
  {
    IrSender.sendPulseDistanceWidth(38, 2150, 800, 700, 1500, 700, 750, 0x90110, 21, PROTOCOL_IS_LSB_FIRST, 0, 0);
    //WS2812_SHOW_WHEN_IR_SEND();
  } 
  else if(V_Value == 2)
  {
    IrSender.sendPulseDistanceWidth(38, 2150, 800, 700, 1500, 700, 800, 0x16D510, 21, PROTOCOL_IS_LSB_FIRST, 0, 0);
    //WS2812_SHOW_WHEN_IR_SEND();
  } 
  else if(V_Value == 3)
  {
    IrSender.sendPulseDistanceWidth(38, 2200, 750, 700, 1500, 700, 750, 0x1A3510, 21, PROTOCOL_IS_LSB_FIRST, 0, 0);
    //WS2812_SHOW_WHEN_IR_SEND();
  }
  else if(V_Value == 4)
  {
    IrSender.sendPulseDistanceWidth(38, 2200, 750, 700, 1500, 700, 800, 0x1E9910, 21, PROTOCOL_IS_LSB_FIRST, 0, 0);
    //WS2812_SHOW_WHEN_IR_SEND();
  } 
}

uint8_t Device_num = 0;

void setup() {  
  for (int i = 0; i < touch_count; i++) {
    touches[i].begin( touch_pins[i] );
    touches[i].threshold += touch_threshold_adjust; // make a bit more noise-proof
  }  
  ws2812fx.init();
  ws2812fx.start();  
  ws2812fx.setPixelColor(0, Color_Device[0]);
  ws2812fx.show();

  pinMode(13, OUTPUT);
  digitalWrite(13,HIGH);
  
  // Initialize components
  Serial.begin(115200);
//   while (!Serial) {
//     ; // wait for serial port to connect. Needed for native USB port only
//   }

  // Just to know which program is running on my Arduino
  Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));

  // Start the receiver and if not 3. parameter specified, take LED_BUILTIN pin from the internal boards definition as default feedback LED
  IrReceiver.begin(IR_RECEIVE_PIN);
  IrSender.begin(IR_SEND_PIN); // Start with IR_SEND_PIN as send pin and enable feedback LED at default feedback LED pin

  Serial.print(F("Ready to receive IR signals of protocols: "));
  printActiveIRProtocols(&Serial);
  Serial.println(F("at pin" STR(IR_RECEIVE_PIN)));

  Ethernet.init(17);  // WIZnet W5100S-EVB-Pico
  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }
  Blynk.begin(BLYNK_AUTH_TOKEN);
}

int on = 0;
unsigned long last = millis();
bool Work_Mode = false;

void loop() {
//  if(Work_Mode)
//  {
    if (IrReceiver.decode()) {
        // If it's been at least 1/4 second since the last
        if (millis() - last > 250) {
            IrReceiver.printIRResultShort(&Serial);
            IrReceiver.printIRSendUsage(&Serial);
            Serial.println();
            if (IrReceiver.decodedIRData.protocol == UNKNOWN) {
              // We have an unknown protocol, print more info
              Serial.println(F("Received noise or an unknown (or not yet enabled) protocol"));
              IrReceiver.printIRResultRawFormatted(&Serial, true);
            }
        }
        last = millis();
        IrReceiver.resume(); // Enable receiving of the next value
    }
//  }
//  else
  {
    Blynk.run();
    timer.run();                                                                                                                       
  }
  // Check for touch button input 
  Touch_handling();
  delay(50);
}

void Touch_handling()
{
  // key handling
  for ( int i = 0; i < touch_count; i++) {
    touches[i].update();
    if ( touches[i].rose() ) {
      Serial.print("Button:");
      Serial.println(i);
      if(i==0)
      {
        time_now = millis();
      }
      else
      {
         switch (Device_num){
          case 0:  //Dyson Fan
            {
              if(i == 1)
              {
                IrSender.sendPulseDistanceWidth(38, 2150, 800, 700, 1500, 700, 800, 0x16D510, 21, PROTOCOL_IS_LSB_FIRST, 0, 0);
                //WS2812_SHOW_WHEN_IR_SEND();
              }
              else if (i ==2)
              {
                IrSender.sendPulseDistanceWidth(38, 2200, 750, 700, 1500, 700, 750, 0xA3510, 21, PROTOCOL_IS_LSB_FIRST, 0, 0);
                //WS2812_SHOW_WHEN_IR_SEND();
              }
              else if(i ==3)
              {
                IrSender.sendPulseDistanceWidth(38, 2200, 750, 700, 1500, 700, 800, 0x1E9910, 21, PROTOCOL_IS_LSB_FIRST, 0, 0);
                //WS2812_SHOW_WHEN_IR_SEND();
              }
            }
            break;
          case 1:  
            {
              
            }
            break;
         }
      }
     }
    if ( touches[i].fell() ) {
     Serial.printf("Release:");
     Serial.println(i);
     if(i == 0)
     {
       if((millis()-time_now)>500)
       {
        Serial.print("Trun on/off Device:");
        Serial.println(Device_num);
        switch (Device_num){
          case 0:
            {
              IrSender.sendPulseDistanceWidth(38, 2150, 800, 700, 1500, 700, 750, 0x90110, 21, PROTOCOL_IS_LSB_FIRST, 0, 0);
              //WS2812_SHOW_WHEN_IR_SEND();
            }
            break;
          case 1:
            {
              
            }
            break;
         }
       }
       else
       {
          ws2812fx.setPixelColor(Device_num, BLACK);
          if(Device_num <11)
          {
            Device_num ++;            
          }else
          {
            Device_num = 0;
          }
          ws2812fx.setPixelColor(Device_num, Color_Device[Device_num]);
          ws2812fx.show();
       }
     }
    }
  }
}

//void WS2812_SHOW_WHEN_IR_SEND(void)
//{
////  for(int i =0; i<12; i++)
////  {
////    ws2812fx.setPixelColor(Device_num, Color_Device[i]);
////    ws2812fx.show();
////    delay(5);
////  } 
////  ws2812fx.setPixelColor(Device_num, Color_Device[Device_num]);
////  ws2812fx.show();
//}

/*风扇开关*/
//IrSender.sendPulseDistanceWidth(38, 2150, 800, 700, 1500, 700, 750, 0x90110, 21, PROTOCOL_IS_LSB_FIRST, /*<RepeatPeriodMillis>*/0, /*<numberOfRepeats>*/0);
/*风扇加大风量*/
//IrSender.sendPulseDistanceWidth(38, 2150, 800, 700, 1500, 700, 800, 0x16D510, 21, PROTOCOL_IS_LSB_FIRST, 0, 0);
/*风扇减小风量*/
//IrSender.sendPulseDistanceWidth(38, 2200, 750, 700, 1500, 700, 750, 0xA3510, 21, PROTOCOL_IS_LSB_FIRST, 0, 0);
/*风扇摇头*/
//IrSender.sendPulseDistanceWidth(38, 2200, 750, 700, 1500, 700, 800, 0x1E9910, 21, PROTOCOL_IS_LSB_FIRST, 0, 0);
