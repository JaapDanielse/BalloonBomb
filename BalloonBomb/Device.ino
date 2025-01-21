// Device module
// Contains all hardware dependend handling

  const uint8_t PlugSeqCnt = 5;
  const uint8_t PlugSequence[2][PlugSeqCnt]       = { { 0b11111, 0b11101, 0b11001, 0b01001, 0b01101 },
                                                      { 0b11111, 0b01111, 0b01011, 0b01001, 0b01101 }};
  const uint8_t PlugConditions[2][PlugSeqCnt - 1] = { {         2,       0,       5,       0,       },
                                                      {         0,       2,       5,       0,       }}; 
  static uint8_t PrevPlugStatus = PlugSequence[0][0];
  static uint8_t CurrPlugStatus = PlugSequence[0][0];
  static uint8_t PlugSeqIdx = 0;


  //-----------------------------------------------------------------------------  
  void DeviceInit()
  {
    pinMode(detonatorOn, OUTPUT);
    digitalWrite(detonatorOn, LOW);
    Detonator.write(0); 
    Detonator.attach(detonatorPin);
    delay(500);
    pinMode(buzzer, OUTPUT);
    pinMode(Plug_1, INPUT_PULLUP);
    pinMode(Plug_2, INPUT_PULLUP);
    pinMode(Plug_3, INPUT_PULLUP);
    pinMode(Plug_4, INPUT_PULLUP);
    pinMode(Plug_5, INPUT_PULLUP);
    pinMode(StartButton, INPUT_PULLUP);
    DisplayInit();

    if(ReadPlugs() != 0b11111)
    {
      LedDisplay.displayString("Err");
      while(ReadPlugs() != 0b11111);
      delay(500);
    }
  }

//-----------------------------------------------------------------------------
void DisplayInit()
{
  Wire.begin(); //Join the bus as master
  LedDisplay.init();
  LedDisplay.displayOff();
  LedDisplay.displayString("");
  LedDisplay.setBrightness(TM1650_MAX_BRIGHT);
  LedDisplay.displayOn();
}


//-----------------------------------------------------------------------------
void SetupBomb()
{
    Sound = true;
    CounterValue = 9999;
    CounterDelay = 1000;
    StrikeCount = 0;
    Detonated = false;
    Disarmed  = false;
    Once = true;
    Blink = true;
    SequenceTimer = 0;
    PrevPlugStatus = PlugSequence[SeqSet][0];
    CurrPlugStatus = PlugSequence[SeqSet][0];
    PlugSeqIdx = 0;
    if(ModuleStat[1] != 3) ModuleStat[1] = 0;
    if(ModuleStat[2] != 3) ModuleStat[2] = 0;
    if(ModuleStat[3] != 3) ModuleStat[3] = 0;
    if(ModuleStat[4] != 3) ModuleStat[4] = 0;
    if(ModuleStat[5] != 3) ModuleStat[5] = 0;
    SwitchMod(0);
    Analyze = true;
}

  //-----------------------------------------------------------------------------  
  void Counter()
  {
    static uint32_t CounterTimer = 0;
    char DisplayBuf[5];
    
    if(CounterTimer > millis()) return;
    CounterTimer = millis() + CounterDelay;
    if(CounterValue > 0) CounterValue--;
    sprintf(DisplayBuf, "%04d", CounterValue);
    LedDisplay.displayString(DisplayBuf);
    // Panel.sendf(Display_3, F("%04d"), CounterValue);
    CounterTick();
    
    return;
  }

//-----------------------------------------------------------------------------
  void CounterTick()
  {
    if(!Sound) return;
    tone(buzzer, 1000);
    delay(20);
    noTone(buzzer);
  }  

//-----------------------------------------------------------------------------
void ModuleFailSound()
{
  if(!Sound) return;
  for(int i=0; i<3; i++)
  {
    tone(buzzer, 1000);
    delay(250);
    noTone(buzzer);
  }
}  

//-----------------------------------------------------------------------------
void CheckStartButton()
{
  const uint8_t EColorsSize = 5;
  const char * EColors[EColorsSize] = {"$BLUE", "$ORANGE", "$YELLOW", "$GREEN", "$PINK"};
  static uint8_t EColorIdx = 0;
  
  static bool WasLow = false;

  char DisplayBuf[5];

  if(digitalRead(StartButton) == LOW) // button goes low
  { 
    if(!PanelSyncRequest(PanelConnected)) // if panel not connected
    {
      SetupBomb(); // reset bomb
      BombLevel = 1;
      SaveSettings();
      LedDisplay.displayString("-L1-");
      delay(1000);
      return; 
    }
    else // if connected
    {
      if(PanelMode != MODE || WasLow || Detonated || Disarmed) return; // if not in module E return
      
      WasLow = true; // remember button was low.
      EColorIdx = random(0,EColorsSize);
      Panel.send(GraphDrawLine,EColors[EColorIdx]);
      Panel.send(GraphDrawLine, _Line(50,100,220,100));
    }
  }
  else
  {
    if(PanelMode != MODE) return; // if not in module E return
    if(WasLow)
    {
      WasLow = false;
      sprintf(DisplayBuf, "%04d", CounterValue);
      for(uint8_t i=0; i<5; i++)
      {
        if(DisplayBuf[i] == '7' && EColorIdx==0) { ModuleSucces(MODE); return; }
        if(DisplayBuf[i] == '4' && EColorIdx==1) { ModuleSucces(MODE); return; }
        if(DisplayBuf[i] == '3' && EColorIdx==2) { ModuleSucces(MODE); return; }
        if(DisplayBuf[i] == '1' && (EColorIdx==3 || EColorIdx==4) ) { ModuleSucces(MODE); return; }
      }
      Panel.send(GraphDrawLine, F("$BLACK"));
      Panel.send(GraphDrawLine, _Line(50,100,220,100));  
      ModuleFail(MODE);
      if(!Detonated) SwitchMod(MODE);
    }
  }
}

//-----------------------------------------------------------------------------
void CheckPlugs()
{
  if(Analyze) return;
  if(ModuleStat[MODD] != 0) return;
 
  PrevPlugStatus = CurrPlugStatus;
  CurrPlugStatus =  ReadPlugs();
  
  if(PrevPlugStatus != CurrPlugStatus)
  {
    if(PrevPlugStatus == PlugSequence[SeqSet][PlugSeqIdx] && CurrPlugStatus == PlugSequence[SeqSet][PlugSeqIdx + 1])
    {
      if((PlugConditions[SeqSet][PlugSeqIdx] != 0))
      {
        if (CounterValue % PlugConditions[SeqSet][PlugSeqIdx] != 0) 
        {
          ModuleFail(MODD); 
          Detonate();
        }
      }
      PlugSeqIdx++;
      if (PlugSeqIdx + 1 == PlugSeqCnt) ModuleSucces(MODD); 
    }
    else
    { 
      ModuleFail(MODD); 
      Detonate();
    }
    delay(50);
  }
  if(PanelMode == MODD)
  {
    for(int i=0; i<5; i++)
    {
      if(bitRead(CurrPlugStatus, i))  Panel.send(GraphDrawPixel, F("$WHITE")); else Panel.send(GraphDrawPixel, F("$DBLUE"));
      if(ModuleStat[MODD] == 1) Panel.send(GraphDrawPixel, F("$RED")); 
      if(ModuleStat[MODD] == 2) Panel.send(GraphDrawPixel, F("$GREEN")); 
      Panel.send(GraphDrawPixel, _Point((uint8_t) (45 + ((4-i) * 40)), (uint8_t) 60));
    }
  }
}

//-----------------------------------------------------------------------------
uint8_t ReadPlugs()
{
  uint8_t PlugStatus = 0;

  if(digitalRead(Plug_1) == LOW) bitSet(PlugStatus, 4); else  bitClear(PlugStatus, 4);
  if(digitalRead(Plug_2) == LOW) bitSet(PlugStatus, 3); else  bitClear(PlugStatus, 3);
  if(digitalRead(Plug_3) == LOW) bitSet(PlugStatus, 2); else  bitClear(PlugStatus, 2);
  if(digitalRead(Plug_4) == LOW) bitSet(PlugStatus, 1); else  bitClear(PlugStatus, 1);
  if(digitalRead(Plug_5) == LOW) bitSet(PlugStatus, 0); else  bitClear(PlugStatus, 0);
  
  return PlugStatus;
}

//-----------------------------------------------------------------------------
void Detonate()
{
  const int ServoRest = 0;
  const int ServoSting = 30;

  Detonated = true;

  Detonator.write(ServoRest); 
  delay(50);
  digitalWrite(detonatorOn, HIGH);
  delay(250);
  Detonator.write(ServoSting);
  delay(250);
  Detonator.write(ServoRest); 
  delay(500);
  digitalWrite(detonatorOn, LOW);
  DisplayInit();

}

//-----------------------------------------------------------------------------
void DisarmSequence()
{
  char DisplayBuf[5];

  if(SequenceTimer > millis()) return;
  SequenceTimer = millis() + 1000;

  if(Once)
  {
    Panel.send(Display_4, false);
    Panel.send(Display_2, F("$GREEN"));
    Panel.send(Display_2, F("$NORMAL"));
    Panel.send(Display_2, F("-- device disarmed! --     "));
    Panel.send(Button_13, F("reset"));
    Panel.send(Graph, false);
    if(BombLevel != 0)
    {
      BombLevel++;
      SaveSettings();
    }
    Panel.send(Button_13, F("reset"));
    Once = false;
  }

  if(Blink)
  {

    sprintf(DisplayBuf, "%04d", CounterValue);
    LedDisplay.displayString(DisplayBuf);
  }
  else
  {
    LedDisplay.displayString("    ");
  }
  Blink = !Blink;
}


//-----------------------------------------------------------------------------
void DetonatedSequence()
{
  if(SequenceTimer > millis()) return;
  SequenceTimer = millis() + 1000;

  if(Once)
  {
    Panel.send(Display_4, false);
    Panel.sendf(Display_2, F("$RED"));
    Panel.sendf(Display_2, F("$NORMAL"));
    Panel.sendf(Display_2, F("-- device detonated! --     "));
    Panel.send(Button_13, F("reset"));
    Panel.send(Graph, false);
    Once = false;
  }

  if(Blink)
  {
    LedDisplay.displayString("----");
  }
  else
  {
    LedDisplay.displayString("    ");
  }
  Blink = !Blink;
}
