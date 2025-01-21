// Main panel only module

//-----------------------------------------------------------------------------
void NoModPanelCallback(vp_channel event) 
{ 
  CommonCallback(event); 

  switch (event) 
  {
    case PanelConnected: CommonPanelInit(); break;
    default: break;
  }
}


