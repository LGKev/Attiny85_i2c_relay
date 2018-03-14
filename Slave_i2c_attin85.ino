// TinyWire Slave RX
// Kevin Kuwata
// recieves byte from master, and activates relay
// Created 2/20/2018

#include <TinyWire.h> //https://github.com/lucullusTheOnly/TinyWire
#include <EEPROM.h>



#define RELAY_PIN   4
#define LED         0 // on board led for the Attiny programmer


#define REGISTER_MAP_SIZE    3// ADDRESS, STATUS, ON

volatile byte SLAVE_ADDRESS  =    0x18;


volatile int ReceivedData[32]; //32 byte array to act as a buffer for I2C data. 32 bytes is the max for an UNO 


//Control Flags
volatile bool update_register = false;
volatile bool relay_state = false; //default off;


void setup() {
	
		byte current_value = EEPROM.read(0);

	
	if(current_value == 0xFF){
	//lets write to eeprom only if not written to before,
	//brand new chip eeprom is 0xFF
	EEPROM.write(0, SLAVE_ADDRESS); //default is 0x18.
	}
	else{
		//been written before, get the value from eeprom and set SLAVE_ADDRESS
		SLAVE_ADDRESS = EEPROM.read(0);
	}


  TinyWire.begin(SLAVE_ADDRESS);
  //TODO: eventually check eeprom, then default    
    pinMode(RELAY_PIN, OUTPUT);
   TinyWire.onReceive(receiveEvent); // register event
    TinyWire.onRequest(onI2CRequest);
	



}

void loop() {
	
	if(ReceivedData[0] == 0x01){
		digitalWrite(RELAY_PIN, HIGH);
	}
	if(ReceivedData[0] == 0x00){
		digitalWrite(RELAY_PIN, LOW);
	}
	
	if(ReceivedData[0] == 0x03){
		
		
		//see if the proposed slave address is valid. 
		if(ReceivedData[1] > 0x07 && ReceivedData[1] < 0x78){
			//valid address
				SLAVE_ADDRESS = ReceivedData[1];
				EEPROM.write(0, SLAVE_ADDRESS);
		}
		else{
			//invalid address
			SLAVE_ADDRESS = EEPROM.read(0); //go back to the original value.
		}
		TinyWire.begin(SLAVE_ADDRESS);
		//store in eeprom
		ReceivedData[0] = 0xFF; //reset the buffer
	}
	
	
}

/*========================================================*/
//        Helper Functions
/*========================================================*/


/*========================================================*/
//        ISR
/*========================================================*/
/*
    @brief: When the master initiates a command and data to slave
		ie) the master says 0x01, then sends a 1, means command: 0x01 then the slave listens for the next thing, which is the relay state 1
    @global: This is modifying ReceivedData array, which stores all the bytes that a master sent to the slave.
	@input:
    @returns: none
    @flags:  none
*/
void receiveEvent(int bytesReceived) {
	int index = 0; //index of all the data received from master.
	
  while(TinyWire.available() > 0){ //this was the solution, 
		ReceivedData[index] = TinyWire.read();
		index++;
  }//end of the while loop, data available
}// end of receive ISR


/*
    @brief: When the master requests data from the slave, this
      ISR is triggered. 
    @input: none, 
    @returns: none
    @flags:  none
*/
void onI2CRequest() {
	//TODO: add in something to report the address.
	if(digitalRead(RELAY_PIN) == HIGH) TinyWire.send(0x01);
	else{
		TinyWire.send(0x00);
	}
}// end of request ISR
