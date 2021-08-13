#include <OneWire.h>

#include <EEPROM.h>

#define pin 10

#define FAMILY_CODE 0x01

OneWire iButton(pin);

byte IDconst[8] = {
   FAMILY_CODE, // Family code

   0xFF,
   0xFF,
   0xFF,
   0xFF,
   0xFF,
   0xFF,

   0x2F // Has to be real CRC
};

byte arr[8]; // temp
byte ID[8];

char com;

void setup() {
   Serial.begin(9600);

   Serial.print("r - Read the ID\n");
   Serial.print("w - Write the ID\n");
   Serial.print("c - Load const ID\n");
   Serial.print("p - Print vars\n");
   Serial.print("k - Enter the ID manually\n");

   ID[0] = 0x01;
}

void iButtonWait() {
   while (!iButton.search(arr)) {
      iButton.reset_search();
      delay(200);
   }
}

void iButtonRead() {
   Serial.print("Ready to read the ID\n");
   iButtonWait();
   Serial.print("ID: ");
   for (byte x = 0; x != 8; ++x) {
      ID[x] = arr[x];

      if (ID[x] < 0x0A) {
         Serial.print('0');
      }
      Serial.print(ID[x], HEX);
      Serial.print(' ');
   }

   byte crc = iButton.crc8(ID, 7);
   Serial.print("CRC: ");
   Serial.println(crc, HEX);
}

void iButtonWrite() {
   if (ID[0] != FAMILY_CODE) {
      Serial.println("You are about to change the family code. Recompile to allow this");
      return;
   }

   byte crc = iButton.crc8(ID, 7);
   if (crc != ID[7]) {
      Serial.println("Warning!!! CRC mismatch!!!");
   }

   Serial.print("Ready to write ID ");
   for (byte x = 0; x != 8; ++x) {

      if (ID[x] < 0x0A) {
         Serial.print('0');
      }
      Serial.print(ID[x], HEX);
      Serial.print(' ');
   }
   Serial.println();
   iButtonWait();
   iButton.skip();
   iButton.reset();
   iButton.write(0x33);
   Serial.print("Current ID:");
   for (byte x = 0; x != 8; ++x) {
      Serial.print(' ');
      Serial.print(iButton.read(), HEX);
   }

   iButton.skip();
   iButton.reset();
   iButton.write(0xD1);
   digitalWrite(10, LOW);
   pinMode(10, OUTPUT);
   delayMicroseconds(60);
   pinMode(10, INPUT);
   digitalWrite(10, HIGH);
   delay(10);

   Serial.println();
   Serial.print("Writing iButton ID: ");
   for (byte x = 0; x != 8; ++x) {
      if (ID[x] < 0x0A) {
         Serial.print('0');
      }
      Serial.print(ID[x], HEX);
      Serial.print(' ');
   }
   Serial.println();

   iButton.skip();
   iButton.reset();
   iButton.write(0xD5);
   for (byte x = 0; x != 8; ++x) {
      writeByte(ID[x]);
      Serial.print("**");
   }
   Serial.println();
   iButton.reset();
   iButton.write(0xD1);
   digitalWrite(10, LOW);
   pinMode(10, OUTPUT);
   delayMicroseconds(10);
   pinMode(10, INPUT);
   digitalWrite(10, HIGH);
   delay(10);
}

void printVars() {
   Serial.print("Current ID: ");
   for (byte x = 0; x != 8; ++x) {
      if (ID[x] < 0x0A) {
         Serial.print('0');
      }
      Serial.print(ID[x], HEX);
      Serial.print(' ');
   }
   Serial.println();

   Serial.print("Const ID: ");
   for (byte x = 0; x != 8; ++x) {
      if (IDconst[x] < 0x0A) {
         Serial.print('0');
      }
      Serial.print(IDconst[x], HEX);
      Serial.print(' ');
   }
   Serial.println();
}

void loadConst() {
   Serial.print("Constant ID: ");
   for (byte x = 0; x != 8; ++x) {
      ID[x] = IDconst[x];

      if (ID[x] < 0x0A) {
         Serial.print('0');
      }
      Serial.print(ID[x], HEX);
      Serial.print(' ');
   }
   Serial.println();
}

void writeByte(byte data) {
   int data_bit;
   for (data_bit = 0; data_bit != 8; ++data_bit) {
      if (data & 1) {
         digitalWrite(pin, LOW);
         pinMode(pin, OUTPUT);
         delayMicroseconds(60);
         pinMode(pin, INPUT);
         digitalWrite(pin, HIGH);
         delay(10);
      } else {
         digitalWrite(pin, LOW);
         pinMode(pin, OUTPUT);
         pinMode(pin, INPUT);
         digitalWrite(pin, HIGH);
         delay(10);
      }
      data >>= 1;
   }
}

void manualID() {
   byte i = 1;
   byte temp = 0;
   byte cnt = 0;
   Serial.println("XX XX XX XX XX XX>");
   for (;;) {
      Serial.flush();
      if (i == 7) {
         ID[i] = iButton.crc8(ID, 7);
         printVars();
         return;
      }

      if (Serial.available()) {
         byte b = Serial.read();

         if (b != 0x0A) { // Enter
            if (b >= 0x41 && b <= 0x46) { // A-F
               temp |= b - 0x37;
               ++cnt;
               if (cnt == 2) {
                  ID[i] = temp;
                  ++i;
                  cnt = 0;
                  temp = 0;
               } else {
                  temp <<= 4;
               }
            } else if (b >= 0x30 && b <= 0x39) { // 0-9
               temp |= b - 0x30;
               ++cnt;
               if (cnt == 2) {
                  ID[i] = temp;
                  ++i;
                  cnt = 0;
                  temp = 0;
               } else {
                  temp <<= 4;
               }
            }
         }
      }
   }
}

void loop() {
   if (Serial.available()) {
      com = Serial.read();
   }
   switch (com) {
   case 'c':
      loadConst();
      break;
   case 'r':
      iButtonRead();
      break;
   case 'p':
      printVars();
      break;
   case 'w':
      iButtonWrite();
      break;
   case 'k':
      manualID();
      break;
   }
}