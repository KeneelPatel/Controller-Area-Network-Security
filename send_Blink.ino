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
uint8_t Diffie_KEY[16];
const uint8_t HASH_LEN = 20;
uint8_t CORRECT_HASH[HASH_LEN];
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

    while (CAN_OK != CAN.begin(CAN_500KBPS))              // init can bus : baudrate = 500k
    {
        Serial.println("CAN BUS Shield init fail");
       Serial.println(" Init CAN BUS Shield again");
        delay(100);
    }
    Serial.println("CAN BUS Shield init ok!");
    Serial.print(CORRECT_HASH[0]);
    //initialize pins
    int l = 0; 
    for(int i = 0; i < 4; i++)
    {
    //char Key[16];

 /* uint8_t t = A;
  int c = 0; 
  char tp[10];
while(t/10 >= 0)
{
 
  int d = t %10;
   tp[c] = d;
  t = t / 10;
  c = c + 1;
  Serial.println(tp[c]);
}
uint8_t g = atoi(tp);
Serial.print(g);*/
  uint8_t tempBits[4] = {0,0,0,0};
  uint8_t extra[4];
  for(int j = 0; j < 4; j++)
  {
    uint8_t a = keyGen();
    extra[j] = a;
  //This is our shared index 'A'
  uint8_t A = pow_mod(generator, a, prime);
  Serial.print("Shared index is: ");
  Serial.println(A);
  tempBits[j] = A;
  }

   uint8_t receivedBits[4];


  uint8_t B = 0;
  //This is our secret key
   char num[] = "12345";
    delay(1000);
   CAN.sendMsgBuf(0x50, 0, 5, num);   
 delay(1000);
  CAN.sendMsgBuf(0x500, 0, 4, &tempBits[0]); 
  delay(1000);
   while(CAN_MSGAVAIL != CAN.checkReceive());
  CAN.readMsgBuf(4, receivedBits);
   delay(1000);
 for(int x = 0; x < 4; x++)
 {
     Serial.print("Value is : ");
    Serial.println(uint8_t (receivedBits[x]));
  k = pow_mod(receivedBits[x], extra[x], prime);
   Serial.print("The key is : ");
   Serial.println(k);
   Diffie_KEY[l + x] = k;
 }
l = l + 4;
    }
   Serial.print("\nDiffie key is :");
    for(int i = 0; i < 16; i++)
    {
      
      Serial.print(Diffie_KEY[i]);
      Serial.print(" ");
    }
    Serial.println();
   
 }
char stmp[] = "1234567890123456";
const uint16_t MESSAGE_LEN = sizeof(stmp);
uint8_t CORRECT_KEY[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
const uint16_t KEY_LEN = sizeof(Diffie_KEY);
uint8_t BAD_KEY[3] = {0x0f, 0x0e, 0x0d};

void loop()
{
  
  char stmp[] = "1234567890123456";
  uint8_t key[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
unsigned long time;
 Serial.print("Time: ");
  time = millis();

  Serial.println(time/1000);
    // send data:  id = 0x70, standard frame, data len = 8, stmp: data buf
   // Serial.println(stmp);
      spritz_mac(&CORRECT_HASH[0], HASH_LEN, &stmp[0], MESSAGE_LEN, &Diffie_KEY[0], KEY_LEN);
      Serial.println("\n");
      /* if (Serial.available()) {
    // returns one byte of unsigned serial data, or -1 if none available
    int16_t inByte = Serial.read();  //mask out lower 8 bits from k
    Serial.write(inByte^(random(256)));
    if(inByte = '\n')
      randomSeed(k);
  }*/
     /* for(int i = 0; i < sizeof(stmp); i++)
      {
        Serial.print(stmp[i]);
      Serial.print(" ");
      }*/
     // Serial.println();
   // for(int i = 0; i < 20; i++)
     //{
     // Serial.print(CORRECT_HASH[0]);
     // Serial.print(" ");
    //}
    aes128_enc_single(Diffie_KEY, stmp);
    
//Serial.print("\nencrypted:\n");
unsigned char copy[16];
for(int i = 0; i < sizeof(stmp)- 1; i++)
      {
        copy[i] = stmp[i];
      
      }
      unsigned char arr[] = "56789";
    Serial.print("\n");
    CAN.sendMsgBuf(0x60, 0, 5, arr);
   delay(200);
    CAN.sendMsgBuf(0x70, 0, 8, &(copy[0]));
    delay(200);
     CAN.sendMsgBuf(0x70, 0, 8, &(copy[8]));
    delay(200);
    CAN.sendMsgBuf(0x80, 0, 8, CORRECT_HASH);
      
        delay(200);   
         CAN.sendMsgBuf(0x80, 0, 8, &CORRECT_HASH[8]);
       delay(200);
           CAN.sendMsgBuf(0x80, 0, 4, &CORRECT_HASH[16]);
            delay(200);
         //delay(100);
    
 // delay(100);
    
       // delay(10000);
        //delay(10000);
    //delay(1000);                       // send data once per second
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
