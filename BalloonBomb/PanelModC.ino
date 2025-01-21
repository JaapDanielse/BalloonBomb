// Module C - Button Order Module


const char * BColors[] = {"$BLACK","$BROWN"};

//const char * ButtonText[]  = {"YES","NO","OK","RIGHT","LEFT","MIDDLE","TOP","BOTTOM","FIRST","HOLD","READY","SEND"};
// label number                 0     1    2     3      4       5       6       7       8       9     10      11
// Indicated button             1     2    3     4      5       6       1       2       3       4      5       6

 const char * ButtonText[] = {"UP","DOWN","LEFT","RIGHT","STOP","ADD","PLUS","START","ENTER","DEL","OFF","BACK"};
// label number                  0    1      2      3       4      5     6      7       8       9     10    11              
// Indicated button              1    2      3      4       5      6     1      2       3       4      5     6

//const char * ButtonText[]  = {"AA-12","AB23","AC34","AD-45","AE56","AF67","AG-78","AH89","BA12","BB-23","BC34","BD45"};
// label number                0       1      2      3       4      5      6      7       8       9      10     11
// Indicated button            1       2      3      4       5      6      1      2       3       4       5      6
uint8_t Disp2Butt[7];

uint8_t StatusCount = 0;

// struct GBStruct
// {
//   uint8_t x;
//   uint8_t y;
//   uint8_t size;
//   uint8_t color;
// };

const int GraphButtonsSize = 7;

GBStruct GraphButtons[GraphButtonsSize] = 
{
  { 10, 170, 11, 0 },
  { 10, 110,  5, 1 },
  { 10,  70,  5, 1 },
  { 10,  30,  5, 1 },
  {105, 110,  5, 1 },
  {105,  70,  5, 1 },
  {105,  30,  5, 1 }
};

//-----------------------------------------------------------------------------
void ModCPanelCallback(vp_channel event) 
{
  CommonCallback(event); 
 
  switch (event) 
  {
    case PanelConnected: ModCPanelInit(); break;

    case GraphButton_1:
      Panel.send(Graph, F("$CLEAR"));
      Panel.send(Display_1, "");
      Panel.send(Display_2, "");
      CreateButtons(GraphButtons);
      DrawStatus();
      break;

    case GraphClick: 
      if(CheckClickedButton()) 
      {
        StatusCount++;
        DrawStatus();
        if(StatusCount == 3)
        {
           ModuleSucces(MODC);
        }
        else
        {
          Panel.send(Graph, F("$CLEAR"));
          CreateButtons(GraphButtons);
          DrawStatus();
        }
      }
      else
      {
        ModuleFail(MODC);
        if(!Detonated) SwitchMod(MODC);
      }
      break;
 
    default: break;
  }
}


//-----------------------------------------------------------------------------
void ModCPanelInit()
{
  CommonPanelInit();
  Panel.send(Graph, true);
  Panel.send(GraphLabel_1, F("C - Button Order Module"));
  randomSeed(millis());
  CreateButtons(GraphButtons);
  StatusCount = 0;
  DrawStatus();
}


//--------------------------------------------------------------------------------
bool CheckClickedButton()
{
  int8_t ClickedButton  = 0;
  int8_t IndButtonLabel = 0;

  ClickedButton = GraphButtonClick(GraphButtons); // get the clicked button
  if(ClickedButton == -1 || ClickedButton == 0) return false; 

  IndButtonLabel = Disp2Butt[(Disp2Butt[0] % 6) + 1]; // label of the button indicated by the display

  for(int i=IndButtonLabel+1; i<12; i++) // loop over the labels after the start label
  {
    for(int j=1; j<7; j++) // loop over the buttons (skip display)
    {
      if(Disp2Butt[j] == i) // button j has a label in the list
      {
        if(ClickedButton == j) return true; else return false;
      }
    }
  }    
  for(int i=0; i<IndButtonLabel; i++) // loop over the labels before the start label
  {
    for(int j=1; j<7; j++)
    {
      if(Disp2Butt[j] == i) // button j has a label in the list
      {
        if(ClickedButton == j) return true; else return false; 
      }
    }
  }    
  return false;
}


//--------------------------------------------------------------------------------
void CreateButtons(GBStruct * Buttons)
{
  bool NewWord = false;

  for(int i=0; i < GraphButtonsSize; i++) // assign a word to all buttons
  {
    do 
    {
       Disp2Butt[i] = random(0, 12);
       NewWord = true;
       for(int j=0; j < i; j++)
       {
         if(Disp2Butt[j] == Disp2Butt[i]) NewWord = false; // word already used
       }      
    } while (!NewWord);  // try again

    GraphButtonDraw(Buttons[i].x, Buttons[i].y, Buttons[i].size, BColors[Buttons[i].color], ButtonText[Disp2Butt[i]]);
    delay(150);
  }
}

//--------------------------------------------------------------------------------
void GraphButtonDraw(uint8_t x, uint8_t y, uint8_t size, const char * color, const char * text)
{
   Panel.send(GraphDrawPixel, "$16PX");
   Panel.send(GraphDrawPixel, color);

  for(int j=0; j<2; j++)
  {
    for(int i = 0; i<size; i++)
    {
      Panel.send(GraphDrawPixel, _Point(x+8+(i*16),y+8+(j*16)));
    }
  }

   Panel.send(GraphDrawLine, F("$1PX"));
   Panel.send(GraphDrawLine, F("$WHITE"));
   Panel.send(GraphDrawLine);
   Panel.send(GraphDrawLine, _Point(x,y));
   Panel.send(GraphDrawLine, _Point(x,y+32));
   Panel.send(GraphDrawLine, _Point(x+(size*16),y+32));
   Panel.send(GraphDrawLine, _Point(x+(size*16),y));
   Panel.send(GraphDrawLine, _Point(x,y));

   Panel.send(GraphText, F("$WHITE"));
   Panel.send(GraphText, _Point(x+8,y+16+6));
   Panel.send(GraphText, text);
}


//--------------------------------------------------------------------------------
int8_t GraphButtonClick(GBStruct * Buttons)
{
  for(int i=0; i < GraphButtonsSize; i++)
  {
    if(    highByte(Panel.vpr_uint) > Buttons[i].x
        && highByte(Panel.vpr_uint) < Buttons[i].x + (Buttons[i].size * 16) 
        && lowByte(Panel.vpr_uint)  > Buttons[i].y 
        && lowByte(Panel.vpr_uint)  < Buttons[i].y + 32
      ) return i;
  }
  return -1;
}


//--------------------------------------------------------------------------------
void DrawStatus()
{
   Panel.send(GraphDrawPixel, F("$16PX"));
   Panel.send(GraphDrawPixel, F("$GRAY"));

   Panel.send(GraphDrawPixel, _Point(220, 190));
   Panel.send(GraphDrawPixel, _Point(220, 150));
   Panel.send(GraphDrawPixel, _Point(220, 110));

   Panel.send(GraphDrawPixel, F("$GREEN"));
   if (StatusCount > 0)  Panel.send(GraphDrawPixel, _Point(220, 110));
   if (StatusCount > 1)  Panel.send(GraphDrawPixel, _Point(220, 150));
   if (StatusCount > 2)  Panel.send(GraphDrawPixel, _Point(220, 190));

}










