#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"

//#include "semihosting.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "timers.h"

#include "trcUser.h"

#include "CommonDefs.h"
#include "StatusLED.h"
#include "Accelerometer.h"
#include "Motorcontrol.h"
#include "SlipSerial.h"
#include "CANtest.h"


void vSystemInit( void)
{
	//! CLOCK setup to 20 MHz
	SysCtlClockSet(SYSCTL_SYSDIV_10 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_6MHZ);
}

//! temporary motor mode
char motorMode = MOTOR_RUNNING;

void SlipSerialProcessPacket(char packet_buffer[], int length)
{
	//packet arrived
	switch (packet_buffer[0])
	{
	case ID_MOTOR_ACT:
		if (length<5)
		{
			//corrupted data
		}
		else
		{
			//need to be tested
		}
		ClearError(ERROR_COMM_SLIP);
		MotorControlSetSpeed(packet_buffer[1]|(packet_buffer[2]<<8), packet_buffer[3]|(packet_buffer[4]<<8));
		MotorControlSetState(motorMode);
		break;
	case ID_MOTOR_MODE:
		if (length<2)
		{
			//corrupted data
		}
		else
		{
			motorMode = packet_buffer[1];
			MotorControlSetState(motorMode);
		}
		break;
	case ID_REG_PARAMS:
		if (length<6)
		{
			//corrupted data
		}
		else
		{
			memcpy(&myDrive.mot1.reg.K,&packet_buffer[1],20);
			memcpy(&myDrive.mot2.reg.K,&packet_buffer[1],20);
		}
		break;
	}
}

void SlipSerialReceiveTimeout( void)
{
	//  timeout
	MotorControlSetState(MOTOR_SHUTDOWN);
	SetError(ERROR_COMM_SLIP);
}

void ControlTask_task( void * param)
{
#if configUSE_TRACE_FACILITY==1
	short taskListPre = 100;
#endif

	while(1)
	{
		if( MotorControlWaitData(50) == pdTRUE )
		{
			short regvalues[6];
			unsigned short values[3];
			char timestamp = 1;

			//acc values
			SlipSend(ID_ACC_STRUCT,(char *) &accData,2*sizeof(short int));
			if (AccelerometerRequestData() == pdPASS)
			{
				//OK
			}

			//regulator values
			regvalues[0] = myDrive.mot1.reg.actual;
			regvalues[2] = myDrive.mot1.reg.action;
			regvalues[4] = myDrive.mot1.reg.error;
			regvalues[1] = myDrive.mot2.reg.actual;
			regvalues[3] = myDrive.mot2.reg.action;
			regvalues[5] = myDrive.mot2.reg.error;

			SlipSend(ID_REG,(char *) &regvalues, sizeof(short[6]));

			//data ADC
			values[0] = myDrive.mot1.current_act;
			values[1] = myDrive.mot2.current_act;
			values[2] = myDrive.mot2.reg.batt_voltage;

			SlipSend(ID_ADC,(char *) &values, sizeof(unsigned short[3]));

			SlipSend(ID_MOTOR_MODE, (char *) &myDrive.state, sizeof(enum MotorState) );

			//timestamp
			SlipSend(ID_TIME_STAMP,(char *) &timestamp, sizeof(char));
		}
		else
		{
			//timeout
			//data outage
		}

#if configUSE_TRACE_FACILITY==1
		//ka�d� 2s
		taskListPre--;
		if (taskListPre == 0)
		{
			taskListPre = 100;

			char tasklist[256];
			vTaskList((signed char*)tasklist);
			SlipSend(ID_TASKLIST,tasklist, strlen(tasklist));
		}
#endif

	}
}

signed portBASE_TYPE ControlTaskInit( unsigned portBASE_TYPE priority )
{
	// we only create control task
	return xTaskCreate(ControlTask_task, (signed portCHAR *) "CTRL", 256, NULL, priority , NULL);
}



int main(void)
{
	//! inicializace z�kladn�ch periferi� syst�mu
	vSystemInit();

	//! HeartBeat �loha indika�n� diody
	if (StatusLEDInit(tskIDLE_PRIORITY+1) != pdPASS)
	{
		//error
	}

	//! SlipSerial komunikace po seriov� lince
	if (SlipSerialInit(tskIDLE_PRIORITY +2, 57600, 200) != pdPASS)
	{
		//error
	}

	//! MotorControl module initialization
	if (MotorControlInit(tskIDLE_PRIORITY+4) != pdPASS)
	{
		// error occurred during MotorControl module initialization
	}

	//! �loha akcelerometru
	if (AccelerometerInit(tskIDLE_PRIORITY +3) != pdPASS)
	{
		//error
	}

	//! �loha ��zen�  - tady budou kraviny kolem, logov�n� atd
	if ( ControlTaskInit(tskIDLE_PRIORITY +2) != pdPASS)
	{
		//error
	}

	//! CANTest task
	if (xTaskCreate(CANtest_task, (signed portCHAR *) "CAN", 256, NULL, tskIDLE_PRIORITY +1, NULL) != pdPASS)
	{
		//error
	}


#if configUSE_TRACE_FACILITY==1
	//! FreeRTOs kernel object trace record start
	if (uiTraceStart() != 1)
		while(1){};
#endif

	//! FreeRTOS scheduler start
	vTaskStartScheduler();

	//! never entering code
	GPIOPinWrite(LED_RED_PORT, LED_RED, ~LED_RED);
	while(1){};
	return 0;
}

