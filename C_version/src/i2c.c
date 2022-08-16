#include "i2c.h"
#include "meta.h"

void i2c_master_setup(void) {
    // SCL_FREQ F_CPU/(16 + 2*(TWBR_VALUE)*PRESCALER)
    // Set I2C frequency to 400kHz
    TWSR = 0x00; // prescaler = 1
    TWBR = 0x0C; // 0x0C = 12 
    TWCR |= (1 << TWEN); // Turn on TWI/I2C module  
}

void i2c_master_start(void) {
    // Send start bit, enable interrupt, enable TWI  
    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN); 
    while ((TWCR & (1 << TWSTA)) == 0){ ; } // Wait until start bit sent
}

void i2c_master_stop(void) {
    // Send stop bit, enable interrupts, enable TWI
    TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN); 
}

void i2c_master_send(uint8_t data_byte) {
    TWDR = data_byte; // Store data in data register
    TWCR = (1 << TWINT) | (1 << TWEN);
    while ((TWCR & (1<<TWINT)) == 0); // Wait for transmission
}

uint8_t i2c_master_read_ack(void) {
    TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA); 
    while ((TWCR & (1<<TWINT)) == 0); // Wait for transmission
    return TWDR; 
}

uint8_t i2c_master_read_nack(void) {
    TWCR = (1<<TWINT)|(1<<TWEN); 
    while ((TWCR & (1<<TWINT)) == 0); // Wait for transmission
    return TWDR; 
}

uint8_t i2c_master_get_status(void) {
    uint8_t status;
    status = TWSR & 0b11111000; // Mask status
    return status;
}

void set_pin(uint8_t Wadd, uint8_t reg, uint8_t value){
    i2c_master_start();       // Start bit
    if (TWIGetStatus() != START_SUCCESS); panic(); 
    
    i2c_master_send(Wadd);    // Send address for write
    if (TWIGetStatus() != SLA_ACK_SUCCESS); panic(); 
    
    i2c_master_send(reg);     // Send data - which register?
    if (TWIGetStatus() != DATA_ACK_SUCCESS); panic();
   
    i2c_master_send(value);   // Send data - what value?
    if (TWIGetStatus() != DATA_ACK_SUCCESS); panic();
  
    i2c_master_stop();        // Stop bit   
}

uint8_t read_pin(uint8_t Wadd, uint8_t Radd, uint8_t reg){
    // uint8_t Radd = Wadd | 0b00000001;
    uint8_t recv;
   
    i2c_master_start();         // Send start bit
    if (TWIGetStatus() != START_SUCCESS); panic();
    
    i2c_master_send(Wadd);      // Send write address
    if (TWIGetStatus() != SLA_ACK_SUCCESS); panic();
   
    i2c_master_send(reg);       // Send the register we want to read from 
    if (TWIGetStatus() != DATA_ACK_SUCCESS); panic();
  
    i2c_master_start();         // Restart
    if ((i2c_master_get_status()) != RESTART_SUCCESS); panic();
 
    i2c_master_send(Radd);      // Send read address
    recv = i2c_master_read_ack();    // Get received value, send ack
    i2c_master_stop();          // Stop bit
    
    return recv;
}

void i2c_read_multiple(uint8_t Wadd, uint8_t Radd, uint8_t reg, uint8_t *raw, int len){
    // uint8_t Radd = Wadd | 0b00000001; // Change the last bit to 1 for read address 
    int i;
    i2c_master_start();       // Send start bit
    if (TWIGetStatus() != START_SUCCESS); panic();
    
    i2c_master_send(Wadd);    // Send write address
    if (TWIGetStatus() != SLA_ACK_SUCCESS); panic();
    
    i2c_master_send(reg);     // Send the register we want to read from 
    if (TWIGetStatus() != DATA_ACK_SUCCESS); panic();
    
    i2c_master_restart();     // Restart
    if ((i2c_master_get_status()) != RESTART_SUCCESS); panic();
    i2c_master_send(Radd);    // Send read address
    
    for (i = 0; i < len; i++){
        if (i == (len-1)){  // break out at the last one 
            raw[i] = i2c_master_read_ack(); 
            break;
        }
        else{
            raw[i] = i2c_master_read_nack();
        }
         
    }
    i2c_master_stop();        // Stop bit

}



