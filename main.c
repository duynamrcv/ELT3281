/*
 * File:   main.c
 * Author: BDN
 *
 * Created on June 3, 2020, 9:57 PM
 */

// BEGIN CONFIG
#pragma config FOSC = XT        // Oscillator Selection bits (XT oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT enabled)
#pragma config BOREN = ON       // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program M
//END CONFIG

#include <pic16f877a.h>
#include <xc.h>

#define _XTAL_FREQ 8000000

/*==========================*
 *=          LCD           =*
 *==========================*/
// Configure the data bus and Control bus as per the hardware connection 
// Dtatus bus is connected to PB0:PB7 and control bus PD0:PD2
#define LcdDataBus PORTB
#define LcdCtrlBus PORTD

#define LcdDataBusDirnReg TRISB
#define LcdCtrlBusDirnReg TRISD

#define LCD_RS 0
#define LCD_RW 1
#define LCD_EN 2

// Function to send the command to LCD
void LcdCmdWrite(char cmd)
{
    LcdDataBus = cmd;            // Send the Command nibble
    LcdCtrlBus &= ~(1<<LCD_RS);  // Send LOW pulse on RS pin for selecting Command register
    LcdCtrlBus &= ~(1<<LCD_RW);  // Send LOW pulse on RW pin for Write operation
    LcdCtrlBus |= (1<<LCD_EN);   // Generate a High-to-low pulse on EN pin
    __delay_ms(10);
    LcdCtrlBus &= ~(1<<LCD_EN); 
}
/* Command
 * LcdCmdWrite(0x38);        // enable 5x7 mode for chars 
 * LcdCmdWrite(0x0E);        // Display OFF, Cursor ON
 * LcdCmdWrite(0x01);        // Clear Display
 * LcdCmdWrite(0x80);        // First line
 * LcdCmdWrite(0xC0);        // Second line
*/
 
// Function to send the Data to LCD 
void LcdDataWrite(char data)
{
    LcdDataBus = data;           // Send the data on DataBus nibble
    LcdCtrlBus |= (1<<LCD_RS);   // Send HIGH pulse on RS pin for selecting data register
    LcdCtrlBus &= ~(1<<LCD_RW);  // Send LOW pulse on RW pin for Write operation
    LcdCtrlBus |= (1<<LCD_EN);   // Generate a High-to-low pulse on EN pin
    __delay_ms(10);
    LcdCtrlBus &= ~(1<<LCD_EN);
}
// Function to show message on LCD
void LcdPrint(char* message)
{
    for(char i = 0; message[i] != '\0'; i++)
    {
        LcdDataWrite(message[i]);
    }
}

/*==========================*
 *=      DHT11 Sensor      =*
 *==========================*/
// Configure the data pin and Ctrl pin
#define DhtDataPin RD3
#define DhtDataDirnReg TRISD3

// Variance
char temp[] = "Temp = 00.0 C";
char humi[] = "RH   = 00.0 %";
unsigned char t_byte1, t_byte2, rh_byte1, rh_byte2, check_sum;
short time_out;

// Function to initialize sensor
void startSignal()
{
    DhtDataDirnReg = 0;     // Configure connection pin as output
    DhtDataPin = 0;         // Connection pin output is low
    
    __delay_ms(25);
    DhtDataPin = 1;         // Connection pin output is high
    __delay_us(30);
    DhtDataDirnReg = 1;     // Configure connection pin as input
}

// Function to check response of sensor
__bit checkResponse()
{
    __delay_us(40);
    if(!DhtDataPin)
    {
        __delay_us(80);
        if(DhtDataPin)
        {
            __delay_us(50);
            return 1;
        }
    }
    return 0;
}

// Function to read data from sensor
__bit readData(unsigned char* dht_data)
{
    *dht_data = 0;

    for(char i = 0; i < 8; i++)
    {
      TMR1H = 0;             // reset Timer1
      TMR1L = 0;

      while(!DhtDataPin)      // wait until DhtDataPIN becomes high
          if(TMR1L > 100) return 1;

      TMR1H = 0;             // reset Timer1
      TMR1L = 0;

      while(DhtDataPin)       // wait until DHT11_PIN becomes low
          if(TMR1L > 100) return 1;

      if(TMR1L > 50)                  // if high time > 50  ==>  Sensor sent 1
         *dht_data |= (1 << (7 - i));  // set bit (7 - i)
    }
    return 0;                          // return 0 (data read OK)
}

/*==========================*
 *=          Main          =*
 *==========================*/
void main(void)
{
    // LCD init
    LcdDataBusDirnReg = 0x00;  // Configure all the LCD pins as output
    LcdCtrlBusDirnReg = 0x00;  // Configure the Ctrl pins as output

    LcdCmdWrite(0x38);        // enable 5x7 mode for chars 
    LcdCmdWrite(0x0E);        // Display OFF, Cursor ON
    
    while(1)
    {
        startSignal();
        if(checkResponse())
        {
            // read (and save) data from the DHT11 sensor and check time out errors
            if(readData(&rh_byte1) || readData(&rh_byte2) || readData(&t_byte1)
                    || readData(&t_byte2) || readData(&check_sum))
            {
                LcdCmdWrite(0x01);        // Clear Display
                LcdCmdWrite(0x80);        // First line
                LcdPrint("Time out!");    // display "Time out!"
            }
            else
            {
                if(check_sum == ((rh_byte1 + rh_byte2 + t_byte1 + t_byte2) & 0xFF))
                {
                    temp[7]  = t_byte1 / 10  + 48;
                    temp[8]  = t_byte1 % 10  + 48;
                    temp[10] = t_byte2 / 10  + 48;
                    
                    humi[7]  = rh_byte1 / 10 + 48;
                    humi[8]  = rh_byte1 % 10 + 48;
                    humi[10] = rh_byte2 / 10 + 48;
                    temp[11] = 223;    // put degree symbol (�)
                    
                    LcdCmdWrite(0x80);        // First line
                    LcdPrint(temp);
                    
                    LcdCmdWrite(0xC0);        // Second line
                    LcdPrint(humi);
                }
                else
                {
                    LcdCmdWrite(0x01);        // Clear Display
                    LcdCmdWrite(0x80);        // First line
                    LcdPrint("Checksum Error!");
                }

            }
        }
        else
        {
            LcdCmdWrite(0x01);        // Clear Display
            LcdCmdWrite(0x80);        // First line
            LcdPrint("No Response");
        }
        __delay_ms(1000);
    }
    return;
}