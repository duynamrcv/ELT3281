/* 
 * File:   adc.h
 * Author: Nam
 *
 * Created on June 20, 2020, 11:04 AM
 */

#ifndef ADC_H
#define	ADC_H

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* ADC_H */

void ADC_Init(){
    //------[There are 2 registers to configure ADCON0 and ADCON1]---------
     ADCON0 = 0x41;
    // Select clock option Fosc/8
//    ADCS0 = 1;
//    ADCS1 = 0;
//    // Turn ADC on
//    ADON = 1;
    
    ADCON1 = 0x80;
//    // Result mode: Right justified
//    ADFM = 1;
//    // Select clock option Fosc/8
//    ADCS2 = 0;
//    // Configure all 8 channels are analog 
//    PCFG0 = 0;
//    PCFG1 = 0;
//    PCFG2 = 0;
//    PCFG3 = 0;
}

int ADC_Read(int ADC_channel){
    // Check channel number
    if(ADC_channel < 0 || ADC_channel > 7)
        return 0;
    
    // Write ADC__channel into register ADCON0
    CHS0 = (ADC_channel & 1) >> 0;
    CHS1 = (ADC_channel & 2) >> 1;
    CHS2 = (ADC_channel & 4) >> 2;
    
    // Wait the Acquisition time 
    __delay_us(25);
    
    // Start A/D conversion
    GO_DONE = 1;
    
    // (Polling) Wait for the conversion to complete
    while(GO_DONE);
    
    // Read the ADC result ("right justified" mode)
    int result = ((ADRESH << 8) + ADRESL);
    return result;
}