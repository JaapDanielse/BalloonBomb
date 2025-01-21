// Module A - Circuit and wires


const uint8_t ColorsSize = 5;
const char * Colors[ColorsSize] = {"$RED", "$PINK", "$BLUE", "$GREEN", "$ORANGE"};
uint8_t ColorIdx = 0;

const char * MazeColor = "$DPURPLE";

const byte XOffset = 17; 
const byte YOffset = 18; //18
const byte MazeWidth = 11;
const byte MazeHeight = 9; //10

uint8_t Maze[MazeWidth][MazeHeight];

uint16_t ClickPoint = 0;
const uint8_t MazeCount = 5;
const uint8_t MazeSeed[MazeCount] = {8, 13, 26, 27, 28};
const bool CorrectExits[MazeCount][MazeWidth] = { { 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0},   // correct solution maze 0
                                                  { 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0},   // maze 1 
                                                  { 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0},   // maze 2
                                                  { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},   // maze 4
                                                  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0} }; // maze 5

bool MazenumSet = false;
uint8_t MazeNum = 0; // selected maze number (0 - MazeCount) will be set random

bool ClickedExits[MazeWidth];
enum tiles {cross, right, left, vert};

//-----------------------------------------------------------------------------
void ModAPanelCallback(vp_channel event) 
{ 
  CommonCallback(event); 

  switch (event) 
  {
    case PanelConnected: ModAPanelInit(); break;
    case GraphButton_1: CheckMaze(); break;

    case GraphClick:
      ClickPoint = Panel.vpr_uint;
      ExitClick();
      break;

    default: break;
  }
}

//-----------------------------------------------------------------------------
void ModAPanelInit()
{
  CommonPanelInit();
  Panel.send(Graph, true);
  Panel.send(GraphButton_1, F("send"));
  Panel.send(GraphLabel_1, F("A - Circuit Board Module"));
  if(!MazenumSet)
  {
    randomSeed(CommonSeed);
    MazeNum = random(0,MazeCount);
    MazenumSet = true;
  }
  else
  {
    if(++MazeNum == MazeCount) MazeNum = 0;
  }
  
  Panel.sendf(GraphLabel_2, F("seed %d"), MazeSeed[MazeNum]);
  for(byte i=0; i<MazeWidth; i++) 
  {
    ClickedExits[i] = false;
  }
  ModuleStat[MODA] = 0;
  ColorIdx = 0;
  GenerateMaze();
}

//----------------------------------------------------------------------------------
void CheckMaze()
{
  bool MazeOk = true;

  Panel.send(GraphDrawPixel, "$8PX");
  byte y = YOffset + (MazeHeight*20) + 10;

  for(byte i=0; i<MazeWidth; i++) 
  {
    if(CorrectExits[MazeNum][i] == ClickedExits[i])
    {
      if(ClickedExits[i])
      {
        Panel.send(GraphDrawPixel, F("$GREEN"));
        Panel.send(GraphDrawPixel, _Point(XOffset + (i*20) + 10, y));
      }
    }
    else
    {
      Panel.send(GraphDrawPixel, F("$RED"));
      Panel.send(GraphDrawPixel, _Point(XOffset + (i*20) + 10, y));
      MazeOk = false;
    }       
  }

  if(MazeOk) 
    ModuleSucces(MODA);
  else
  {
    ModuleFail(MODA);
    if(!Detonated) SwitchMod(MODA);
  } 

}


//----------------------------------------------------------------------------------
void ExitClick()
{
  if(ModuleStat[MODA] != 0) return;

  byte ClickX = highByte(ClickPoint); 
  byte ClickY = lowByte(ClickPoint);
  //bool AllExits = true;

  if (ClickY > YOffset + (MazeHeight*20) && 
      ClickY < YOffset + ((MazeHeight+1)*20) &&
      ClickX > XOffset && 
      ClickX < XOffset + (MazeWidth*20))
  {
    byte x = (ClickX - XOffset)/20;
    byte y = YOffset + (MazeHeight*20) + 10;

    Panel.send(GraphDrawPixel, "$8PX");
    ClickedExits[x] = !ClickedExits[x]; // toggle exit
    
    if(ClickedExits[x])
    {
      Panel.send(GraphDrawLine, F("$8PX"));
      Panel.send(GraphDrawLine, F("$DBLUE"));
      Panel.send(GraphDrawLine); // new line
      Panel.send(GraphDrawLine, _Point(XOffset + (x*20) + 10, y-10));
      Panel.send(GraphDrawLine, _Point(XOffset + (x*20) + 10, y+6));
      Panel.send(Beep, _Sound(360,100));
      Panel.send(Beep, _Sound(480,100));
    } 
    else
    {
      Panel.send(GraphDrawLine, F("$8PX"));
      Panel.send(GraphDrawLine, F("$YELLOW"));
      Panel.send(GraphDrawLine); // new line
      Panel.send(GraphDrawLine, _Point(XOffset + (x*20) + 10, y-10));
      Panel.send(GraphDrawLine, _Point(XOffset + (x*20) + 10, y+6));
      Panel.send(Beep, _Sound(480,100)); 
      Panel.send(Beep, _Sound(360,100));
    }
  }
}


//----------------------------------------------------------------------------------
void GenerateMaze()
{
  byte xp, yp;

  randomSeed(MazeSeed[MazeNum]);

  for (byte x=0; x<MazeWidth; x++)
  {
    DrawEntry((x*20)+XOffset, YOffset);
    for (byte y=0; y<MazeHeight; y++)
    {
      Maze[x][y] = (byte)random(0,4);
      xp = (x*20) + XOffset;
      yp = (y*20) + YOffset;
      switch (Maze[x][y])
      {
        case cross: DrawSquareCross(xp,yp); break;
        case right: DrawSquareRight(xp,yp); break;
        case left: DrawSquareLeft(xp,yp);  break;
        case vert: DrawSquareVert(xp,yp);  break;
        default: break;
      }
    }
    DrawEndBox((x*20)+XOffset, byte(YOffset+(MazeHeight*20)));
  }
  Panel.send(GraphDrawLine,_Line(XOffset,YOffset,XOffset,YOffset+(MazeHeight*20)+12));
  Panel.send(GraphDrawLine,_Line(XOffset+(MazeWidth*20),YOffset,XOffset+(MazeWidth*20),YOffset+(MazeHeight*20)+12));
  Panel.send(GraphText, _Point(160,65));
  Panel.send(GraphText, "TED-A451");
}


//----------------------------------------------------------------------------------
void DrawSquareCross(int x, int y)
{
  Panel.send(GraphDrawLine, F("$DPURPLE"));
  Panel.send(GraphDrawLine, F("$4PX"));
  Panel.send(GraphDrawLine); // new line
  Panel.send(GraphDrawLine, _Point(x+10,y));
  Panel.send(GraphDrawLine, _Point(x+10,y+20));
  Panel.send(GraphDrawLine); // new line  
  Panel.send(GraphDrawLine, _Point(x,y+10));
  Panel.send(GraphDrawLine, _Point(x+20,y+10));
  
  Panel.send(GraphDrawLine, F("$WHITE"));
  Panel.send(GraphDrawLine, F("$1PX"));   
  Panel.send(GraphDrawLine); // new line
  Panel.send(GraphDrawLine, _Point(x+6,y));
  Panel.send(GraphDrawLine, _Point(x+6,y+6));
  Panel.send(GraphDrawLine, _Point(x,y+6));
  
  Panel.send(GraphDrawLine); // new line
  Panel.send(GraphDrawLine, _Point(x+13,y));
  Panel.send(GraphDrawLine, _Point(x+13,y+6));
  Panel.send(GraphDrawLine, _Point(x+20,y+6));
  
  Panel.send(GraphDrawLine); // new line
  Panel.send(GraphDrawLine, _Point(x,y+13));
  Panel.send(GraphDrawLine, _Point(x+6,y+13));
  Panel.send(GraphDrawLine, _Point(x+6,y+20));
  
  Panel.send(GraphDrawLine); // new line
  Panel.send(GraphDrawLine, _Point(x+13,y+20));
  Panel.send(GraphDrawLine, _Point(x+13,y+13));
  Panel.send(GraphDrawLine, _Point(x+20,y+13));
}


//----------------------------------------------------------------------------------
void DrawSquareVert(int x, int y)
{
  Panel.send(GraphDrawLine, F("$DPURPLE"));
  Panel.send(GraphDrawLine, F("$4PX"));
  Panel.send(GraphDrawLine); // new line
  Panel.send(GraphDrawLine,_Point(x+10,y));
  Panel.send(GraphDrawLine,_Point(x+10,y+20));
 
  Panel.send(GraphDrawLine, F("$WHITE"));
  Panel.send(GraphDrawLine, F("$1PX"));
  Panel.send(GraphDrawLine); // new line
  Panel.send(GraphDrawLine, _Point(x+6,y));
  Panel.send(GraphDrawLine, _Point(x+6,y+20));
  
  Panel.send(GraphDrawLine); // new line
  Panel.send(GraphDrawLine, _Point(x+13,y));
  Panel.send(GraphDrawLine, _Point(x+13,y+20));
  
  Panel.send(GraphDrawLine); // new line
  Panel.send(GraphDrawLine, _Point(x,y+6));
  Panel.send(GraphDrawLine, _Point(x,y+13));
  
  Panel.send(GraphDrawLine); // new line
  Panel.send(GraphDrawLine, _Point(x+20,y+6));
  Panel.send(GraphDrawLine, _Point(x+20,y+13));
}


//----------------------------------------------------------------------------------
void DrawSquareLeft(int x, int y)
{
  Panel.send(GraphDrawLine, F("$DPURPLE"));
  Panel.send(GraphDrawLine, F("$4PX"));
  Panel.send(GraphDrawLine); // new line
  Panel.send(GraphDrawLine, _Point(x+10,y));
  Panel.send(GraphDrawLine, _Point(x+1,y+9));
  Panel.send(GraphDrawLine); // new line
  Panel.send(GraphDrawLine, _Point(x+20,y+10));
  Panel.send(GraphDrawLine, _Point(x+10,y+20));
     
  Panel.send(GraphDrawLine, F("$WHITE"));
  Panel.send(GraphDrawLine, F("$1PX"));
  Panel.send(GraphDrawLine); // new line
  Panel.send(GraphDrawLine, _Point(x,y+6));
  Panel.send(GraphDrawLine, _Point(x+6, y));
  
  Panel.send(GraphDrawLine); // new line
  Panel.send(GraphDrawLine, _Point(x+13, y));
  Panel.send(GraphDrawLine, _Point(x, y+13));
  
  Panel.send(GraphDrawLine); // new line
  Panel.send(GraphDrawLine, _Point(x+6,y+20));
  Panel.send(GraphDrawLine, _Point(x+20,y+6));
  
  Panel.send(GraphDrawLine); // new line
  Panel.send(GraphDrawLine, _Point(x+13,y+20));
  Panel.send(GraphDrawLine, _Point(x+20,y+13));
}


//----------------------------------------------------------------------------------
void DrawSquareRight(int x, int y)
{
  Panel.send(GraphDrawLine, F("$DPURPLE"));
  Panel.send(GraphDrawLine, F("$4PX"));
  Panel.send(GraphDrawLine); // new line
  Panel.send(GraphDrawLine, _Point(x+10,y));
  Panel.send(GraphDrawLine, _Point(x+20,y+10));
  Panel.send(GraphDrawLine); // new line
  Panel.send(GraphDrawLine, _Point(x+1,y+9));
  Panel.send(GraphDrawLine, _Point(x+10,y+20));
  
  Panel.send(GraphDrawLine, F("$WHITE"));
  Panel.send(GraphDrawLine, F("$1PX"));  
  Panel.send(GraphDrawLine); // new line
  Panel.send(GraphDrawLine, _Point(x,y+6));
  Panel.send(GraphDrawLine, _Point(x+13,y+19));
  
  Panel.send(GraphDrawLine); // new line
  Panel.send(GraphDrawLine, _Point(x,y+14));
  Panel.send(GraphDrawLine, _Point(x+6,y+20));
  
  Panel.send(GraphDrawLine); // new line
  Panel.send(GraphDrawLine, _Point(x+6,y));
  Panel.send(GraphDrawLine, _Point(x+19,y+13));
  
  Panel.send(GraphDrawLine); // new line
  Panel.send(GraphDrawLine, _Point(x+14,y));
  Panel.send(GraphDrawLine, _Point(x+20,y+6));
}


//----------------------------------------------------------------------------------
void DrawEndBox(int x, int y)
{
  Panel.send(GraphDrawLine, F("$8PX"));
  Panel.send(GraphDrawLine, F("$YELLOW"));
  Panel.send(GraphDrawLine); // new line
  Panel.send(GraphDrawLine, _Point(x+10, y));
  Panel.send(GraphDrawLine, _Point(x+10, y+16));
  
  Panel.send(GraphDrawLine, F("$WHITE"));
  Panel.send(GraphDrawLine, F("$1PX"));
  Panel.send(GraphDrawLine); // new line
  Panel.send(GraphDrawLine, _Point(x+6,y));
  Panel.send(GraphDrawLine, _Point(x+2,y+3));
  Panel.send(GraphDrawLine, _Point(x+2,y+15));
  Panel.send(GraphDrawLine, _Point(x+6,y+18));
  Panel.send(GraphDrawLine, _Point(x+13,y+18));
  Panel.send(GraphDrawLine, _Point(x+18,y+15));
  Panel.send(GraphDrawLine, _Point(x+18,y+3));
  Panel.send(GraphDrawLine, _Point(x+13,y));
}


//----------------------------------------------------------------------------------
void DrawEntry(int x, int y)
{
  Panel.send(GraphDrawLine, F("$WHITE"));
  Panel.send(GraphDrawLine, F("$1PX"));
  Panel.send(GraphDrawLine); // new line
  Panel.send(GraphDrawLine, _Point(x,y));
  Panel.send(GraphDrawLine, _Point(x+6,y));
  Panel.send(GraphDrawLine); // new line
  Panel.send(GraphDrawLine, _Point(x+13,y));
  Panel.send(GraphDrawLine, _Point(x+20,y));
  
  Panel.send(GraphDrawLine); // new line
  Panel.send(GraphDrawLine, F("$8PX"));
  Panel.send(GraphDrawLine, Colors[ColorIdx]);
  Panel.send(GraphDrawLine, _Point(x+10,y-16));
  Panel.send(GraphDrawLine, _Point(x+10,y));
  
  if(++ColorIdx == ColorsSize) ColorIdx = 0;

}
