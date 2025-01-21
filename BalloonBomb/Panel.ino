// Panel Dispatcher
// And common panel code

//-----------------------------------------------------------------------------
void PanelCallback(vp_channel event) 
{ 
  switch (PanelMode)
  {
    case NOMOD: NoModPanelCallback(event); break;
    case MODA:  ModAPanelCallback(event); break;
    case MODB:  ModBPanelCallback(event); break;
    case MODC:  ModCPanelCallback(event); break;
    case MODD:  ModDPanelCallback(event); break;
    case MODE:  ModEPanelCallback(event); break;
    default: break;
  }
}

//-----------------------------------------------------------------------------
void CommonPanelInit()
{
  Connected = true;

  Panel.send(ApplicationName, F("IΔ Timed Expolsive Device Analyzer"));
  Panel.send(ApplicationName, F("$SMALL"));
  Panel.send(Display_4, F("$10PT"));
  Panel.send(Display_4, F("$BLACK"));

  if(Analyze)
  {
    Panel.send(Display_1, F("analyzing device...     "));
    if(ReadPlugs() != 0b11111)
    {
      Panel.sendf(Display_1, F("$RED"));
      Panel.sendf(Display_1, F("-- Wire panel not reset --"));
      while(ReadPlugs() != 0b11111);
      delay(500);
    }
    delay(2000);
    randomSeed(micros());
    CommonSeed = random(0,255);
    Panel.sendf(MonitorLog, "Common seed %d", CommonSeed);
    RelVer = random(10,30);
    SeqSet = 0;
    if(RelVer%2 == 0) SeqSet = 1;
    SelectModules();
    Analyze = false;
  }

  uint8_t ModulesUsed = 0;
  for(uint8_t i=1; i<ModuleCount + 1; i++)
  {
    if(ModuleStat[i] != 3) ModulesUsed++;
  }
  Panel.send(Display_1, F("$WHITE"));
  Panel.sendf(Display_1, F("%d module device (L%d)      "), ModulesUsed, BombLevel);
  Panel.send(Display_2, F("$BLACK"));
  Panel.send(Display_2, F("$10PT"));
  Panel.sendf(Display_2, F("type: TED-A451\nrelease V2.%d"), RelVer);

  Panel.send(Button_4, F("mod\nA"));
  SetModuleControl(MODA, Led_2, Button_4);
  Panel.send(Button_5, F("mod\nB"));
  SetModuleControl(MODB, Led_3, Button_5);
  Panel.send(Button_6, F("mod\nC"));
  SetModuleControl(MODC, Led_4, Button_6);
  Panel.send(Button_7, F("mod\nD"));
  SetModuleControl(MODD, Led_5, Button_7);
  Panel.send(Button_8, F("mod\nE"));
  SetModuleControl(MODE, Led_6, Button_8);

  Panel.send(Button_12, F("deto\nnate"));
  Panel.send(Button_12, F("$RED"));
  Panel.send(Button_14, F("sound"));
  if(Sound) Panel.send(Led_8, F("$GREEN")); else Panel.send(Led_8, F("$OFF"));

  Panel.send(MonitorInput_1, true);
  Panel.send(MonitorInputLabel_1, "level");
  Panel.send(MaxMonitorInput_1, (uint8_t) 15);
  Panel.send(MonitorInput_1, BombLevel);

  if(BombLevel == 0)
  {
    Panel.send(MonitorInput_2, true);
    Panel.send(MonitorInputLabel_2, "counter");
    Panel.send(MaxMonitorInput_2, (uint16_t) 9999);
    Panel.send(MonitorInput_2, CounterValue);
  
    Panel.send(MonitorInput_3, true);
    Panel.send(MonitorInputLabel_3, "modules");
    
    char ModuleList[ModuleCount + 1 + 1] = "";
    int j=0;
    for(int i=1; i<ModuleCount + 1; i++)
    {
      if(ModuleStat[i] != 3) ModuleList[j++] = 65 - 1 + i;
    }
    ModuleList[j]= 0;
    Panel.send(MaxMonitorInput_3, ModuleCount);
    Panel.send(MonitorInput_3, ModuleList);
  }

  ShowStrikes();
}


//-----------------------------------------------------------------------------
void CommonCallback(vp_channel event) 
{ 
  switch (event) 
  {
    case Button_4:  SwitchMod(MODA); break;
    case Button_5:  SwitchMod(MODB); break;
    case Button_6:  SwitchMod(MODC); break;
    case Button_7:  SwitchMod(MODD); break;
    case Button_8:  SwitchMod(MODE); break;
    case Button_12: Detonate(); break;
    case Button_13: ResetDevice(); break;
    case Button_14: ToggleSound(); break;
    case MonitorInput_1: SetLevel(); break;
    case MonitorInput_2: SetCount(); break;
    case MonitorInput_3: SetModules(); break;
    default: break;
  }
}


//-----------------------------------------------------------------------------
void ResetDevice()
{
  SetupBomb();
  Panel.send(Reset); // reset the panel
}


//-----------------------------------------------------------------------------
void SelectModules()
{
  uint8_t From = 0;
  uint8_t Upto = 0;
  float   Timefactor = 1.0;
  uint8_t Modules = 0;
  uint8_t Module = 0;

  if(BombLevel == 0) 
  {
    CounterValue = CounterStart;
    return;
  }
  
  if(BombLevel == 1) { From = 2; Upto = 3; Timefactor = 3.0;} 
  if(BombLevel == 2) { From = 2; Upto = 4; Timefactor = 2.0;} 
  if(BombLevel == 3) { From = 3; Upto = 5; Timefactor = 1.5;} 
  if(BombLevel >= 4) { From = 4; Upto = 6; Timefactor = 1.0 - ((BombLevel - 4) * 0.05);} 
  
  randomSeed(CommonSeed);
  Modules = random(From, Upto);

  Panel.sendf(MonitorLog, F("Level: %d, From: %d, Upto: %d, Modules: %d "), BombLevel, From, Upto, Modules);

  for(uint8_t i=1; i < ModuleCount + 1; i++) { ModuleStat[i] = 3; } // switch all modules off
  
  while (Modules != 0)
  {
    Module = random(1, ModuleCount + 2);
    if( ModuleStat[Module] != 3) continue;
    ModuleStat[Module] = 0;
    Panel.sendf(MonitorLog, F("Module: %d"), Module);
    Modules--;
  }

  CounterStart = 0;

  for(uint8_t i=1; i < ModuleCount + 1; i++) 
  { 
    if(ModuleStat[i] != 3) CounterStart += (uint16_t) ((float) ModuleTime[i] * Timefactor);
  } 
  
  Panel.sendf(MonitorLog, F("TimeFactor: %s, Counter %d"), _FString(Timefactor,3,1), CounterStart);
 
  CounterValue = CounterStart;
}


//-----------------------------------------------------------------------------
void SetModuleControl(int16_t Module, uint8_t Led, uint8_t Button)
{
  if(ModuleStat[Module] == 0) Panel.send(Led, F("$OFF"));
  if(ModuleStat[Module] == 1) Panel.send(Led, F("$RED"));
  if(ModuleStat[Module] == 2) Panel.send(Led, F("$GREEN"));
  if(ModuleStat[Module] == 3) 
  { 
    Panel.send(Led, F("$GRAY"));
    Panel.send(Button, F("$GRAY"));
  } 
}


//-----------------------------------------------------------------------------
void ToggleSound()
{
  Sound = !Sound;
  if(Sound)   Panel.send(Led_8, F("$GREEN")); else Panel.send(Led_8, F("$OFF"));
}


//-----------------------------------------------------------------------------
void SwitchMod(int16_t Module)
{
  if(PanelMode == Module) Module = NOMOD;

  if(ModuleStat[Module] != 2 && ModuleStat[Module] != 3 && !Detonated) 
  {
    PanelMode = Module; // update the mode
    Panel.send(Reset); // reset the panel
  }
} 


//-----------------------------------------------------------------------------
void ModuleFail(int16_t Module)
{
  ModuleFailSound();
  ModuleStat[Module] = 1;
  SetModuleControl(Module, ModuleLed[Module], 0);
  CounterDelay = CounterDelay - 250;
  StrikeCount++;
  ShowStrikes();
  if (StrikeCount == 3) Detonate();
} 


//-----------------------------------------------------------------------------
void ShowStrikes()
{
  if (StrikeCount == 1) Panel.send(Display_4, F("$YELLOW"));
  else if (StrikeCount == 2) Panel.send(Display_4, F("$ORANGE"));
  if (StrikeCount != 3)
    Panel.sendf(Display_4, F("strike count: %d    \n"), StrikeCount);
}


//-----------------------------------------------------------------------------
void ModuleSucces(int16_t Module)
{
  uint8_t ModulesDone = 0;

  ModuleStat[Module] = 2;
  SetModuleControl(Module, ModuleLed[Module], 0);
  for(int i = 1; i < ModuleCount + 1; i++)
  {
    if(ModuleStat[i] == 2 || ModuleStat[i] == 3) ModulesDone++;
  }
  if (ModulesDone == ModuleCount)
  {
    Disarmed = true;
  }
  if(!Disarmed) SwitchMod(NOMOD);
} 


//------------------------------------------------------------------------------
void SetLevel() // set level manual
{
  if(Panel.vpr_type==vp_type::vp_byte)  
  { 
    BombLevel = (uint8_t) Panel.vpr_byte;
    SaveSettings();
    SetupBomb();
    CounterValue = CounterStart;
    Panel.send(Reset); // reset the panel
  }
}


//------------------------------------------------------------------------------
void SetCount() // set count manual
{
  if(Panel.vpr_type==vp_type::vp_uint)  
  { 
    CounterStart = Panel.vpr_uint;
    SaveSettings();
    SetupBomb();
    CounterValue = CounterStart;
    Panel.send(Reset); // reset the panel
  }
}


//------------------------------------------------------------------------------
void SetModules() // set module selection manual (caps A-E)
{
   if(Panel.vpr_type==vp_type::vp_string)
   {
    
    for(uint8_t i=1; i < ModuleCount + 1; i++) { ModuleStat[i] = 3; }

    for(uint8_t i=0; i < (uint8_t)strlen(Panel.vpr_string); i++)
    {
      if(Panel.vpr_string[i]=='A') ModuleStat[1]=0;
      if(Panel.vpr_string[i]=='B') ModuleStat[2]=0;
      if(Panel.vpr_string[i]=='C') ModuleStat[3]=0;
      if(Panel.vpr_string[i]=='D') ModuleStat[4]=0;
      if(Panel.vpr_string[i]=='E') ModuleStat[5]=0;
    }
    SaveSettings();
    SetupBomb();
    CounterValue = CounterStart;
    Panel.send(Reset); // reset the panel
  }
}

