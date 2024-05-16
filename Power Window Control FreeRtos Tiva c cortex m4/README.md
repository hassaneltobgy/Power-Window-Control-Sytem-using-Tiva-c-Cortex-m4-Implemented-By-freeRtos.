# CSE411 Major Task: Power Window Control System using Tiva-C running FreeRTOS
## Project by Team 4 Mechatronics and Automation
This is a repository for the major task of the **CSE411: Real-Time and Embedded Systems Design**  course taught during the Spring semester of 2023 at the faculty of engineering, Ain Shams University.

**Professor**: Dr. Sherif Hammad

**Teaching Assistants**: Eng. Hesham Salah and Eng. Mohamed Tarek

**Team 4 members**

| Name | ID |
| --- | --- |
| Ahmed Khaled Mohamed Fathy | 19P1787 |
| Amr Issa El-Sayed Mohamed Tawfik | 18P8601 |
| Assem Khaled Farouk Morsi El-Barky | 18P4802 |
| Marwan Ayman Mahmoud Elatfehy | 19P2588 |
| John Ashraf Adeeb | 18P5411 |
| Marina Mourad KaramAllah | 18P7884 |

## Project Description
The project target is to program a control system for a power window using a TivaC123GH6PM running FreeRTOS.

### Project scope
1. Implementation of front passenger door window with both passenger and driver control
panels.
2. FreeRTOS implementation is a must.
3. Implementation of 2 limit switches to limit the window motor from top and bottom limits of
the window.
4. Obstacle detection implementation is required, with a push button used to indicate jamming instead of a sensor.

The desired area to be controlled is shown in the following figure:

![image](https://github.com/AssemEl-Barky/CSE411-Major-Task-Powerwindowcontrolsystem-using-Tiva-C-running-FreeRTOS-by-Team-4-MCTA/assets/63543410/9b763744-29bb-4c17-afda-add8a0d02ada)

### System basic features
1. Manual open/close function
  
     When the power window switch is pushed or pulled
continuously, the window opens or closes until the switch
is released.

2. One touch auto open/close function

     When the power window switch is pushed or pulled
shortly, the window fully opens or closes.

3. Window lock function

      When the window lock switch is turned on, the opening and closing of
all windows except the driver’s window is disabled.

4. Jam protection function

     This function automatically stops the power window and moves it
downward about 0.5 second if foreign matter gets caught in the
window during one touch auto close operation.

## Project Implementation
### Circuit Schematic
The following figure shows the circuit schematic

![image](https://github.com/AssemEl-Barky/CSE411-Major-Task-Powerwindowcontrolsystem-using-Tiva-C-running-FreeRTOS-by-Team-4-MCTA/assets/63543410/1268f241-f83b-421a-b252-1085f9200c75)
### Hardware
The following figure shows a hardware implementation of the circuit. 

![WhatsApp Image 2023-05-15 at 8 27 15 PM](https://github.com/AssemEl-Barky/CSE411-Major-Task-Powerwindowcontrolsystem-using-Tiva-C-running-FreeRTOS-by-Team-4-MCTA/assets/63543410/889dab75-5344-4f9d-83df-ad73eba094d0)

The buttons from left to right are:
1. Manual driver up
2. Manual driver down
3. Manual passenger up
4. Manual passenger down
5. Automatic driver up
6. Automatic driver down
7. Automatic passenger up
8. Automatic passenger down
9. Button to simulate the jamming sensor

There are 2 limit switches for the maximum upwards and downwards positions, as well as a normal switch used for locking the passenger control.

All inputs are connected using pull-up resistances (operating on inverted logic).
### Finite state machine
The system can be considered as a finite state machine which can be seen in the follwing diagram.

![MCTA Team 4 CSE411 State Flow ](https://github.com/AssemEl-Barky/CSE411-Major-Task-Powerwindowcontrolsystem-using-Tiva-C-running-FreeRTOS-by-Team-4-MCTA/assets/63543410/d54894a5-7e52-43e2-a994-2de049498a12)
### Code implementation
The code is implemented using the Tivaware library for interfacing with the GPIOs of the Tiva microcontrollel, and the FreeRTOS kernel as the system's OS.

The OS consists of **10 tasks**, **10 Semaphores**, **1 Queue**, and **1 Mutex**

_tasks_

There are 10 tasks; one for each input button, and one for the lock switch

_Semaphores_

There are 10 semaphores ; one for each input button, and one for the lock switch. These semaphores are utilized in each respective task according to the pressed button

_Queue_

There is a single queue which is used for communication between the jamming task and the automatic up tasks.

_Mutex_

There is a single mutex used to allow control for the shared resource which is the motor.
### Demonstration video
The project [demostration video](https://drive.google.com/drive/folders/1Wlv8UiPhQYOf2a-1ebrvUrTnOT7fQLjv?usp=sharing) 
