//Settings Module

const uint32_t SettingsID = 00021;

struct BombSettings
{
  uint8_t  BombLevel;
  uint16_t CounterStart;
  uint8_t  ModuleStat[ModuleCount + 1]; 

} Settings;


//-----------------------------------------------------------------------------------------------
void SaveSettings()
{
  Settings.BombLevel = BombLevel;
  Settings.CounterStart = CounterStart;
  for(int i=0; i<ModuleCount + 1; i++)
  {
    Settings.ModuleStat[i] = ModuleStat[i];
  }
  WriteStructEEPROM(Settings, SettingsID);
}


//-----------------------------------------------------------------------------------------------
bool ReadSettings()
{
  if ( ReadStructEEPROM(Settings, SettingsID))
  {
    BombLevel = Settings.BombLevel;
    CounterStart = Settings.CounterStart;
    for(int i=0; i<ModuleCount + 1; i++)
    {
      ModuleStat[i] = Settings.ModuleStat[i];
    }
    return true;
  }
  return false;
}