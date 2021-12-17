
const unsigned char PN9[50]=
{
  0xFF,0xE1,0x1D,0x9A,0xED,0x85,0x33,0x24,0xEA,0x7A, // 0 -  9
  0xD2,0x39,0x70,0x97,0x57,0x0A,0x54,0x7D,0x2D,0xD8, //10 - 19
  0x6D,0x0D,0xBA,0x8F,0x67,0x59,0xC7,0xA2,0xBF,0x34, //20 - 29
  0xCA,0x18,0x30,0x53,0x93,0xDF,0x92,0xEC,0xA7,0x15, //30 - 39
  0x8A,0xDC,0xF4,0x86,0x55,0x4E,0x18,0x21,0x40,0xC4  //40 - 49
};

//*********************************************************************
void DWData(unsigned char *pData, unsigned char Size)
{
  unsigned char i;
  
  if(Size>sizeof(PN9))
    return;
  
  for(i=0;i<Size;i++)
  {
    pData[i]^=PN9[i]; //insert array of whitening codes here
  }
}

//************************************************************************************
unsigned char IsDWRequired(unsigned char *pData, unsigned char Size)
{
  unsigned char i;
  unsigned char DC_0=0,DC_1=0;
    unsigned char MaxDCChars = 16;
  
  for(i=0;i<Size;i++)
  {
    DC_0=(pData[i]==0x00)?DC_0+1:0;
    DC_1=(pData[i]==0xFF)?DC_1+1:0;
      
    if((DC_0>=MaxDCChars)||(DC_1>=MaxDCChars))
      return(1);
  } 
  return(0);
}

