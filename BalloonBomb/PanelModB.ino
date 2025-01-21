// Module B - Morse Code


// label, sleep, ocean, shape,   major, level, noise, clock, loose,   minor, trick, mines, stops, cheer



  const uint8_t KeywordCount = 5;
  uint8_t KeywordSet = 0;

  const char * Keyword[KeywordCount] =  { " -. --- .. ... .",     // noise
                                          " - .-. .. -.-. -.-",   // trick
                                          " .-. --- -... --- -",  // robot
                                          " ... - --- .--. ...",  // stops
                                          " .-.. . ...- . .-.." };// level   
  const char * Response[KeywordCount] = { "BH-89", "CM-94", "AC-34", "CO-96", "BG-78" };
  uint8_t KeywordIdx = 0;

//-----------------------------------------------------------------------------
void ModBPanelCallback(vp_channel event) 
{ 
  CommonCallback(event); 

  switch (event) 
  {
    case PanelConnected: ModBPanelInit(); break;
    case GraphInput_2: GetFreq(); break;
    case DynamicDisplay: SendMorse(); break;

    default: break;
  }
}

//-----------------------------------------------------------------------------
void ModBPanelInit()
{
  CommonPanelInit();
  Panel.send(Graph, true);
  Panel.send(GraphLabel_1, F("D - Morse code Module"));
  Panel.send(GraphInput_2, true);
  Panel.send(GraphInput_2, F(""));
  Panel.send(GraphInputLabel_2, F("response"));
  Panel.send(DynamicDisplay, 100);
  Panel.send(GraphGrid, true);
  Panel.send(GraphDrawLine, F("$16PX"));
  Panel.send(GraphDrawLine, F("$GREEN"));

  randomSeed(CommonSeed);
  KeywordSet = random(0,KeywordCount);
  KeywordIdx = 0;
}

//-----------------------------------------------------------------------------
void GetFreq()
{
  if(Panel.vpr_type == vp_type::vp_string)
  {
    Panel.send(DynamicDisplay, false);
    if(!strcmp(Panel.vpr_string, Response[KeywordSet]))
      ModuleSucces(MODB);
    else
    {
      ModuleFail(MODB);
      if(!Detonated) SwitchMod(MODB);
    }
  }
}


//-----------------------------------------------------------------------------
void SendMorse()
{
//  static uint32_t Timer = 0;

  static uint8_t Counter = 20;
  const int DotTime   =  2;
  const int DashTime  =  6;
  const int BitPause  =  2;
  const int CharPause =  6;
  const int WordPause = 30;
  static bool High = false;

  Counter--;
  if(High && Counter == 1) High = false;
  if(High)
    Panel.send(GraphValue_1, (uint8_t) 180);
  else
  {
    Panel.send(GraphValue_1, (uint8_t) 150);
    Panel.send(GraphDrawLine, F("$BLACK"));
    Panel.send(GraphDrawLine, _Line(115,100,155,100));
    Panel.send(GraphDrawLine, _Line(50,100,220,100));
    Panel.send(GraphDrawLine, F("$GREEN"));
    Panel.send(GraphDrawLine, _Line(134,100,136,100));
  }

  if(Counter > 0) return;

  KeywordIdx++;
  if( KeywordIdx == strlen(Keyword[KeywordSet]))
  {
    KeywordIdx = 0;
    Counter = WordPause;
    return;
  }

  if(Keyword[KeywordSet][KeywordIdx] == '.')
  {
    Panel.send(Beep, _Sound(1000, DotTime * 100));
    Panel.send(GraphDrawLine, _Line(115,100,155,100));
    Counter = DotTime + BitPause;
    High = true;
  }
  else if(Keyword[KeywordSet][KeywordIdx] == '-')
  {
    Panel.send(Beep, _Sound(1000, DashTime * 100));
    Panel.send(GraphDrawLine, _Line(50,100,220,100));
    Counter = DashTime + BitPause;
    High = true;      
  }
  else if(Keyword[KeywordSet][KeywordIdx] == ' ')
  {
    Counter = CharPause;      
  }
}






