//Header File config
/********************************************************************************************************/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/delay.h>
#include <util/delay.h>

//Define value
#define m_sec 100

//Mobility Status Data - It is used as an interface and check bit.
/********************************************************************************************************/
volatile char emergency_button  = 0;
volatile char door_status       = 0;
volatile char air_con_status    = 0;
volatile char rgb_led_status    = 0;
volatile char clcd_status       = 0;
volatile char passenger         = 0;
volatile char motion_detect_val = 0;

//Message transmission using ASCII code
/********************************************************************************************************/

//There was an emergency. Please check Mobility IF-1.
volatile char emergency_message[51] = {0x54, 0x68, 0x65, 0x72, 0x65, 0x20, 0x77, 0x61, 0x73, 0x20, 0x61, 0x6e, 0x20, 0x65, 
0x6d, 0x65, 0x72, 0x67, 0x65, 0x6e, 0x63, 0x79, 0x2E, 0x20, 0x50, 0x6C, 0x65, 0x61, 0x73, 0x65, 0x20, 0x63, 0x68, 0x65, 0x63, 
0x6B, 0x20, 0x4D, 0x6F, 0x62, 0x69, 0x6C, 0x69, 0x74, 0x79,0x20,0x49,0x46,0x2D,0x31,0x2E};

//The occupant is not moving. Please check Mobility IF-1.
volatile char not_moving_message[55] = {0x54, 0x68, 0x65, 0x20, 0x6F, 0x63, 0x63, 0x75, 0x70, 0x61, 0x6E, 0x74, 0x20, 0x69,
0x73, 0x20, 0x6e, 0x6f, 0x74, 0x20, 0x6d, 0x6f, 0x76, 0x69, 0x6e, 0x67, 0x2e, 0x20, 0x50, 0x6c, 0x65, 0x61, 0x73, 0x65, 0x20,
0x63, 0x68, 0x65, 0x63, 0x6b, 0x20, 0x4d, 0x6f, 0x62, 0x69, 0x6c, 0x69, 0x74, 0x79, 0x20, 0x49, 0x46, 0x2d, 0x31, 0x2e};

//Motion is detected on IF-1. Please check Mobility IF-1.
volatile char motion_detected_message[55] = {0x4d, 0x6f, 0x74, 0x69, 0x6f, 0x6E, 0x20, 0x69, 0x73, 0x20, 0x64,  0x65, 0x74,
0x65,  0x63, 0x74, 0x65, 0x64, 0x20, 0x6f, 0x6e, 0x20, 0x49, 0x46, 0x2d, 0x31, 0x2e, 0x20, 0x50, 0x6c, 0x65, 0x61, 0x73, 0x65, 0x20,
0x63, 0x68, 0x65, 0x63, 0x6b, 0x20, 0x4d, 0x6f, 0x62, 0x69, 0x6c, 0x69, 0x74, 0x79, 0x20, 0x49, 0x46, 0x2d, 0x31, 0x2e};



//Function Declaration
/********************************************************************************************************/
void CLCD_Init(void);
void CLCD_write_instruction(unsigned char data);
void CLCD_write_data(unsigned char data);
void CLCD_door_get_in(void);
void CLCD_door_get_out(void);
void CLCD_door_open(void);
void CLCD_door_close(void);
void CLCD_led_On(void);
void CLCD_led_Off(void);
void CLCD_air_con_on(void);
void CLCD_air_con_off(void);
void RGB_LED_setColor(int r,int g,int b);
void RGB_LED_init(void);
void RGB_LED_output(void);
void Servo_open_door(void);
void Servo_close_door(void);
void Init_interrupt(void);
void Init_Timer(void);
void Init_PWM(void);
void Pin_Config(void);
void Bluetooth_Config(void);
void Sensor_module_Config(void);

//Use Interrupt
//UART Interrupt Func - connectivity
/********************************************************************************************************/

//use index
volatile char idx  = 0;

// Periodic vehicle status information transmission and motion detection
ISR(TIMER1_OVF_vect)
{
   cli();
   //Set timer cycle
   TCNT1H = 0xC2;
   TCNT1L = 0xF6;

   emergency_button  = 0;

   //PIR(Passive Infrared Sensor) Func -Motion detection
   /*****************************************************************************************************/
   motion_detect_val = 0;

   if(PINF & 0x01) // PF0??? ?????? ????????????
   {
      motion_detect_val = 1;
   }
   else
   {
      motion_detect_val = 0;
   }

   //???????????? ??????????????? ????????? ???????????? ?????????
   if(passenger ==1 && motion_detect_val == 0)
   {  
      for(idx = 0; idx <55; idx++)
      {
         while(!(UCSR1A& 0x20));
         UDR1 = not_moving_message[idx];
      }

   }
   //???????????? ???????????? ???????????? ????????? ?????? ??????
   else if(passenger ==0 && motion_detect_val == 1)
   {  
      for(idx = 0; idx <55; idx++)
      {
         while(!(UCSR1A& 0x20));
         UDR1 = motion_detected_message[idx];
      }

   }

   sei();
}

ISR(SIG_UART1_RECV){
   cli();

   while(!(UCSR1A & 0x20)); //Data is Empty?
   
   if(UDR1==0x00){ // Door Close
         CLCD_door_close();
         Servo_close_door();        
         door_status = 0;
      }
   else if(UDR1 == 0x01){ // Door Open
         CLCD_door_open();
         Servo_open_door();
         door_status = 1;
      }
   else if(UDR1 == 0x10) // Air con OFF
      {
         //????????? ?????? ?????????
       CLCD_air_con_off();
         Servo360_aircon_off();
         air_con_status = 0;
      }
   else if(UDR1 == 0x11) // Air con ON
      {
         //????????? ?????? ?????????
       CLCD_air_con_on();
         Servo360_aircon_on();
         air_con_status = 1;
      }
   else if(UDR1 == 0x20) // RGB_LED OFF
      {
         rgb_led_status = 0;
         RGB_LED_output();
         CLCD_led_Off();
      }
   else if(UDR1 == 0x21) // RGB_LED White
      {
         rgb_led_status = 1;
         RGB_LED_output();
         CLCD_led_On();
      }
   else if(UDR1 == 0x22) // RGB_LED RED
      {
         rgb_led_status =2;
         RGB_LED_output();
         CLCD_led_On();
      }
   else if(UDR1 == 0x23) // RGB_LED Green
      {
         rgb_led_status = 3;
         RGB_LED_output();
         CLCD_led_On();
	      }
   else if(UDR1 == 0x24) // RGB_LED Blue
      {
         rgb_led_status = 4;
         RGB_LED_output();
         CLCD_led_On();
      }
   else if(UDR1 == 0x25) // RGB_LED Violet
      {
         rgb_led_status = 5;
         RGB_LED_output();
         CLCD_led_On();
      }
   else if(UDR1 == 0x30) // CLCD OFF
      {
         CLCD_Init();
         clcd_status = 0;
      }
   else if(UDR1 == 0xF0) // Get_out
      {
         CLCD_get_out();
         passenger =0;
      }
   else if(UDR1 == 0xF1) // Get_in
      {
         CLCD_get_in();
         passenger =1;
      }
            
   sei();
}

//Button Func - emergency button
/********************************************************************************************************/

ISR(INT0_vect) { 

cli();

emergency_button ++;
   if(emergency_button == 1){

      for(idx = 0; idx <51; idx++){
         
         while(!(UCSR1A& 0x20));
         UDR1 = emergency_message[idx];
      }
   }

sei();

}

//CLCD Func - Display
/********************************************************************************************************/

//CLCD config
#define ENABLE  PORTE |= 0x01;  // LCD ?????? ??????  
#define DISABLE PORTE &= ~0x01; // LCD ?????? ??????
#define RS_SET  PORTE |= 0x04;  // ????????? ??????
#define RS_CLI  PORTE &= ~0x04; // ?????? ??????
#define RW_SET  PORTE |= 0x02;  // ??????
#define RW_CLI  PORTE &= ~0x02; // ??????

//CLCD Init
void CLCD_Init( ){
 
_delay_ms(5);
CLCD_write_instruction(0x30);   // 8??????, 1???, 5*7
_delay_ms(5);
CLCD_write_instruction(0x30);
_delay_ms(5);
CLCD_write_instruction(0x30);
_delay_ms(5);
CLCD_write_instruction(0x38);   // 8??????, 2???, 5*7
_delay_ms(5);
CLCD_write_instruction(0x08);   // ?????? off, ????????? off, ?????? off
_delay_ms(5);
CLCD_write_instruction(0x01);   // ?????? 0?????? ??????
_delay_ms(5);
CLCD_write_instruction(0x06);   // ?????? ??????
_delay_ms(5);
CLCD_write_instruction(0x0C);   // ?????? on
_delay_ms(5);
}

//CLCD Location
void CLCD_write_instruction(unsigned char data){   // LCD??? ??? ????????? ??????

RS_CLI
RW_CLI

ENABLE
_delay_us(1);
PORTC = data;
_delay_us(1);
DISABLE

}

//CLCD Data write
void CLCD_write_data(unsigned char data){   // LCD??? ??? ????????? ????????? ??????

RS_SET
RW_CLI
ENABLE
PORTC = data;
_delay_us(1);
DISABLE
_delay_us(1);

_delay_ms(10);

}

void CLCD_get_in(){
   
   CLCD_Init();
   //Welcome
   //Have a nice day!
   CLCD_write_instruction(0x84); // Location
    
   CLCD_write_data('W');
   CLCD_write_data('W'); 
   CLCD_write_data('e'); 
   CLCD_write_data('l'); 
   CLCD_write_data('c'); 
   CLCD_write_data('o'); 
   CLCD_write_data('m'); 
   CLCD_write_data('e'); 

   CLCD_write_instruction(0xC0); 

   CLCD_write_data('H');
   CLCD_write_data('H'); 
   CLCD_write_data('a'); 
   CLCD_write_data('v'); 
   CLCD_write_data('e');
   CLCD_write_data('_');
   CLCD_write_data('a');
   CLCD_write_data('_');
   CLCD_write_data('n'); 
   CLCD_write_data('i'); 
   CLCD_write_data('c');
   CLCD_write_data('e');
   CLCD_write_data('_');
   CLCD_write_data('D'); 
   CLCD_write_data('a'); 
   CLCD_write_data('y');

}

void CLCD_get_out(){
   
   CLCD_Init();

   //Bye
   //Thank you!
   CLCD_write_instruction(0x86);
   
   CLCD_write_data('B');
   CLCD_write_data('B'); 
   CLCD_write_data('y'); 
   CLCD_write_data('e'); 

   CLCD_write_instruction(0xC3);

   CLCD_write_data('T');
   CLCD_write_data('T'); 
   CLCD_write_data('h'); 
   CLCD_write_data('a'); 
   CLCD_write_data('n');
   CLCD_write_data('k');
   CLCD_write_data('_'); 
   CLCD_write_data('y'); 
   CLCD_write_data('o');
   CLCD_write_data('u');
   CLCD_write_data('!');

}

void CLCD_door_open(){
   
   CLCD_Init();
   //Door
   //Open
   CLCD_write_instruction(0x84); 
    
   CLCD_write_data('D');
   CLCD_write_data('D'); 
   CLCD_write_data('o'); 
   CLCD_write_data('o'); 
   CLCD_write_data('r'); 

   CLCD_write_instruction(0xC0);

   CLCD_write_data('O');
   CLCD_write_data('O'); 
   CLCD_write_data('p'); 
   CLCD_write_data('e'); 
   CLCD_write_data('n');

}

void CLCD_door_close(){
   
   CLCD_Init();

   //Door
   //Close
   CLCD_write_instruction(0x86); 
   
   CLCD_write_data('D');
   CLCD_write_data('D'); 
   CLCD_write_data('o'); 
   CLCD_write_data('o'); 
   CLCD_write_data('r'); 

   CLCD_write_instruction(0xC3);

   CLCD_write_data('C');
   CLCD_write_data('C'); 
   CLCD_write_data('l'); 
   CLCD_write_data('o'); 
   CLCD_write_data('s');
   CLCD_write_data('e');

}

void CLCD_led_On(){
   
   CLCD_Init();
   
   //LED ON
    CLCD_write_instruction(0x85); // Location
 
    
    CLCD_write_data('L');
		CLCD_write_data('L'); 
    CLCD_write_data('E'); 
    CLCD_write_data('D'); 
    CLCD_write_data('-'); 
    CLCD_write_data('O'); 
    CLCD_write_data('N'); 

}

void CLCD_led_Off(){
   
   CLCD_Init();
   
   //LED OFF
    CLCD_write_instruction(0x85); // Location 
 

   CLCD_write_data('L'); 
   CLCD_write_data('L');
   CLCD_write_data('E'); 
   CLCD_write_data('D'); 
   CLCD_write_data('-'); 
   CLCD_write_data('O'); 
   CLCD_write_data('F'); 
   CLCD_write_data('F'); 

}

void CLCD_air_con_on(){
   
   CLCD_Init();

   //Air Con On
    CLCD_write_instruction(0x83); // ?????? ????????? 
 
    
   CLCD_write_data('A'); 
   CLCD_write_data('A');
   CLCD_write_data('i'); 
   CLCD_write_data('r'); 
   CLCD_write_data('c'); 
   CLCD_write_data('o'); 
   CLCD_write_data('n'); 
   CLCD_write_data('-'); 
   CLCD_write_data('O'); 
   CLCD_write_data('n'); 

}

void CLCD_air_con_off(){

   CLCD_Init();
   //Air Con Off
   
    CLCD_write_instruction(0x83); // ?????? ????????? 
 
   CLCD_write_data('A');
   CLCD_write_data('A'); 
   CLCD_write_data('i'); 
   CLCD_write_data('r'); 
   CLCD_write_data('c'); 
   CLCD_write_data('o'); 
   CLCD_write_data('n'); 
   CLCD_write_data('-'); 
   CLCD_write_data('O'); 
   CLCD_write_data('f'); 
   CLCD_write_data('f'); 
}

//RGB LED Func -ambient light
/********************************************************************************************************/

void RGB_LED_setColor(int r,int g,int b){
OCR3A=r;
OCR3B=g;
OCR3C=b;
}

void RGB_LED_init(){   // LED_Off

   RGB_LED_setColor(0,0,0); 

}

void RGB_LED_output(){

   if( rgb_led_status == 0)// ????????? 16??????
      RGB_LED_setColor(0,0,0);
   else if( rgb_led_status == 1) //LED_ON
      RGB_LED_setColor(255,255,255);
   else if( rgb_led_status == 2) //LED_RED
      RGB_LED_setColor(255,0,0);
   else if( rgb_led_status == 3) //LED_Green
      RGB_LED_setColor(0,255,0);
   else if( rgb_led_status == 4) //LED_Blue
      RGB_LED_setColor(0,0,255);
   else if( rgb_led_status == 5) //LED_Violet
      RGB_LED_setColor(255,0,255); 

}

//Motor Func - airconditioner
/********************************************************************************************************/
void Servo360_aircon_on(){

   OCR0 = 0x28; // ????????? ??????

   }

void Servo360_aircon_off(){

   OCR0 = 0x00;
   }

//Servo Motor Func - Door
/********************************************************************************************************/
void Servo_open_door(){

   OCR1A = 40; // degree 90 open door
   _delay_ms(1000);

   }

void Servo_close_door(){

   OCR1A = 25; // degree 0 close door
   _delay_ms(1000);
   }

//MCU Init
/********************************************************************************************************/
void Init_interrupt(void){
   //Interrupt Set
   cli();

   EICRA = (0<<ISC31) | (0<<ISC30) | (0<<ISC21) | (0<<ISC20) | (1<<ISC11) | (0<<ISC10) | (0<<ISC01) | (0<<ISC00); 
		// Init Interrupt "Falling Edge"
   EIMSK = (0<<INT7) | (0<<INT6) | (0<<INT5) | (0<<INT4) | (0<<INT3) | (0<<INT2) | (0<<INT1) | (1<<INT0);           
		// Button Interrupt use 'No.0'

   sei();
}

void Init_Timer(void){
   //Timer Set
   cli();
    TIMSK  = 0x04; // Use Overflow interupput
    TCCR1A = 0x82; // OC1A ??????, ?????? ?????? ??? OC1?????? 0, Overflow??? OC1 ?????? 1
    TCCR1B = 0x1D; // Use Fast PWM , Use ICR1, prescail 1024
    
		TCNT1H = 0xFF; // Timer-Counter Register
    TCNT1L = 0xFE; //
    
		TCCR3A = 0xFE;  // OC3A, OC3B, OC3C ??????, ?????? ?????? ??? OC3?????? 1, Overflow??? OC3 ?????? 0
    TCCR3B = 0x1A;  // Past PWM ??????, ICR3 ??????, prescale 8

    sei();
}

void Init_PWM(void){
   cli();
   TCCR0 = (0<<FOC0) | (1<<WGM00) | (1<<COM01) | (0<<COM00) | (1<<WGM01) | (1<<CS02) | (1<<CS01) | (1<<CS00); 
   // clear compare match, prescale 1024, Past PWM ??????, OC0 ??????, ?????? ?????? ??? OC0?????? 1, Overflow??? OC0 ?????? 0
   
   sei();
}

void Pin_Config(void){
   //Pin Cofig
   cli();
    DDRB = 0x30; //PB5      Servo Motor      4 360Servo 
    DDRC = 0xFF; //PC0~7    CLCD output
    DDRD = 0x0A; //PD0      Button           2~3   Bluetooth // ??????????????? ????????? ?????? ?????????
    DDRE = 0x3F; //PE0~2    CLCD             3~5   RGL_LED
    DDRF = 0x00; //PF0      PIR  
   sei();
}

void Bluetooth_Config(){
   //Bluetooth 
   //RX TX 1
   cli();
   UCSR1C = 0x07; // parity ?????? x, 8????????????
   UCSR1B = 0x98; // TX + RX ?????? ??????
   UBRR1L = 0x67; // baud rate 103?????? ?????? (9600)
   sei();

}

void Sensor_module_Config(){
   //CLCD Set

   //RGB LED Set
   PORTE |= 0x0F; //CLCD out 0~2 , Servo 3
   ICR3 = 255; //RGB_LED Range (top)

   //PIR Set
    PORTF = 0x01; //PIR 0 pin Use

   //Servo motor 
   ICR1 = 311; // top

}

void Set_main()
{
   cli();
	
   //Init
   Init_interrupt();
   Init_Timer();
   Init_PWM();
   CLCD_Init(); 

	//Cofig
   Pin_Config();
   Bluetooth_Config();
   Sensor_module_Config();

   sei();
}

//Main
/********************************************************************************************************/
int main()
{
   cli();

   Set_main();

   sei();
   do{

   }while(1);

}