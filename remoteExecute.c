#include <stdio.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "gpio.h"
#include <stdbool.h>
#include <time.h>
#include <string.h>
// socket
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
// ip
#include <unistd.h> /* for close */
#include <sys/types.h>
#include <sys/ioctl.h>
#include <net/if.h>

//  valves
#define pin_spi_cs1  P9_16 // 1_19=51
#define pin_spi_other P9_22 // 0_2=2
#define pin_spi_mosi P9_30 // 3_15=112
#define pin_spi_sclk P9_21 // 0_3=3
#define pin_spi_cs2  P9_42 // 0_7 =7
#define NUM_OF_CHANNELS 16
// sensors
#define pin_din_sensor  P9_11 // 0_30=30
#define pin_clk_sensor P9_12 // 1_28=60
#define pin_cs_sensor P9_13 // 0_31=31
#define pin_dout1_sensor P9_14 // 1_18=50
#define pin_dout2_sensor  P9_15 // 1_19=51
#define NUM_ADC_PORT 8
//#define NUM_ADC 2
unsigned int  NUM_ADC = 2;

#define NUM_BUFFER 1024

char filename_ADC[] = "params/ADC_NUM.dat";

// SPI for valves
bool clock_edge = false;
unsigned short resolution = 0x0FFF;
void set_SCLK(bool value) { digitalWrite(pin_spi_sclk, value); }
void set_OTHER(bool value) { digitalWrite(pin_spi_other, value); }
void set_MOSI(bool value) { digitalWrite(pin_spi_mosi, value); }
void setCS1(bool value){ digitalWrite(pin_spi_cs1, value); }
void setCS2(bool value){ digitalWrite(pin_spi_cs2, value); }
void set_clock_edge(bool value){ clock_edge = value; }
bool get_MISO(void) { return false; } // dummy
void wait_SPI(void){}

// value 1: Enable chipx
void chipSelect1(bool value){ setCS1(!value); wait_SPI(); wait_SPI(); }
void chipSelect2(bool value){ setCS2(!value); wait_SPI(); wait_SPI(); }

unsigned char transmit8bit(unsigned char output_data){
	unsigned char input_data = 0;
	int i;
	for(i = 7; i >= 0; i--){
		// MOSI - Master : write with down trigger
		//        Slave  : read with up trigger
		// MISO - Master : read before down trigger
		//        Slave  : write after down trigger
		set_SCLK(!clock_edge);
		set_MOSI( (bool)((output_data>>i)&0x01) );
		input_data <<= 1;
		wait_SPI();
		set_SCLK(clock_edge);
		input_data |= get_MISO() & 0x01;
		wait_SPI();
	}
	return input_data;
}

unsigned short transmit16bit(unsigned short output_data){
	unsigned char input_data_H, input_data_L;
	unsigned short input_data;
	input_data_H = transmit8bit( (unsigned char)(output_data>>8) );
	input_data_L = transmit8bit( (unsigned char)(output_data) );
	input_data = (((unsigned short)input_data_H << 8)&0xff00) | (unsigned short)input_data_L;
	return input_data;
}


void setDARegister(unsigned char ch, unsigned short dac_data){
	unsigned short register_data;

	if (ch < 8) {
		register_data = (((unsigned short)ch << 12) & 0x7000) | (dac_data & 0x0fff);
		chipSelect1(true);
		transmit16bit(register_data);
		chipSelect1(false);
	}
	else if (ch >= 8) {
		register_data = (((unsigned short)(ch & 0x0007) << 12) & 0x7000) | (dac_data & 0x0fff);
		chipSelect2(true);
		transmit16bit(register_data);
		chipSelect2(false);
	}
}

// pressure coeff: [0.0, 1.0]
void setState(unsigned int ch, double pressure_coeff)
{
	setDARegister(ch, (unsigned short)(pressure_coeff * resolution));
}

/**** SPI for sensors ****/
void set_DIN_SENSOR(bool value) { digitalWrite(pin_din_sensor, value); }
void set_CLK_SENSOR(bool value) { digitalWrite(pin_clk_sensor, value); }
void set_CS_SENSOR(bool value) { digitalWrite(pin_cs_sensor, value); }
int get_DOUT_SENSOR(int adc_num) { 
	if(adc_num==0){
		digitalRead(pin_dout1_sensor); 
	}
	else{
		digitalRead(pin_dout2_sensor); 
	}
}

unsigned long *read_sensor(unsigned long adc_num,unsigned long* sensorVal){
	
	unsigned long pin_num=0x00;
	unsigned long sVal;
	unsigned long commandout=0x00;
	
	int i;
	
    for(pin_num=0;pin_num<NUM_ADC_PORT;pin_num++){
    	sVal=0x00;
		set_CS_SENSOR(true);
		set_CLK_SENSOR(false);
		set_DIN_SENSOR(false);
		set_CS_SENSOR(false);
		
    	commandout=pin_num;
    	commandout|=0x18;
    	commandout<<=3;
		
	    for(i=0;i<5;i++){
			if(commandout&0x80){
				set_DIN_SENSOR(true);
	  		}
	  		else{
				set_DIN_SENSOR(false);
	  		}
	  		commandout<<=1;
	  		set_CLK_SENSOR(true);
	  		set_CLK_SENSOR(false);
      	}
      	for(i=0;i<2;i++){
			set_CLK_SENSOR(true);
	    	set_CLK_SENSOR(false);
      	}
      	for(i=0;i<12;i++){
			set_CLK_SENSOR(true);
			sVal<<=1;
	  		if(get_DOUT_SENSOR(adc_num)){
	    		sVal|=0x01;
	    	}
	  	set_CLK_SENSOR(false);
    	}
    	sensorVal[pin_num]=sVal;
    }
    return(sensorVal);
}

/*******************************************/
/*              Init Functions              /
/*******************************************/
void init_pins()
{
	set_SCLK(LOW);
	set_MOSI(LOW);
	set_OTHER(LOW);
	setCS1(HIGH);
	setCS2(HIGH);

/*	set_SCLK(HIGH);
	set_MOSI(HIGH);
	set_OTHER(HIGH);
	setCS1(HIGH);
	setCS2(HIGH);
*/
/*
	analog_pin[0] = P9_33;
	analog_pin[1] = P9_35;
	analog_pin[2] = P9_36;
	analog_pin[3] = P9_37;
	analog_pin[4] = P9_38;
	analog_pin[5] = P9_39;
	analog_pin[6] = P9_40;
*/
}


void init_DAConvAD5328(void) {
	set_clock_edge(false);// negative clock (use falling-edge)

	// initialize chip 1
	chipSelect1(true);
	transmit16bit(0xa000);// synchronized mode
	chipSelect1(false);

	chipSelect1(true);
	transmit16bit(0x8003);// Vdd as reference
	chipSelect1(false);

	// initialize chip 2
	chipSelect2(true);
	transmit16bit(0xa000);// synchronized mode
	chipSelect2(false);

	chipSelect2(true);
	transmit16bit(0x8003);// Vdd as reference
	chipSelect2(false);
}

void init_sensor(void) {
	set_DIN_SENSOR(false);
	set_CLK_SENSOR(false);
	set_CS_SENSOR(false);
}


// below my function
char* getIP(void){
  int fd;
  struct ifreq ifr;

  fd = socket(AF_INET, SOCK_DGRAM, 0);
  ifr.ifr_addr.sa_family = AF_INET; // IPv4 IP address
  strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);   // eth0 IP address
  ioctl(fd, SIOCGIFADDR, &ifr);
  close(fd);

  // printf("%s\n", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));

  return inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
}

int setServer(void){
  int welcomeSocket, newSocket_;
  struct sockaddr_in serverAddr;
  struct sockaddr_storage serverStorage;
  socklen_t addr_size;
  int port_num = 7891;

  char* ip_address = getIP();
  printf("ip address: %s\n", ip_address );

  welcomeSocket = socket(PF_INET, SOCK_STREAM, 0);
  serverAddr.sin_family = AF_INET;
  //serverAddr.sin_port = htons(7891);
  serverAddr.sin_port = htons(port_num);
  serverAddr.sin_addr.s_addr = inet_addr(ip_address);
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
  bind(welcomeSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

  if(listen(welcomeSocket,5)==0)
    printf("Listening\n");
  else
    printf("Error\n");

  addr_size = sizeof serverStorage;
  newSocket_ = accept(welcomeSocket, (struct sockaddr *) &serverStorage, &addr_size);

  return newSocket_;
}

void exhaustAll(){
  int ch_num;
  double Exhaust = 0.0;
  for ( ch_num = 0; ch_num< NUM_OF_CHANNELS; ch_num++ )
    setState( ch_num, Exhaust ); 
}

int getADCNumber(void){
  FILE *fp;
  char val[NUM_BUFFER];

  fp = fopen( filename_ADC, "r" );

  if ( fp == NULL ){
    printf("File open error (ADC)\n");
    return;
  }

  fgets( val, sizeof(val), fp );
  int NUM_ADC_ = atoi(val);

  fclose(fp);

  return NUM_ADC_;
}

int main( int argc, char *argv[] ){
  NUM_ADC = getADCNumber();
  printf( "num of ADC: %d\n", NUM_ADC );
  
  unsigned int ch_num;
  double Exhaust = 0.0;

  //char buffer[99999];
  char buffer[NUM_BUFFER];
  int newSocket = setServer();

  // initialize
  init();
  init_pins(); // ALL 5 pins are HIGH except for GND
  init_DAConvAD5328();
  init_sensor();
  
  exhaustAll();
  
  // loop
  unsigned long *tmp_val0;
  unsigned long tmp_val[NUM_ADC_PORT];
  char char_val[12];
  char *char_top, *char_command;
  double Pressure;
  int j, k;
  //struct timeval ini, now;
  //double elasped = 0;
  //gettimeofday( &ini, NULL );  

  while (1){
    // send sensor value
    strcpy( buffer, "sensor: " );
    for ( j = 0; j< NUM_ADC; j++){
      tmp_val0 = read_sensor(j,tmp_val);
      for ( k = 0; k< NUM_ADC_PORT; k++){
	//printf("%d\t", tmp_val0[k]);
	sprintf( char_val, "%d ", tmp_val0[k] );
	strcat( buffer, char_val );
      }
    }
    //send( newSocket, buffer, 128, 0);
    send( newSocket, buffer, NUM_BUFFER, 0);
    //printf("\n");
    //printf( "%s\n", buffer );
  
    // recieve command value
    recv( newSocket, buffer, NUM_BUFFER, 0);
    //printf( "%s\n", buffer );
    if ( strlen( buffer ) != 0 ){
      char_top = strtok( buffer, " " );
      if ( strcmp( char_top, "command:" ) == 0 ){
	for (ch_num = 0; ch_num< NUM_OF_CHANNELS; ch_num++){
	  //if ( strlen( buffer ) == 0 )
	  //break;
	  char_command = strtok( NULL, " " );
	  Pressure = atof( char_command );
	  setState( ch_num, Pressure ); 
	  //printf( "%4.3f ", Pressure );
	}
	//printf( "\n" );
	printf( "%s\n", buffer );
	//break;
      }
    }
    //gettimeofday( &now, NULL );  
    //elasped = now.tv_sec - ini.tv_sec;
    //if ( elasped > 1 )
    //break;
  }

  //exhaustAll();
    
  return 0;
}


