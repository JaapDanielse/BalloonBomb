// BalloonBomb V2.16 - Jaap Daniëlse 2025 
// MIT License - Copyright (c) 2025 Jaap Daniëls
// Inspired by / based on "Keep talking and nobody explodes"
// but with a physical implementation

  #include <VirtualPanel.h>
  #include <Servo.h>
  #include <Wire.h>
  #include <TM1650.h>

  struct GBStruct  // Graph Button struct
  {
    uint8_t x; // lower left button edge x
    uint8_t y; // lover left button edge y
    uint8_t size; // button is 2 x 16 pixels high and size times pixels wide
    uint8_t color; // color index in color array
  };

  const int detonatorPin = 9; 
  const int detonatorOn = 10;
  const int buzzer = 11;
  const int Plug_5 = 2;
  const int Plug_4 = 3;
  const int Plug_3 = 4;
  const int Plug_2 = 5;
  const int Plug_1 = 6;
  const int StartButton = 12;

  Servo  Detonator;
  TM1650 LedDisplay;

  bool Connected = false;
  bool Analyze = true; // initialisation flag
  bool Sound = true; // sound toggle

  uint16_t CounterStart = 9999; // 300=5m. 360=6m. 420=7m. 480=8m. 540=9m. 600=10m. 900=15m. 1200=20m.
  uint16_t CounterValue = CounterStart; // Current Counter value
  uint16_t CounterDelay = 1000; // default Counter delay (1 sec)

  bool Detonated = false; // true if device has been detonated
  bool Disarmed  = false; // true if device has been disarmed
  uint32_t SequenceTimer = 0;
  bool Blink = true;
  bool Once  = true; 

  uint8_t StrikeCount = 0; // number of strikes (3=detonate)
  uint8_t CommonSeed = 0; // 
  uint8_t RelVer = 0; // release version (random defined)
  uint8_t SeqSet = 0; // wired module sequence selected (based on RelVer)

  uint8_t Complexity = 0; 

  const uint8_t  ModuleCount = 5;
  enum ModeList {NOMOD, MODA, MODB, MODC, MODD, MODE}; // mode enum
  int16_t PanelMode = NOMOD; // int used because it can increment

  uint8_t BombLevel = 1;
                                      //     -    A    B    C    D    E
  const uint8_t ModuleTime[ModuleCount+1] = {0, 110, 110,  70,  80,  60}; // module reference time (all mod. 430)

                                    //   -  A  B  C  D  E
  uint8_t ModuleStat[ModuleCount + 1] = {0, 0, 0, 0, 0, 0}; // module status 0 = wait, 1 = fail, 2 = solved, 3 = blocked
  const uint8_t ModuleLed[ModuleCount + 1]  = {0, Led_2, Led_3, Led_4, Led_5, Led_6}; //module led assignment

//-----------------------------------------------------------------------------
void setup() 
{
  DeviceInit();
  ReadSettings();
  Panel.begin();
}

//-----------------------------------------------------------------------------
void loop() 
{
  Panel.receive();
  CheckStartButton();
  if(!Detonated && !Disarmed) 
  { 
    Counter();
    CheckPlugs();
  }
  if(!Detonated && CounterValue == 0) Detonate();
  if(Disarmed)  DisarmSequence();
  if(Detonated) DetonatedSequence();
}

