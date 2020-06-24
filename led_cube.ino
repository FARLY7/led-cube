  #include <SPI.h>
  
  int latchPin = 9;
  int outputenable = 10;
  
  int currlevel = 0;
  
  byte layer[8];
  
  byte LED0[8][8];
  byte LED1[8][8];
  byte LED2[8][8];
  byte LED3[8][8];
  
  int BAM_Bit, BAM_Counter = 0; // Bit Angle Modulation variables to keep track of things
  
  void setup()
  {
    noInterrupts(); /* Halt interrupts until we setup Arduino */
    
    TCCR1A = B00000000;//Register A all 0's since we're not toggling any pins
    TCCR1B = B00001011;//bit 3 set to place in CTC mode, will call an interrupt on a counter match
    //bits 0 and 1 are set to divide the clock by 64, so 16MHz/64=250kHz
    TIMSK1 = B00000010;//bit 1 set to call the interrupt on an OCR1A match
    //OCR1A = 30; // you can play with this, but I set it to 30, which means:
    OCR1A = 30;
    //our clock runs at 250kHz, which is 1/250kHz = 4us
    //with OCR1A set to 30, this means the interrupt will be called every (30+1)x4us=124us, 
    // which gives a multiplex frequency of about 8kHz
    
    layer[0]=B10000000;
    layer[1]=B01000000;
    layer[2]=B00100000;
    layer[3]=B00010000;
    layer[4]=B00001000;
    layer[5]=B00000100;
    layer[6]=B00000010;
    layer[7]=B00000001;
    
    SPI.begin();
    SPI.setBitOrder(LSBFIRST);
    SPI.setDataMode(SPI_MODE0);// Mode 0 Rising edge of data, keep clock low
    SPI.setClockDivider(SPI_CLOCK_DIV2); 
    
    pinMode(latchPin, OUTPUT);
    pinMode(outputenable, OUTPUT);
  
    interrupts(); /* Begin the multiplexing */
  }
  
  
  void loop()
  {
    for(int i = 0 ; i < 5 ; i++)
      sweepingPerimeter(15);
      
    increasingSweep(15);
    
    for(int j = 0 ; j < 3 ; j++) {
      for(int i = 1 ; i <= 15 ; i++) { sphere(i); delay(150); }
      for(int i = 15 ; i > 0 ; i--) { sphere(i); delay(150); }
    }
    for(int j = 0 ; j < 3 ; j++) {   
      for(int i = 1 ; i <= 15 ; i++) { arrow(i); delay(150); }
      for(int i = 15 ; i > 0 ; i--) { arrow(i); delay(150); }
    }
    
    buildUp(15);
    
    for(int j = 0 ; j < 3 ; j++) {     
      for(int i = 15 ; i > 0 ; i--) { fullCube(i); delay(150); }
      for(int i = 1 ; i <= 15 ; i++) { fullCube(i); delay(150); }
    }
    
    for(int i = 0 ; i < 10 ; i++)
      perimeterDance(15);
  }
  
  
  /* ---------------------------------------------------------- */
  /* ------------------- INTERRUPT ROUTINE -------------------- */
  ISR(TIMER1_COMPA_vect)
  {
    if(currlevel == 8) currlevel = 0;
    
    digitalWrite(outputenable, HIGH);
    digitalWrite(latchPin, LOW);
    
    if(BAM_Counter == 8)
      BAM_Bit++;
    else if(BAM_Counter == 24)
      BAM_Bit++;
    else if(BAM_Counter == 56)
      BAM_Bit++;
  
    BAM_Counter++;
    
    switch(BAM_Bit)
    {   
      case 0: for(int i = 0 ; i < 8 ; i++) { SPI.transfer(LED0[currlevel][i]); } break; 
      case 1: for(int i = 0 ; i < 8 ; i++) { SPI.transfer(LED1[currlevel][i]); } break; 
      case 2: for(int i = 0 ; i < 8 ; i++) { SPI.transfer(LED2[currlevel][i]); } break;
      case 3: for(int i = 0 ; i < 8 ; i++) { SPI.transfer(LED3[currlevel][i]); } break;
    }
    SPI.transfer(layer[currlevel]);
    
    digitalWrite(latchPin, HIGH);
    digitalWrite(outputenable, LOW);
    
    if(BAM_Counter == 120)
    {
      BAM_Counter = 0;
      BAM_Bit = 0;
    }
    currlevel++;
  }
  /* ---------------------------------------------------------- */
  /* ---------------------------------------------------------- */
  
  
  
  /* ========================================================================================== */
  /* ===================================== USEFUL FUNCTIONS =================================== */
  void LED(int x, int y, int z, byte brightness)
  {
    bitWrite(LED0[y][z], 7 - x, bitRead(brightness, 0));
    bitWrite(LED1[y][z], 7 - x, bitRead(brightness, 1));
    bitWrite(LED2[y][z], 7 - x, bitRead(brightness, 2)); 
    bitWrite(LED3[y][z], 7 - x, bitRead(brightness, 3)); 
    // LEDs[y][z] |= (B10000000 >> x);
  }
  
  void drawCuboid(int x, int y, int z, int length, int breadth, int height, byte brightness)
  {
    /* ========================= */
    for(int i = 0 ; i <= length ; i++)
      LED(x + i, y, z, brightness);
    
    for(int i = 0 ; i <= length ; i++)
      LED(x + i, y, z + breadth, brightness);
      
    for(int i = 0 ; i <= length ; i++)
      LED(x + i, y + height, z, brightness);
      
    for(int i = 0 ; i <= length ; i++)
      LED(x + i, y + height, z + breadth, brightness);
      
    /* ======================== */
    for(int i = 0 ; i <= height ; i++)
      LED(x, y + i, z, brightness);
    
    for(int i = 0 ; i <= height ; i++)
      LED(x + length, y + i, z, brightness);
      
    for(int i = 0 ; i <= height ; i++)
      LED(x, y + i, z + breadth, brightness);
      
    for(int i = 0 ; i <= height ; i++)
      LED(x + length, y + i, z + breadth, brightness);
      
    /* ======================== */
    for(int i = 0 ; i <= breadth ; i++)
      LED(x, y, z + i, brightness);
    
    for(int i = 0 ; i <= breadth ; i++)
      LED(x, y + height, z + i, brightness);
      
    for(int i = 0 ; i <= breadth ; i++)
      LED(x + length, y, z + i, brightness);
      
    for(int i = 0 ; i <= breadth ; i++)
      LED(x + length, y + height, z + i, brightness);
  }
  
  void fillCuboid(int x, int y, int z, int length, int breadth, int height, byte brightness)
  {
    for(int k = 0 ; k < height ; k++)
      for(int j = 0 ; j < breadth ; j++) 
        for(int i = 0 ; i < length ; i++)
          LED(x + i, y + k, z + j, brightness);
  }
  
  void wipeCube() {
    for(int i = 0 ; i < 8 ; i++) {
      for(int j = 0 ; j < 8 ; j++) {
        LED0[i][j] = 0;
        LED1[i][j] = 0;
        LED2[i][j] = 0;
        LED3[i][j] = 0;
      }
    }
  }
  
  void fullCube(int brightness)
  {
    wipeCube();  
    
    if(bitRead(brightness, 0) == 1)
    {
       for(int i = 0 ; i < 8 ; i++)
        for(int j = 0 ; j < 8 ; j++)
        LED0[i][j] = B11111111;        
    }
    if(bitRead(brightness, 1) == 1)
    {
       for(int i = 0 ; i < 8 ; i++)
        for(int j = 0 ; j < 8 ; j++)
        LED1[i][j] = B11111111;        
    }
    if(bitRead(brightness, 2) == 1)
    {
       for(int i = 0 ; i < 8 ; i++)
        for(int j = 0 ; j < 8 ; j++)
        LED2[i][j] = B11111111;        
    }
    if(bitRead(brightness, 3) == 1)
    {
       for(int i = 0 ; i < 8 ; i++)
        for(int j = 0 ; j < 8 ; j++)
        LED3[i][j] = B11111111;        
    }
  }
  /* ========================================================================================== */
  /* ========================================================================================== */
  

  /* \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ ANIMATIONS \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */

  void buildUp(byte brightness)
  {
    wipeCube();
    for(int i = 0 ; i < 8 ; i++)
      for(int j = 0 ; j < 8 ; j++)
        for(int k = 0 ; k < 8 ; k++)
        {
          LED(k,i,j, brightness);
          delay(10);
        }
  }
  
  void perimeterDance(byte brightness)
  {
    wipeCube();  
    drawCuboid(0, 0, 0, 7, 7, 7, brightness);
    delay(110);
    
    wipeCube();
    drawCuboid(1, 1, 1, 5, 5, 5, brightness);
    delay(90);
    
    wipeCube();
    drawCuboid(2, 2, 2, 3, 3, 3, brightness);
    delay(90);
    
    wipeCube();
    drawCuboid(3, 3, 3, 1, 1, 1, brightness);
    delay(90);
    
    wipeCube();
    drawCuboid(2, 2, 2, 3, 3, 3, brightness);
    delay(90);
    
    wipeCube();
    drawCuboid(1, 1, 1, 5, 5, 5, brightness);
    delay(90);
  }
  
  
  void arrow(byte brightness)
  {
    wipeCube();
    for(int i = 3 ; i <= 4 ; i++)
    {
      LED(7, 4, i, brightness);
      
      LED(6, 3, i, brightness);
      LED(6, 4, i, brightness);
      LED(6, 5, i, brightness);
      
      LED(5, 2, i, brightness);
      LED(5, 3, i, brightness);
      LED(5, 4, i, brightness);
      LED(5, 5, i, brightness);
      LED(5, 6, i, brightness);
      
      LED(4, 3, i, brightness);
      LED(4, 4, i, brightness);
      LED(4, 5, i, brightness);
      LED(3, 3, i, brightness);
      LED(3, 4, i, brightness);
      LED(3, 5, i, brightness);
      LED(2, 3, i, brightness);
      LED(2, 4, i, brightness);
      LED(2, 5, i, brightness);
    
      LED(1, 3, i, brightness);
      LED(1, 4, i, brightness);
      LED(1, 5, i, brightness);
      LED(0, 3, i, brightness);
      LED(0, 4, i, brightness);
      LED(0, 5, i, brightness);
    }
  }
  
  
  void sweepingPerimeter(byte brightness)
  {
    wipeCube();
    for(int i = 0 ; i <= 7 ; i++)
    {
      wipeCube();
      drawCuboid(0,0,0,i,i,i, brightness);
      delay(80);
    }
    
    for(int i = 1 ; i <= 7 ; i++)
    {
      wipeCube();
      drawCuboid(i,i,i,7-i,7-i,7-i, brightness);
      delay(80);
    }
  }
  
  void sphere(byte brightness)
  {
    wipeCube();
    for(int i = 3 ; i <= 4 ; i++)
    {
      LED(3,1,i, brightness);
      LED(4,1,i, brightness);
      
      LED(2,2,i, brightness);
      LED(3,2,i, brightness);
      LED(4,2,i, brightness);
      LED(5,2,i, brightness);
      
      LED(1,3,i, brightness);  LED(1,4,i, brightness);
      LED(2,3,i, brightness);  LED(2,4,i, brightness);
      LED(3,3,i, brightness);  LED(3,4,i, brightness);
      LED(4,3,i, brightness);  LED(4,4,i, brightness);
      LED(5,3,i, brightness);  LED(5,4,i, brightness);
      LED(6,3,i, brightness);  LED(6,4,i, brightness);
    
      LED(2,5,i, brightness);
      LED(3,5,i, brightness);
      LED(4,5,i, brightness);
      LED(5,5,i, brightness);
      
      LED(3,6,i, brightness);
      LED(4,6,i, brightness);
    }
    
    for(int i = 3 ; i <= 4 ; i++)
    {
      LED(i,1,3, brightness);
      LED(i,1,4, brightness);
      
      LED(i,2,2, brightness);
      LED(i,2,3, brightness);
      LED(i,2,4, brightness);
      LED(i,2,5, brightness);
      
      LED(i,3,1, brightness);  LED(i,4,1, brightness);
      LED(i,3,2, brightness);  LED(i,4,2, brightness);
      LED(i,3,3, brightness);  LED(i,4,3, brightness);
      LED(i,3,4, brightness);  LED(i,4,4, brightness);
      LED(i,3,5, brightness);  LED(i,4,5, brightness);
      LED(i,3,6, brightness);  LED(i,4,6, brightness);
    
      LED(i,5,2, brightness);
      LED(i,5,3, brightness);
      LED(i,5,4, brightness);
      LED(i,5,5, brightness);
      
      LED(i,6,3, brightness);
      LED(i,6,4, brightness);
    }
    
    LED(2,3,2, brightness);
    LED(2,4,2, brightness);
    
    LED(2,3,5, brightness);
    LED(2,4,5, brightness);
    
    LED(5,3,2, brightness);
    LED(5,4,2, brightness);
    
    LED(5,3,5, brightness);
    LED(5,4,5, brightness);
  }
  
  void increasingSweep(byte brightness)
  {
    wipeCube();
    for(int i = 0; i < 45 ; i+= 5)
    {
      if(i == 40)
        for(int j = 0 ; j < 5 ; j++) sweep(50 - i, brightness);
      else
        sweep(50 - i, brightness);
    }
  }
  
  void sweep(int time, byte brightness)
  {
    /* Up and down */
    for(int i = 0 ; i < 8 ; i++)
    {
      wipeCube();
      for(int j = 0 ; j < 8 ; j++)
        for(int k = 0 ; k < 8 ; k++)
          LED(k,i,j, brightness);
      delay(time);
    }
    for(int i = 6 ; i > 0 ; i--)
    {
      wipeCube();
      for(int j = 0 ; j < 8 ; j++)
        for(int k = 0 ; k < 8 ; k++)
          LED(k,i,j, brightness);
      delay(time);
    }
    
    /* left to right */
    for(int i = 0 ; i < 8 ; i++)
    {
      wipeCube();
      for(int j = 0 ; j < 8 ; j++)
        for(int k = 0 ; k < 8 ; k++)
          LED(i,k,j, brightness);
      delay(time);
    }
    for(int i = 6 ; i > 0 ; i--)
    {
      wipeCube();
      for(int j = 0 ; j < 8 ; j++)
        for(int k = 0 ; k < 8 ; k++)
          LED(i,k,j, brightness);
      delay(time);
    }
    
    /* forward and back */
    for(int i = 0 ; i < 8 ; i++)
    {
      wipeCube();
      for(int j = 0 ; j < 8 ; j++)
        for(int k = 0 ; k < 8 ; k++)
          LED(k,j,i, brightness);
      delay(time);
    }
    for(int i = 6 ; i > 0 ; i--)
    {
      wipeCube();
      for(int j = 0 ; j < 8 ; j++)
        for(int k = 0 ; k < 8 ; k++)
          LED(k,j,i, brightness);
      delay(time);
    }
  }
  
  /* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\ */
  
  

