/*
Team Members:

Hassan Mohamed Mohamed Fathy El-Tobgy 			
Mazen Mohamed Elsaied 					
Farida Mohamed Mohamed El-husseiny 			
Mariam Dia el Den Nady 					
Abdulrahman Maged Abdulrahman				 
Hazem Zaki Aboukhalil Zaki Ahmed 				




Hardware Connections:
		//Inputs 
	****	Driver on Port A ****
PA2 - Manual Driver  Up & Automatic Driver up
PA3 - Manual Driver  Down & Automatic Driver Down
	****	Passenger on Port B ****
PB6 - Manual & Automatic Passenger Up
PB7 - Manual & Automatic Passenger Down

	**** Special Cases ****	
PA4 - Jamming
PA5 - Lock
	**** Switches ****
PC4 - Window down Limit switch 
PC5 - Window up Limit switch

		****Motor****
PE1 - MOTOR A1
PE2 - MOTOR A2
*/

    /***Includes***/
#define PART_TM4C123GH6PM		
#include "TM4C123GH6PM.h"
#include <FreeRTOS.h>
#include "task.h"
#include <semphr.h>
#include <driverlib/gpio.c>
#include <driverlib/gpio.h>
#include <driverlib/sysctl.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"

			/*Function declarations*/
void init(); // Initializations
void GPIOA_Handler(); // Driver Isr and buttons of Driver in PORTA
void GPIOB_Handler(); // Passenger Isr and buttons of Passenger in PORTB

		/*Just a Simple Delay function in ms to eliminate bouncing problem of buttons  */
void delayMs(uint32_t n)
{
  int i,j;
  for(i=0;i<n;i++)
  {
    for(j=0;j<3180;j++)
    {}
  }
}

				/***Tasks***/


void jamming (void* params);
void lock (void* params);
void driverup_1_button(void* params);
void driverdown_1_button(void* params);
void passengerup_1_button(void* params);
void passengerdown_1_button(void* params);

/****Flags are used in this code to ensure that code will go as exactly intended ****/
int flag_if_Driver_up;
int flag_if_Driver_down;
int flag_if_passenger_up;
int flag_if_passenger_down;
/*Mutex Declarations*/
							
SemaphoreHandle_t MotorMutex;
							
							/*Queue Declarations*/
							
xQueueHandle xupQueue;

					/*Semaphores declarations*/


SemaphoreHandle_t S_jamming;
SemaphoreHandle_t S_lock;
SemaphoreHandle_t S_driverup_handler;
SemaphoreHandle_t	S_driverdown_handler;
SemaphoreHandle_t S_passengerup_handler;
SemaphoreHandle_t S_passengerdown_handler;


int main()
{
							/***Initializations***/
	
	init();
	
					/***Create the semaphhores***/
	

	S_jamming = xSemaphoreCreateBinary();
	S_lock = xSemaphoreCreateBinary();
	S_driverup_handler= xSemaphoreCreateBinary(); //Semaphore for Handler driver automatic & manual up.
	S_driverdown_handler= xSemaphoreCreateBinary();//Semaphore for Handler driver automatic * manual down.
	S_passengerup_handler=xSemaphoreCreateBinary();
	S_passengerdown_handler=xSemaphoreCreateBinary();
	

					/*****Create Mutex****/
	
	MotorMutex = xSemaphoreCreateMutex();
	
					/***Create the tasks***/
 

  xTaskCreate(jamming,"jamming",80,NULL,4,NULL);
  xTaskCreate(lock,"lock_passenger",80,NULL,3,NULL);
 xTaskCreate(driverup_1_button,"Handler of Defered Driver up",80,NULL,3,NULL);
 xTaskCreate(driverdown_1_button,"Handler of Defered Driver down",80,NULL,3,NULL);
 xTaskCreate(passengerup_1_button,"Handler of Defered Passenger Up",80,NULL,2,NULL);
  xTaskCreate(passengerdown_1_button,"Handler of Defered Passenger Down",80,NULL,2,NULL);
	
						/***Create the Queues***/
						
	xupQueue = xQueueCreate( 1, sizeof(int));

						/***Start the scheduler***/
	vTaskStartScheduler();
	
for(;;)
	{
	// This loop should never be reached	
	}
}

									/**Functions**/

void init() // Initializations of GPIOs
{
		SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
		while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA))
			;
		SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
		while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB))
			;
		SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
		while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOC))
			;
		SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);	
		while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE))
			;
		SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);	
		while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF))
			;
		// Configure pins of PORTA
		GPIOPinTypeGPIOInput(GPIO_PORTA_BASE,GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4|GPIO_PIN_5);
		GPIOPadConfigSet(GPIO_PORTA_BASE,GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4  , GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
		GPIOIntEnable(GPIO_PORTA_BASE,GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5);
		GPIOIntTypeSet(GPIO_PORTA_BASE, GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4| GPIO_PIN_5, GPIO_FALLING_EDGE); //All buttons, limit switches, and lock switch are set to work on falling edge detection
		GPIOIntRegister(GPIO_PORTA_BASE, GPIOA_Handler);
		IntPrioritySet(INT_GPIOA, 0XE0);
		
		// Configure pins of PORTB
		GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_6 | GPIO_PIN_7);
		GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_6 | GPIO_PIN_7, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
		GPIOIntEnable(GPIO_PORTB_BASE, GPIO_PIN_6 | GPIO_PIN_7);
		GPIOIntTypeSet(GPIO_PORTB_BASE, GPIO_PIN_6 | GPIO_PIN_7, GPIO_FALLING_EDGE);
		GPIOIntRegister(GPIO_PORTB_BASE, GPIOB_Handler);
		IntPrioritySet(INT_GPIOB, 0XE0);
		
		//Configure pins of PORTC
		GPIOPinTypeGPIOInput(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5);
		GPIOPadConfigSet(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 , GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
		
		//Configure pins of PORTE
		GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2); 		// PORTE output pins for the motor
		GPIOUnlockPin(GPIO_PORTE_BASE, GPIO_PIN_1 | GPIO_PIN_2);
			
}

				/**Interrupt Service Routines**/
/* Isr of Port A used For Driver thats why it has higher  priority than Isr of passenger */
void GPIOA_Handler() 
	{
		delayMs(10); // Remove Deboucning effect 
		
	BaseType_t xHigherPriorityTaskWoken= pdFALSE;    //Highest priority flag is initialized to false , the flag is used for context switching if a higher or equal priority task is preempted from the ISR
	/*******Handler for Manual Driver up & Automatic Driver Up******/
	if (GPIOPinRead(GPIO_PORTA_BASE,GPIO_PIN_2) != GPIO_PIN_2) 

	{
		GPIOIntClear(GPIO_PORTA_BASE,GPIO_PIN_2); // Clear Interrupt
		xSemaphoreGiveFromISR(S_driverup_handler,&xHigherPriorityTaskWoken);
		portEND_SWITCHING_ISR(xHigherPriorityTaskWoken); //
	}
	/********Handler for Manual Driver Down & automatic driver down******/
		else if (GPIOPinRead(GPIO_PORTA_BASE,GPIO_PIN_3) != GPIO_PIN_3)  // Check if the button still pressed after debouncing effect is removed 
	{
		GPIOIntClear(GPIO_PORTA_BASE,GPIO_PIN_3); // Clear Interrupt
		xSemaphoreGiveFromISR(S_driverdown_handler,&xHigherPriorityTaskWoken);
		flag_if_Driver_down=0;
		portEND_SWITCHING_ISR(xHigherPriorityTaskWoken); //context switching if flag == 1
	}
	
	
	/**** Lock ***/
	else if ((GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_5))!= GPIO_PIN_5)  // Check if the button still pressed after debouncing effect is removed 
{
    
		GPIOIntClear(GPIO_PORTA_BASE, GPIO_PIN_5); 
    xSemaphoreGiveFromISR(S_lock, &xHigherPriorityTaskWoken);
    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);  
		
}

	/***Jamming***/
	
else if (GPIOPinRead(GPIO_PORTA_BASE,GPIO_PIN_4) != GPIO_PIN_4) // check anhy button kan metdas (fel tivaware) 
  {
 		GPIOIntClear(GPIO_PORTA_BASE,GPIO_PIN_4); 
 		xSemaphoreGiveFromISR(S_jamming,&xHigherPriorityTaskWoken);
 		portEND_SWITCHING_ISR(xHigherPriorityTaskWoken); 
 }
	
	else 
		{
			/* if entered here this means that buttin debounced */
		GPIOIntClear(GPIO_PORTA_BASE,GPIO_PIN_2);
		GPIOIntClear(GPIO_PORTA_BASE,GPIO_PIN_3);
		GPIOIntClear(GPIO_PORTA_BASE,GPIO_PIN_5);
		GPIOIntClear(GPIO_PORTA_BASE,GPIO_PIN_4);		
	  }
}
	
void GPIOB_Handler() //ISR of PORT B, Checks which button has been pressed
	{
		delayMs(10);
	BaseType_t xHigherPriorityTaskWoken= pdFALSE;    //Highest priority flag is initialized to false , the flag is used for context switching if a higher or equal priority task is preempted from the ISR
	/********Automatic Driver Up******/

	/********Automatic Passenger Up & Manual Passenger Up******/
	 if (GPIOPinRead(GPIO_PORTB_BASE,GPIO_PIN_6) != GPIO_PIN_6)
	{
		GPIOIntClear(GPIO_PORTB_BASE,GPIO_PIN_6); // Clear Interrupt
		xSemaphoreGiveFromISR(S_passengerup_handler,&xHigherPriorityTaskWoken);
		portEND_SWITCHING_ISR(xHigherPriorityTaskWoken); //context switching if flag == 1
	}

	/********Automatic Passenger Down & Manual Passenger Down******/
		else if (GPIOPinRead(GPIO_PORTB_BASE,GPIO_PIN_7) != GPIO_PIN_7) 
	{
		GPIOIntClear(GPIO_PORTB_BASE,GPIO_PIN_7); // Clear Interrupt
		xSemaphoreGiveFromISR(S_passengerdown_handler,&xHigherPriorityTaskWoken);
		portEND_SWITCHING_ISR(xHigherPriorityTaskWoken); //context switching if flag == 1
	}
	else 
		{
		
		GPIOIntClear(GPIO_PORTB_BASE,GPIO_PIN_6);
		GPIOIntClear(GPIO_PORTB_BASE,GPIO_PIN_7);
	  }
}
	
					/**Tasks**/
void driverup_1_button (void *params)	
{

 xSemaphoreTake(S_driverup_handler,0);//safety mechanism
	for(;;)
	{
			
		xSemaphoreTake(S_driverup_handler,portMAX_DELAY);
		portTickType xLastWakeTime;
		
		xLastWakeTime = xTaskGetTickCount();
		vTaskDelayUntil( &xLastWakeTime, ( 1000 / portTICK_RATE_MS ) );
		
		//You have to press long button  this will give you manual driver up 
		if (GPIOPinRead(GPIO_PORTA_BASE,GPIO_PIN_2) != GPIO_PIN_2) //button pressed even after the delay --> manual up 
		{

	xSemaphoreTake(MotorMutex, portMAX_DELAY);
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1,GPIO_PIN_1); //Turn the motor on
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_2,0x00000000);
		while(GPIOPinRead(GPIO_PORTA_BASE,GPIO_PIN_2) != GPIO_PIN_2 && GPIOPinRead(GPIO_PORTC_BASE,GPIO_PIN_5) ==  GPIO_PIN_5) //As long as the corresponding button is pressed, and the corresponding limit switch is not reached, the code will be stuck in here , and the motor will stay on 
		{
//			int x=5;
//			int y=3;
//			int z;
//			
//			z=x+y; 
// was used for debugging 
			
			
		}
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1,0x00);  //Turn the motor off
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_2,0x00);
		xSemaphoreGive(MotorMutex);
		
		
		
	
		}
		else{				
		
		//short press automatic driver  up 
		xSemaphoreTake(MotorMutex, portMAX_DELAY);
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1,GPIO_PIN_1); //Turn the motor on
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_2,0x00);	
			while(GPIOPinRead(GPIO_PORTC_BASE,GPIO_PIN_5) ==  GPIO_PIN_5) //Keep the motor on as long as the corresponding limit switch is not reached
			{
				int auto_pressed = 0;
				if(xQueueReceive(xupQueue, &auto_pressed,0) == pdTRUE)	//check if jamming occured
				{
					if(auto_pressed == 1){ //check if the window is currently in automatic up mode
					// Stop the motor			
					GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1,0x00);
					GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_2,0x00);
					delayMs(300);
					// Bring the window down for a little bit	
					GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1,0x00); 
					GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_2,GPIO_PIN_2);
					delayMs(500);
					GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1,0x00);
					GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_2,0x00);
					break;	// Break from the nested if condition to check if the jamming condition is cleared
					}
				 }
			}
			
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1,0x00); //Turn the motor off
	  GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_2,0x00);
		xSemaphoreGive(MotorMutex);
		

		 
			}

		
	
	}

	}

void driverdown_1_button(void *params)
{
xSemaphoreTake(S_driverdown_handler,0);
	for(;;)
	{	 
		xSemaphoreTake(S_driverdown_handler,portMAX_DELAY);
		portTickType xLastWakeTime;
		xLastWakeTime = xTaskGetTickCount();
		vTaskDelayUntil( &xLastWakeTime, ( 1000 / portTICK_RATE_MS ) );
		if (GPIOPinRead(GPIO_PORTA_BASE,GPIO_PIN_3) != GPIO_PIN_3) // if pressing for long time this will give you manual down.
		{
			xSemaphoreTake(MotorMutex, portMAX_DELAY);
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1,0x00); //Turn the motor on
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_2,GPIO_PIN_2);
			//long press will give you manual driver down 
	while(GPIOPinRead(GPIO_PORTA_BASE,GPIO_PIN_3) != GPIO_PIN_3 && GPIOPinRead(GPIO_PORTC_BASE,GPIO_PIN_4) ==  GPIO_PIN_4) //As long as the corresponding button is pressed, and the corresponding limit switch is not reached, the code will be stuck in here , and the motor will stay on 
	{
		
		}
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1,0); //Turn the motor off
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_2,0x0);
		xSemaphoreGive(MotorMutex);
		
		
			
		}
		// short press will give you autmatic driver down 
		else  {
			
			xSemaphoreTake(MotorMutex, portMAX_DELAY);
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1,0x00); //Turn the motor on
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_2,GPIO_PIN_2);
		while(GPIOPinRead(GPIO_PORTC_BASE,GPIO_PIN_4) ==  GPIO_PIN_4) //Keep the motor on as long as the corresponding limit switch is not reached
			{
			
			}
			
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1,0x00); //Turn the motor off
	  GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_2,0x00);
   	  xSemaphoreGive(MotorMutex);
	
}
	}
}

void passengerup_1_button(void *params)
{
	xSemaphoreTake(S_passengerup_handler,0);
	 //safety mechanism
	for(;;)
	{
		
		xSemaphoreTake(S_passengerup_handler,portMAX_DELAY);
		portTickType xLastWakeTime;
		xLastWakeTime = xTaskGetTickCount();
		vTaskDelayUntil( &xLastWakeTime, ( 1000 / portTICK_RATE_MS ) );
		//You have to press long button to get manual passenger up 
		if (GPIOPinRead(GPIO_PORTB_BASE,GPIO_PIN_6) != GPIO_PIN_6) //button pressed even after the delay --> manual up 
		{

	GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1,GPIO_PIN_1); //Turn the motor on
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_2,0x00);
			
		while((GPIOPinRead(GPIO_PORTB_BASE,GPIO_PIN_6) != GPIO_PIN_6) && (GPIOPinRead(GPIO_PORTC_BASE,GPIO_PIN_5) ==  GPIO_PIN_5)) //As long as the corresponding button is pressed, and the corresponding limit switch is not reached, the code will be stuck in here , and the motor will stay on 
		{
			
		}
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1,0x00); //Turn the motor off
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_2,0x00);
		
		
		
		
		
		}
		else {
		
		//short press passenger automatic up 
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1,GPIO_PIN_1); //Turn on the motor
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_2,0x00);
			while(GPIOPinRead(GPIO_PORTC_BASE,GPIO_PIN_5) ==  GPIO_PIN_5) //Keep the motor on as long as the corresponding limit switch is not reached
			{
				int auto_pressed = 0;
				if(xQueueReceive(xupQueue, &auto_pressed,0) == pdTRUE)//Check if jamming occured
				{
					if(auto_pressed == 1){ //check if the window is currently in automatic up mode
					// Stop the motor			
					GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1,0x00);
					GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_2,0x00);
					delayMs(300);
					// Bring the window down for a little bit	
					GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1,0x00); 
					GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_2,GPIO_PIN_2);
					delayMs(500);
					GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1,0x00);
					GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_2,0x00);
					break;	
					}
			}
			}
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1,0x00); // Turn off the motor
	  GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_2,0x00);
		


		


}
		
	
	}

	
}
void passengerdown_1_button(void* params){
	xSemaphoreTake(S_passengerdown_handler,0);
	for(;;)
	{ 
		
		
		xSemaphoreTake(S_passengerdown_handler,portMAX_DELAY);
		portTickType xLastWakeTime;
		xLastWakeTime = xTaskGetTickCount();
		vTaskDelayUntil( &xLastWakeTime, ( 1000 / portTICK_RATE_MS ) );
		//long press manual passenger down 
		if (GPIOPinRead(GPIO_PORTB_BASE,GPIO_PIN_7) != GPIO_PIN_7)
		{
		
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1,0x00); //Turn the motor on
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_2,GPIO_PIN_2);
		while((GPIOPinRead(GPIO_PORTB_BASE,GPIO_PIN_7) != GPIO_PIN_7) && (GPIOPinRead(GPIO_PORTC_BASE,GPIO_PIN_4) ==  GPIO_PIN_4)) //As long as the corresponding button is pressed, and the corresponding limit switch is not reached, the code will be stuck in here , and the motor will stay on
		{
		// empty
		}
    GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1, 0x00);// Turn the motor off
    GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_2, 0x00);			
			
	}
		//short press automatic passenger down 
		else{
			GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1,0x00); //Turn on the motor
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_2,GPIO_PIN_2);
				while(GPIOPinRead(GPIO_PORTC_BASE,GPIO_PIN_4) ==  GPIO_PIN_4) //Keep the motor on as long as the corresponding limit switch is not reached
			{
			// empty	
			}	
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1,0x00); //Turn off the motor
	  GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_2,0x00);
			}
	
}

}




	


void lock(void *params)
{
	xSemaphoreTake(S_lock, 0);
	for(;;)
	{
	xSemaphoreTake(S_lock, portMAX_DELAY);	
	while(!(GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_5))) //Check if lock switch is pressed
		{
			GPIOIntDisable(GPIO_PORTA_BASE, GPIO_INT_PIN_6 | GPIO_INT_PIN_7 ); //Disable manual control by the passenger
			GPIOIntDisable(GPIO_PORTB_BASE, GPIO_INT_PIN_6 | GPIO_INT_PIN_7 ); //Disable automatic control by the passenger


		}
			GPIOIntEnable(GPIO_PORTA_BASE, GPIO_INT_PIN_6 | GPIO_INT_PIN_7 ); //Enable manual control by the passenger
			GPIOIntEnable(GPIO_PORTB_BASE, GPIO_INT_PIN_6 | GPIO_INT_PIN_7 ); //Enable automatic control by the passenger
		}
}

void jamming (void* params)
	{
		 
		 xSemaphoreTake(S_jamming,0); 
		for(;;)
		{
		
		xSemaphoreTake (S_jamming, portMAX_DELAY);
		const TickType_t xDelay = 10 / portTICK_RATE_MS; //standard delay to be used with queues
		int flag = 1;
		xQueueSend(xupQueue,&flag,xDelay);	//Queue that let's other tasks know that jamming was detected
	}
}
	
