/////////////////////////////////////////////////////////////////// 
// Beginning of code 
/////////////////////////////////////////////////////////////////// 
#include <project.h>
#include <stdio.h>
#define addr 0x29u // I2C address of VL6180 shifted by 1 bit
                    //(0x29 << 1) so the R/W command can be added

///////////////////////////////////////////////////////////////////
// Split 16-bit register address into two bytes and write
// the address + data via I2C
/////////////////////////////////////////////////////////////////// 

void WriteByte(uint8 reg,uint8 data) {
    
    uint8 data_write[3];
    uint8 SlaveAddress = addr;
    data_write[0] = (reg >> 8) & 0xFF;; // MSB of register address 
    data_write[1] = reg & 0xFF; // LSB of register address 
    data_write[2] = data & 0xFF;
    char out[10];
    //STM32 F401を使う時に使用
    /*
    i2c.write(addr, data_write, 3); 
    */
    uint8 temp;
     
    I2C_1_MasterClearStatus();
    temp = I2C_1_MasterWriteBuf(addr,data_write,3,I2C_1_MODE_COMPLETE_XFER);//I2C_1_MasterWriteBuf(SlaveAddress,DataAddress,Byte_Count,I2C_Mode)
    while (temp != I2C_1_MSTR_NO_ERROR);
    while(I2C_1_MasterStatus() & I2C_1_MSTAT_XFER_INP);
    temp = I2C_1_MasterClearStatus();
}

/////////////////////////////////////////////////////////////////// 
// Split 16-bit register address into two bytes and write
// required register address to VL6180 and read the data back 
/////////////////////////////////////////////////////////////////// 

uint8 ReadByte(uint8_t reg) {
    uint8 data_write[2]; 
    uint8 data_read[1];
    uint8 SlaveAddress = addr;
    uint8_t result;
    char out[10];
        uint8 temp;
    
    
    data_write[0] = (reg >> 8) & 0xFF; // MSB of register address 
    data_write[1] = reg & 0xFF; // LSB of register address
    //using for STM32 F401
    /*
    i2c.write(addr, data_write, 2); 
    i2c.read(addr, data_read, 1); 
    return data_read[0];
    */      
    I2C_1_MasterClearStatus();
    //I2Cデバイスから読み込むために書き込み
    temp = I2C_1_MasterWriteBuf(addr,data_write,2,I2C_1_MODE_COMPLETE_XFER);//I2C_1_MasterWriteBuf(SlaveAddress,DataAddress,Byte_Count,I2C_Mode)
    while (temp != I2C_1_MSTR_NO_ERROR);
    while(I2C_1_MasterStatus() & I2C_1_MSTAT_XFER_INP);
    temp = I2C_1_MasterClearStatus();
    //I2Cデバイスから読み込み
    temp = I2C_1_MasterReadBuf(SlaveAddress,data_read,1,I2C_1_MODE_COMPLETE_XFER);
    while (temp != I2C_1_MSTR_NO_ERROR);
    while(I2C_1_MasterStatus() & I2C_1_MSTAT_XFER_INP);
    temp = I2C_1_MasterClearStatus();
    return data_read[0];

    /*STM32 F401を使う時に使用
    I2C_1_MasterWriteByte(reg);
    uint8 data ;
    data=I2C_1_MasterReadByte(0) ;  // ignore error 
    return( data )
    */
     
}

/////////////////////////////////////////////////////////////////// 
// load settings 
/////////////////////////////////////////////////////////////////// 

int VL6180_Init() {
uint8 reset;
uint8 reset_16;
char out[10];
reset = ReadByte(0x016);
 
if (reset==1){  // check to see has it be Initialised already
       
    
/////////////////////////////////////////////////////////////////// 
// Added latest settings here - see Section 9
/////////////////////////////////////////////////////////////////// 
        WriteByte(0x016, 0x00);//change fresh out of set status to 0
        
         
    }
    return 0; 
}


/////////////////////////////////////////////////////////////////// 
// Start a range measurement in single shot mode 
/////////////////////////////////////////////////////////////////// 

int VL6180_Start_Range() {
    WriteByte(0x018,0x01);
    return 0; 
}
/////////////////////////////////////////////////////////////////// 
// poll for new sample ready ready 
/////////////////////////////////////////////////////////////////// 

int VL6180_Poll_Range() {
    char status;
    char range_status;
    char out[10];
    // check the status
    status = ReadByte(0x04f); 
    range_status = status & 0x07;
    // wait for new measurement ready status 

    while (range_status != 0x04) {
        status = ReadByte(0x04f); 
        range_status = status & 0x07; 
        CyDelay(1); // (can be removed) 
        
    }
      
    return 0; 

}

/////////////////////////////////////////////////////////////////// 
// Read range result (mm) 
/////////////////////////////////////////////////////////////////// 

int VL6180_Read_Range() {
    int range; 
    range=ReadByte(0x062); 
    return range;
}


/////////////////////////////////////////////////////////////////// 
// clear interrupts 
/////////////////////////////////////////////////////////////////// 

int VL6180_Clear_Interrupts() {
    WriteByte(0x015,0x07);
    return 0; 
}


/////////////////////////////////////////////////////////////////// 
// Main Program loop
 /////////////////////////////////////////////////////////////////// 
 
int main()
{
    /* Enable global interrupts.(割り込みを使う事を許可する) */
    CyGlobalIntEnable; 

    //3.3Vで動かす
    USBUART_1_Start(0, USBUART_1_3V_OPERATION);
    
    // USBUARTの設定が出来るまで待つ
    while(!USBUART_1_GetConfiguration());
    
    //CDCUSBUARTをスタートさせる
    USBUART_1_CDC_Init();
    I2C_1_Start();
    
    int range;
    char out[10];
   
   // load settings onto VL6180X VL6180_Init();
    VL6180_Init();
 
    while (1){
         // start single range measurement 
        VL6180_Start_Range();
        // poll the VL6180 till new sample ready 
        VL6180_Poll_Range();
         // read range result
        range = VL6180_Read_Range();
        // clear the interrupt on VL6180 
        VL6180_Clear_Interrupts();
        
        //ここでrangeの値を出力
        sprintf(out,"range=%d",range);
        USBUART_1_PutString(out);
        while(USBUART_1_CDCIsReady()==0u){};
        USBUART_1_PutCRLF();
        CyDelay(10);

        /*using for STM32 F401
        // send range to pc by serial 
        pc.printf("%d\r\n", range); 
        wait(0.1);
        */
    }
}