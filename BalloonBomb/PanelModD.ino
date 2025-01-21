// Module D - Wire Connection Module
// This refers to the physical connection box on the back of the bomb.
// The code handling this module is in Device.ino


//-----------------------------------------------------------------------------
void ModDPanelCallback(vp_channel event) 
{
  CommonCallback(event); 
 
  switch (event) 
  {
    case PanelConnected: ModDPanelInit(); break;
    default: break;
  }
}

//-----------------------------------------------------------------------------
void ModDPanelInit()
{
  CommonPanelInit();
  Panel.send(Graph, true);
  //Panel.send(GraphButton_1, F("Suc-\nsess"));
  //Panel.send(GraphButton_2, F("Fail"));
  Panel.send(GraphLabel_1, F("D - Wire Connection Module"));
  Panel.send(GraphText, _Point(30, 180));
  Panel.send(GraphText, F("5-Wire module.\nPhysical connection panel\nbomb rear."));
  Panel.send(GraphText, _Point(30, 130));
  Panel.send(GraphText, F("$YELLOW"));
  Panel.send(GraphText, F("WARNING !!\nZero strike module\ndevice will explode on failure directly!"));
  Panel.send(GraphDrawPixel, F("$16PX"));

}