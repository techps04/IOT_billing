//Code for shopping automation using RFID reader
//IMPLEMENT THIS CODE ON ARDUINO UNO WITH SPECIFIED LIBRARIES

#include <MFRC522.h>
#include<LiquidCrystal.h>
#include <SPI.h>

LiquidCrystal lcd(2, 3, 4, 5, 6, 7);

#define SS_PIN 10
#define RST_PIN 9
#define no_ims 3               // No. of items in the shop
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key; 


int switchPin = 11;
boolean lastButton = LOW;
boolean currentButton = LOW;



int code[no_ims][4] = {
                       {212,138,223,43},
                       {212,138,223,44},
                       {212,138,223,45}
                       };                 //This is the stored UID
                       
char pnm  [no_ims][50] = {"oreo biscuit", "apple", "washing powder"}; //Product name
int  price[no_ims] = {40,160,100}, price_sum = 0, j = 0;


int codeRead = 0;
String uidString;



void setup() {
  lcd.begin(16,2);
  Serial.begin(9600);
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 
  pinMode(8,OUTPUT);
  pinMode(A0,OUTPUT);
  pinMode(switchPin, INPUT);
    
}

boolean debounce(boolean last)
{
  boolean current = digitalRead(switchPin);
  if (last != current)
  {
    delay(5);
    current = digitalRead(switchPin);
  }
  return current;
}

 void clearUID()
  {
    lcd.print(uidString); 
  }
  
 void printDec(byte *buffer, byte bufferSize) {                                                //Conversion to Decimal value of RFID
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}

  void printMessage()                                             
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(pnm[j][0]);
    lcd.setCursor(0,1);
    lcd.print("Rs");                         // This part Prints the apperent price
    lcd.setCursor(3,1);
    lcd.print(price[j]);
    
    digitalWrite(A0,HIGH);
    delay(2000);                             // LED output
    
    lcd.clear ();
    lcd.print("RFID");
    lcd.setCursor(0,1);                      // Printing the RFID
    lcd.print(uidString);
    digitalWrite(A0,LOW);
    delay(2000);
    }   

 
  void printUID()
  {
   
  }


void loop() {

  currentButton = debounce(lastButton);
  
  lcd.clear();
    lcd.print("TOTAL ->");
    lcd.setCursor(0,1);
    lcd.print("Rs");
    lcd.setCursor(3,1);
    lcd.print(price_sum);
    
  if(lastButton == LOW && currentButton == HIGH){

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("TOTAL PURCHASE :");
    lcd.setCursor(0,1);
    lcd.print("Rs");                         // This part Prints the TOTAL price
    lcd.setCursor(3,1);
    lcd.print(price_sum);
     
     return;  
  }
  lastButton = currentButton;
    
  if(  rfid.PICC_IsNewCardPresent())
  {
      readRFID();
  }
  delay(100);
  lcd.clear();
}




void readRFID()                                                   //The heart of the code ! Sensing and displaying
{
  
  rfid.PICC_ReadCardSerial();
  Serial.print(F("\nPICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }

    clearUID();
   
    Serial.println("Scanned PICC's UID:");
    printDec(rfid.uid.uidByte, rfid.uid.size);

    uidString = String(rfid.uid.uidByte[0])+" "+String(rfid.uid.uidByte[1])+" "+String(rfid.uid.uidByte[2])+ " "+String(rfid.uid.uidByte[3]);
    
    printUID();

    int numb = no_ims, i = 0;
    boolean match = true;

    while(numb--){           //Loop for seeing anyone one the RFID mathes with the given one ?
      
    i = 0; match = true;  
    while(i<rfid.uid.size)
    {
      if(!(rfid.uid.uidByte[i] == code[j][i]))
      {
           match = false;
      }
      i++;
    }
   
    if(match){                      //If it matches, i.e the product has been sensed => its price should be added
       
       price_sum += price [j];      // j contains the index of the RFID array
      break;                        //If anyone of them matches then, the loop breaks, and procees towards showing the name, price and ID  
     }

     j++;
    
    }
    
    j--;

    if(match)                                                 //If the RFID matches
    {
      Serial.println("\nI know this Tag!");
      lcd.clear();
      lcd.print(uidString);
      printMessage();
    }
    else                                                     //If not, that means the product RFID dosen't match the directory's. 
    {
           lcd.clear();
      lcd.print(uidString);
      lcd.setCursor(0,1);
      digitalWrite(8,HIGH);
      lcd.print("INVALID ID");
      delay(2000);
      digitalWrite(8,LOW);
      lcd.clear();
      Serial.println("\nUnknown Tag");
    }


    // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
}
