#include <SpritzCipher.h>

#include <mcp_can.h>
#include <mcp_can_dfs.h>
#include <stdio.h>
// demo: CAN-BUS Shield, send data
#include <mcp_can.h>
#include <SPI.h>

// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 10;
const int ledHIGH    = 1;
const int ledLOW     = 0;

MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin

void setup()
{
    Serial.begin(115200);

    while (CAN_OK != CAN.begin(CAN_500KBPS))              // init can bus : baudrate = 500k
    {
        Serial.println("CAN BUS Shield init fail");
        Serial.println(" Init CAN BUS Shield again");
        delay(100);
    }
    Serial.println("CAN BUS Shield init ok!");
}

unsigned char stmp[8] = {ledHIGH, 1, 2, 3, ledLOW, 5, 6, 7};
const uint16_t MESSAGE_LEN = sizeof(stmp);
uint8_t CORRECT_KEY[3] = {0x00, 0x01, 0x02};
const uint16_t KEY_LEN = sizeof(CORRECT_KEY);
uint8_t BAD_KEY[3] = {0x0f, 0x0e, 0x0d};
const uint8_t HASH_LEN = 20;

uint8_t CORRECT_HASH[HASH_LEN];

void loop()
{
    // send data:  id = 0x70, standard frame, data len = 8, stmp: data buf
      spritz_mac(&CORRECT_HASH[0], HASH_LEN, &stmp[0], MESSAGE_LEN, &CORRECT_KEY[0], KEY_LEN);
      Serial.println("\n");
     for(int i = 0; i < 20; i++)
     {
      Serial.print(CORRECT_HASH[i]);
      Serial.print(" ");
    }
    Serial.print("\n");
    CAN.sendMsgBuf(0x70, 0, 8, stmp);
    CAN.sendMsgBuf(0x80, 0, 8, CORRECT_HASH);
    CAN.sendMsgBuf(0x80, 0, 8, &CORRECT_HASH[8]);
    CAN.sendMsgBuf(0x80, 0, 4, &CORRECT_HASH[16]);
    delay(1000);                       // send data once per second
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
