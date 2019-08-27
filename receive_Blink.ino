#include <aes.h>
#include <aes128_dec.h>
#include <aes128_enc.h>
#include <aes192_dec.h>
#include <aes192_enc.h>
#include <aes256_dec.h>
#include <aes256_enc.h>
#include <AESLib.h>
#include <aes_dec.h>
#include <aes_enc.h>
#include <aes_invsbox.h>
#include <aes_keyschedule.h>
#include <aes_sbox.h>
#include <aes_types.h>
#include <bcal-basic.h>
#include <bcal-cbc.h>
#include <bcal-cmac.h>
#include <bcal-ofb.h>
#include <bcal_aes128.h>
#include <bcal_aes192.h>
#include <bcal_aes256.h>
#include <blockcipher_descriptor.h>
#include <gf256mul.h>
#include <keysize_descriptor.h>
#include <memxor.h>

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
uint8_t Diffie_KEY[16];
MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin
uint8_t CORRECT_KEY[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
const uint16_t KEY_LEN = sizeof(Diffie_KEY);
const uint8_t HASH_LEN = 20;
uint8_t CORRECT_HASH[HASH_LEN];
 int count = 9999;
  unsigned long FinalTime;
  unsigned long StartTime; 
  //public variable representing the shared secret key.
//note that a uint8_t size key is suceptable to brute force attacks, consider a different data type of at least 1024 bits
uint8_t k; 
//prime number 
const uint8_t prime = 2147483647;
//generator
const uint8_t generator = 16807;  

//generates our 8 bit private secret 'a'.
uint8_t keyGen(){
  //Seed the random number generator with a reading from an unconnected pin, I think this on analog pin 2
  randomSeed(analogRead(2));

  //return a random number between 1 and our prime .
  return random(1,prime);
}

//code to compute the remainder of two numbers multiplied together.
uint8_t mul_mod(uint8_t a, uint8_t b, uint8_t m){


  uint8_t result = 0; //variable to store the result
  uint8_t runningCount = b % m; //holds the value of b*2^i

  for(int i = 0 ; i < 8 ; i++){

    if(i > 0) runningCount = (runningCount << 1) % m;
    if(bitRead(a,i)){
      result = (result%m + runningCount%m) % m; 

    } 

  }
  return result;
}

//The pow_mod function to compute (b^e) % m that was given in the class files  
uint8_t pow_mod(uint8_t b, uint8_t e, uint8_t m)
{
  uint8_t r;  // result of this function

  uint8_t pow;
  uint8_t e_i = e;
  // current bit position being processed of e, not used except for debugging
  uint8_t i;

  // if b = 0 or m = 0 then result is always 0
  if ( b == 0 || m == 0 ) { 
    return 0; 
  }

  // if e = 0 then result is 1
  if ( e == 0 ) { 
    return 1; 
  }

  // reduce b mod m 
  b = b % m;

  // initialize pow, it satisfies
  //    pow = (b ** (2 ** i)) % m
  pow = b;

  r = 1;

  // stop the moment no bits left in e to be processed
  while ( e_i ) {
    // At this point pow = (b ** (2 ** i)) % m

    // and we need to ensure that  r = (b ** e_[i..0] ) % m
    // is the current bit of e set?
    if ( e_i & 1 ) {
      // this will overflow if numbits(b) + numbits(pow) > 32
      r= mul_mod(r,pow,m);//(r * pow) % m; 
    }

    // now square and move to next bit of e
    // this will overflow if 2 * numbits(pow) > 32
    pow = mul_mod(pow,pow,m);//(pow * pow) % m;

    e_i = e_i >> 1;
    i++;
  }

  // at this point r = (b ** e) % m, provided no overflow occurred
  return r;
}
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
    int l = 0; 
    for(int i = 0; i < 4; i++)
    {
 

 uint8_t tempBits[4] = {0,0,0,0};
 uint8_t extra[4];
  uint8_t receivedBits[4] = {0,0,0,0};

  uint8_t B = 0;
  //This is our secret key
  unsigned char num[5];
    while(CAN_MSGAVAIL != CAN.checkReceive());
    
  CAN.readMsgBuf(5, num);
  for(int i = 0; i < 5; i++)
  {
    //Serial.print("Index is : ");
    //Serial.println(char(num[i]));
  }
  //delay(1000);
      while(CAN_MSGAVAIL != CAN.checkReceive());

  CAN.readMsgBuf(4, tempBits);
  for(int j = 0; j < 4; j++)
  {
    Serial.print("Value is : ");
    Serial.println(uint8_t (tempBits[j]));
     
   
  }
   
  delay(1000);
  for(int x = 0; x < 4; x++)
  {
    uint8_t a = keyGen();
extra[x] = a;
  //This is our shared index 'A'
  uint8_t A = pow_mod(generator, a, prime);

  Serial.print("Shared index is: ");
  Serial.println(A);
  receivedBits[x] = A;
  }
   CAN.sendMsgBuf(0x50, 0, 4, &receivedBits[0]);
   delay(1000);
   for(int y = 0; y < 4; y++)
   {
    k = pow_mod(tempBits[y],  extra[y], prime);
   Serial.print("The key is : ");
   Serial.println(k);
   
    Diffie_KEY[l + y] = k;
   }
   l = l + 4;
}Serial.print("\nDiffie key is :");
    for(int i = 0; i < 16; i++)
    {
      
      Serial.print(Diffie_KEY[i]);
      Serial.print(" ");
    }
    Serial.println();
}


void loop()
{
  
    unsigned char len = 0;
    unsigned char len1 = 0;
char buf[16];
char buf1[8];
    const uint8_t len_HASH = 20;
    byte buf_HASH[20];
    unsigned char arr[] = "56789";
    unsigned char arr1[5];
    
 //while(CAN_MSGAVAIL != CAN.checkReceive());
  // check if data coming
  {
     Serial.println("\n-----------------------------");
     
  /*for(int i = 0; i < sizeof(arr) - 1; i++)
  {
    Serial.print(char(arr[i]));
    Serial.print(arr1[i]);
    Serial.println(" ");
  }*/
  //while(CAN_MSGAVAIL == CAN.checkReceive())
  while(1)
  {
    while(CAN_MSGAVAIL != CAN.checkReceive());
    CAN.readMsgBuf(&len1, arr1);
    if((char(arr1[0]) == char(arr[0])) &&
        (char(arr1[1]) == char(arr[1])) &&
        (char(arr1[2]) == char(arr[2])) &&
        (char(arr1[3]) == char(arr[3])) &&
        (char(arr1[4]) == char(arr[4])))
        {
          if(count == 9999)
          {
            StartTime = millis();
           // count = 9999;
          }
          //Serial.println("Breaking the loop");
    break;
        }
        else
        {
          continue;
        }
  }
  while(CAN_MSGAVAIL != CAN.checkReceive());
        CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf
        //aes128_dec_single(CORRECT_KEY, buf);
        /*Serial.print("\ndecrypted:");
       for(int i = 0; i < 8; i++)
      {
        Serial.print((uint8_t)buf[i]);
        Serial.print(" ");
      }*/
      while(CAN_MSGAVAIL != CAN.checkReceive());
        CAN.readMsgBuf(&len, buf1);    // read data,  len: data length, buf: data buf
        for(int i = 0; i < 8; i++)
        {//Serial.print("here");
          buf[i + 8] = buf1[i];
          
        }
        aes128_dec_single(Diffie_KEY, buf);
    
       // Serial.println("\n-----------------------------");
       // Serial.print("get data from ID: 0x");
        unsigned long canId = CAN.getCanId();
       // Serial.println(canId, HEX);
        
         
        for(int i = 0; i<16; i++)    // print the data
        {
          // Serial.print(buf[i]);
            //Serial.print("\t");
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
       // Serial.println();
    }
 
  while(CAN_MSGAVAIL != CAN.checkReceive());
    {
     
      CAN.readMsgBuf(&len_HASH, buf_HASH);
       
        unsigned long canId = CAN.getCanId();
         //Serial.print("get data from ID: 0x");
      //  Serial.println(canId, HEX);
       
       
    }

    while(CAN_MSGAVAIL != CAN.checkReceive());
    {
     
      CAN.readMsgBuf(&len_HASH, &buf_HASH[8]);
        unsigned long canId = CAN.getCanId();
         
          
    }
    while(CAN_MSGAVAIL != CAN.checkReceive());
    {
      CAN.readMsgBuf(&len_HASH, &buf_HASH[16]);
        unsigned long canId = CAN.getCanId();
     
       
       // Serial.println("----------------");
       char buf2[17]; 
       for(int i = 0; i < 16; i++)
       {
        buf2[i] = buf[i];
      //  Serial.print(buf2[i]);
       // Serial.print(" ");
       }
        const uint16_t MESSAGE_LEN = 17;
        //Serial.println(sizeof(buf2));
        spritz_mac(&CORRECT_HASH[0], HASH_LEN, &buf2[0],MESSAGE_LEN, &Diffie_KEY[0], KEY_LEN);
        //Serial.println("Hash is : ");
      
        for(int i  = 0; i < 20; i++)
        {
          if(buf_HASH[i] != CORRECT_HASH[i])
          {
            Serial.println("Failed");
          }
        }
         Serial.print("\nCount: ");
         Serial.println(count);
         if(count == 0)
         {
           FinalTime = millis();
           Serial.print("Time in mili seconds : ");
           Serial.println(((FinalTime - StartTime)/10000.0) - 1200);
           Serial.print("Time in seconds : ");
           Serial.println((((FinalTime - StartTime)/10000.0) - 1200)/1000.0);
           count = 9999;
         }
         
         count--;
       Serial.print("\nSuccess!! The correct message is received");
          }    
}

//END FILE
