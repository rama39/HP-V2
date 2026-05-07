//================================================================================================
// KEYPAD

#include <Keypad.h>
const byte ROWS = 4, COLS = 3; 
//define the symbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'-','0','+'}
};
// __ C2 R1 C1 R4 C3 R3 R2 __
// __ 18 33 35 37 40 38 36 __
//byte rowPins[ROWS] = {35, 40, 33, 37}, colPins[COLS] = {36, 34, 38};
byte rowPins[ROWS] = {33, 36, 38, 37}, colPins[COLS] = {35, 18, 40};
//initialize an instance of class NewKeypad
Keypad teclado = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

//================================================================================================
// DISPLAY OLED

#include <Wire.h>
#include <Arduino.h>
#include <U8x8lib.h>
#ifdef U8X8_HAVE_HW_SPI
  #include <SPI.h>
#endif

U8X8_SH1106_128X64_NONAME_HW_I2C display_(U8X8_PIN_NONE);

int HPMAX = 128, HP = HPMAX;
int modNum = 0, len = 0; uint8_t modStr[4] = "   ", prevMod[5] = "    ";
bool morrendo = 0, morto = 0; int falhas = 0, sucessos = 0;

void drawClear() {
}

void drawHP(void) {

  int frac, fulls, edgeLen, col, row;
  if(HPMAX){
    //frac=HP*128/HPMAX;
    fulls = HP*15/HPMAX;
    //edgeLen = ((128*HP)/HPMAX)%16;
  }
  //uint8_t edgeFill = 0x0F>>(8-edgeLen/2);
  //const uint8_t edge_tile[] = { edgeFill, edgeFill, edgeFill, edgeFill, edgeFill, edgeFill, edgeFill, edgeFill };

  static const uint8_t void_tile[128] = {};
  static const uint8_t full_tile[] = {  //16 rows
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF 
  };
  display_.drawTile(0, 0, fulls, (uint8_t*)full_tile);
  display_.drawTile(fulls, 0, 15-fulls, (uint8_t*)void_tile);

  display_.setFont(u8x8_font_lucasarts_scumm_subtitle_o_2x2_f);
  char print[32];
  sprintf(print, "%3i /%3i", HP, HPMAX);
  display_.drawString(0,3, print);
}

void draw(void) {

  drawHP();

  display_.setFont(u8x8_font_amstrad_cpc_extended_f);
  char print[32];
  sprintf(print, "%s        %s", prevMod, modStr);
  display_.drawString(0,7, print);

  //Serial.printf("%3i / %3i | m:%c M:%c | s:%i f:%i | prev: '%s', mod: '%s'\n", HP, HPMAX, morrendo?'s':'n', morto?'s':'n', sucessos, falhas, prevMod, modStr);
}

//================================================================================================
// LEDS

#define VERDE_1 1
#define VERDE_2 2
#define VERDE_3 4
#define AMARELO 7
#define VERMELHO_1 9
#define VERMELHO_2 11
#define VERMELHO_3 12

void computeLeds() {
  digitalWrite(AMARELO, (morrendo)?HIGH:LOW);
  digitalWrite(VERMELHO_1, (falhas>0)?HIGH:LOW);
  digitalWrite(VERMELHO_2, (falhas>1)?HIGH:LOW);
  digitalWrite(VERMELHO_3, (falhas>2)?HIGH:LOW);
  digitalWrite(VERDE_1, (sucessos>0)?HIGH:LOW);
  digitalWrite(VERDE_2, (sucessos>1)?HIGH:LOW);
  digitalWrite(VERDE_3, (sucessos>2)?HIGH:LOW);
}

//================================================================================================

#include <EEPROM.h>
/**/
//addresses in EEPROM of HP and HPMAX
#define SAVEDADDR 0 //is saved?
#define morrendoADDR 1
#define sucessosADDR 2
#define falhasADDR 3
#define HPADDR 4
#define HPMAXADDR 8
#define mortoADDR 12
void writeInt(int addr, int val) {/**/
  EEPROM.write(addr,   (val    )&255);
  EEPROM.write(addr+1, (val>> 8)&255);
  EEPROM.write(addr+2, (val>>16)&255);
  EEPROM.write(addr+3, (val>>24)&255);
}
int readInt(int addr) {/**/
  int val = 0;
  val += EEPROM.read(addr  )    ;
  val += EEPROM.read(addr+1)<< 8; 
  val += EEPROM.read(addr+2)<<16; 
  val += EEPROM.read(addr+3)<<24;
  return val;
}

void saveFlags() {
  EEPROM.write(morrendoADDR, morrendo); EEPROM.write(mortoADDR, morto);
  EEPROM.write(sucessosADDR, sucessos); EEPROM.write(falhasADDR, falhas);
}
void loadFlags() {
  morrendo = EEPROM.read(morrendoADDR); morto = EEPROM.read(mortoADDR);
  sucessos = EEPROM.read(sucessosADDR); falhas = EEPROM.read(falhasADDR);
}

//================================================================================================
// UTILITIES

//update HPMAX
#define UPDATEMAX 6

// Atualiza leds, printa ultima atualização, reseta entrada
void confirma(char modCha) {
  computeLeds();
  prevMod[0]=modCha;
  prevMod[1]=modStr[0];
  prevMod[2]=modStr[1];
  prevMod[3]=modStr[2];
  len=0, modNum=0, modStr[0]=modStr[1]=modStr[2]=' '; 
}

//================================================================================================

void setup(){
  pinMode(VERMELHO_1, OUTPUT);
  pinMode(VERMELHO_2, OUTPUT);
  pinMode(VERMELHO_3, OUTPUT);
  pinMode(VERDE_1, OUTPUT);
  pinMode(VERDE_2, OUTPUT);
  pinMode(VERDE_3, OUTPUT);
  pinMode(AMARELO, OUTPUT);

  Serial.begin(9600);

  Wire.begin(5, 3);
  display_.begin();
  display_.setPowerSave(0);

  EEPROM.begin(16);
  delay(100);
  //EEPROM.write(SAVEDADDR, 0); //reset saved
  if (EEPROM.read(SAVEDADDR)) {
    HP = readInt(HPADDR);
    HPMAX = readInt(HPMAXADDR);
    loadFlags();
    computeLeds();
  }
  else {
    EEPROM.write(SAVEDADDR, 1), writeInt(HPADDR, HP), writeInt(HPMAXADDR, HPMAX);
    EEPROM.write(morrendoADDR, 0), EEPROM.write(mortoADDR, 0);
    EEPROM.write(sucessosADDR, 0), EEPROM.write(falhasADDR, 0);
  }

  drawClear();
  draw();
}

void loop(){

  char b = teclado.getKey();
  if (!b) {
    unsigned long prev = millis();
    while (digitalRead(UPDATEMAX)==1); 
    if ((millis()-prev)>100) b='U';
    while (digitalRead(UPDATEMAX)); 
  }

  if (morto) {
    if (b=='U')
      morto=morrendo=falhas=sucessos=modNum=0, saveFlags(), HP=HPMAX, writeInt(HPADDR, HP), modStr[0]=modStr[1]=modStr[2]=' ', confirma(' ');
    else
      return;
  }

  if (b) {
    switch (b) {
      case 'U':
        if (modNum>0) {
          HPMAX=modNum;
          if(HP>HPMAX) HP=HPMAX, writeInt(HPADDR, HP); 
          writeInt(HPMAXADDR, HPMAX);
          confirma('U');
        }
      break;
      case '-': 
        if (morrendo) { // '-' é usado para computar resultado
          if (modNum==1)       falhas+=2;
          else if (modNum<10)  falhas++;
          else if (modNum==20) morrendo=falhas=sucessos=0, HP=1, writeInt(HPADDR, HP);
          else                 sucessos++;
          if (falhas >= 3) morrendo=sucessos=0, morto=1;
          if (sucessos >= 3) morrendo=falhas=0;
          confirma((modNum<10)?'-':'+');
        }
        else {
          HP -= modNum; 
          if(HP<=0) {
            if(HP<=-HPMAX) falhas=3, morto=1;
            else morrendo=1; 
            HP=0;
          }
          writeInt(HPADDR, HP);
          confirma('-');
        }
      break;
      case '+': 
        if(modNum>0) {
          HP += modNum; if(HP>HPMAX) HP=HPMAX; 
          morrendo=falhas=sucessos=0;
          writeInt(HPADDR, HP);
          confirma('+');
        }
      break;
      default:
        len++;
        if(len>=4) len=0, modNum=0, modStr[0]=modStr[1]=modStr[2]=' ';
        else modNum*=10, modNum+=(b-'0'), modStr[0]=modStr[1], modStr[1]=modStr[2], modStr[2]=b;
    }
    
    saveFlags();
    EEPROM.commit();

    draw();
    computeLeds();

  }
}

//================================================================================================
