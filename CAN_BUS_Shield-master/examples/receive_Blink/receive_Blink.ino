#include <SpritzCipher.h>

#include <mcp_can.h>
#include <mcp_can_dfs.h>

// demo: CAN-BUS Shield, receive data with check mode
// send data coming to fast, such as less than 10ms, you can use this way
// loovee, 2014-6-13

#include <SPI.h>
#include "mcp_can.h"

// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 10;
const int LED        = 8;
boolean ledON        = 1;

MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin
uint8_t CORRECT_KEY[3] = {0x00, 0x01, 0x02};
const uint16_t KEY_LEN = sizeof(CORRECT_KEY);
byte *hash = new byte[20];
void setup()
{
    Serial.begin(115200);
    pinMode(LED,OUTPUT);

    while (CAN_OK != CAN.begin(CAN_500KBPS))              // init can bus : baudrate = 500k
    {
        Serial.println("CAN BUS Shield init fail");
        Serial.println("Init CAN BUS Shield again");
        delay(100);
    }
    Serial.println("CAN BUS Shield init ok!");
}


void loop()
{
    unsigned char len = 0;
    unsigned char buf[8];
    const uint8_t len_HASH = 20;
    byte buf_HASH[20];
    
  while(CAN_MSGAVAIL != CAN.checkReceive());
  // check if data coming
    {
        CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf

        Serial.println("\n-----------------------------");
        Serial.print("get data from ID: 0x");
        unsigned long canId = CAN.getCanId();
        Serial.println(canId, HEX);
        
         
        for(int i = 0; i<len; i++)    // print the data
        {
            Serial.print(buf[i]);
            Serial.print("\t");
            if(ledON && i==0)
            {

                digitalWrite(LED, buf[i]);
                ledON = 0;
                delay(500);
            }
            else if((!(ledON)) && i==4)
            {

                digitalWrite(LED, buf[i]);
                ledON = 1;
            }
        }
        Serial.println();
    }
 
  while(CAN_MSGAVAIL != CAN.checkReceive());
    {
     
      CAN.readMsgBuf(&len_HASH, buf_HASH);
       
        unsigned long canId = CAN.getCanId();
         Serial.print("get data from ID: 0x");
        Serial.println(canId, HEX);
        for(int i  = 0; i < 8; i++)
        {

          Serial.print(buf_HASH[i]);
          Serial.print(" ");
        }
       
    }

    while(CAN_MSGAVAIL != CAN.checkReceive());
    {
     
      CAN.readMsgBuf(&len_HASH, &buf_HASH[8]);
        unsigned long canId = CAN.getCanId();
         
        for(int i  = 8; i < 16; i++)
        {
        
          Serial.print(buf_HASH[i]);
          Serial.print(" ");
        }        
    }
    while(CAN_MSGAVAIL != CAN.checkReceive());
    {
      CAN.readMsgBuf(&len_HASH, &buf_HASH[16]);
        unsigned long canId = CAN.getCanId();
        for(int i  = 16; i < 20; i++)
        {
       
          Serial.print(buf_HASH[i]);
          Serial.print(" ");
        }
        Serial.println("");
       // Serial.println("----------------");
        
        spritz_mac(&hash[0], len_HASH, &buf[0], len, &CORRECT_KEY[0], KEY_LEN);
        Serial.println("Hash is : ");
        for(int i  = 0; i < 20; i++)
        {
          
          Serial.print(hash[i]);
          Serial.print(" ");
        }
        for(int i  = 0; i < 20; i++)
        {
          if(buf_HASH[i] != hash[i])
          {
            Serial.println("Failed");
          }
        }
        Serial.print("\nSuccess!! The correct message is received");
      
    }
   


    
}

//END FILE
