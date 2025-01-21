// Module E - Bomb Button Module
// This refers to the physical button on the bomb.
// The code handling this module is in Device.ino


//-----------------------------------------------------------------------------
void ModEPanelCallback(vp_channel event) 
{ 
  CommonCallback(event); 

  switch (event) 
  {
    case PanelConnected: ModEPanelInit(); break;
    default: break;
  }
}

//-----------------------------------------------------------------------------
void ModEPanelInit()
{
  CommonPanelInit();
  Panel.send(Graph, true);
  Panel.send(GraphLabel_1, F("E - Bomb Button Module"));
  Panel.send(GraphText, _Point(30, 180));
  Panel.send(GraphText, F("Bomb Button Module.\nPhysical Button\nnext to bomb counter display"));
  Panel.send(GraphDrawLine, F("$16PX"));
  // Panel.send(GraphDrawLine, F("$BLUE"));
  randomSeed(CommonSeed);

}