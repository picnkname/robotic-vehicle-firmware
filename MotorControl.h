#ifndef  __MOTORCONTROL_H__
#define  __MOTORCONTROL_H__

/*! \file MotorControl.h
 * \author Lukas Otava
 * \date 2013
 * \defgroup MotorControl MotorControl module
 *
 * Motor control module use the PWM module and the QEI module of microcontroller for speed control of two motors.
 *
 * */

#include "CommonDefs.h"
#include "Regulator.h"
#include "FreeRTOS.h"

//! this macro defines PWM frequency in Hz (H-bridge max frequency limiting parameter)
#define MOTOR_PWM_FREQ		(5000)

//! this macro define regulator sampling frequency in Hz
#define SPEED_REG_FREQ		(50)   //přednastavení QEI časovače pro periodu 20ms (50Hz)

//! motor and encoder coeficient
#define MOTOR_PULSES_PER_VOLT	(700.0)//(767.2)

//! maximal mean value voltage for motor
#define MOTOR_MAX_MEAN_VOLTAGE (6.0)

//! minimal battery voltage - when measured is lower -> motor control will be switched into shutdown state (4.7 V is minimal possible value, 5.5 V for the 2-cell LiIon battery pack)
#define BATERRY_MINIMAL_VOLTAGE (4.7)

//! this macro define PWM output for harmonic balance test
#define HARMONIC_BALANCE_RELAY_LIMIT  (1000)

//! distance between wheels in mm for odometry and speed conversions
#define WHEEL_DISTANCE (65.0)

//! wheel pulses peer revolution - according to the QEI setting and encoder parameters
#define WHEEL_PULSES_PER_REVOLUTION	(4*512)

//! wheel circumference in mm
#define WHEEL_CIRCUMFERENCE	(147.65)

//! motor-wheel gear-box ration n  ( n = n_input/n_output )
#define WHEEL_GEAR_RATIO (100.0/12.0)

//! wheel distance peer EQI pulse
#define WHEEL_DISTANCE_PEER_QEI_PULSE  (WHEEL_CIRCUMFERENCE/WHEEL_GEAR_RATIO/WHEEL_PULSES_PER_REVOLUTION)


#define MOTOR_ENABLE_ODOMETRY (1)

//#define BIPOLAR_PWM_CONTROL

/*! \brief Available states of MotorControl module
 * \ingroup MotorControl */
enum MotorState
{
	//! in this state H-bridge is in Hi-Z
	MOTOR_SHUTDOWN = 0,
	//! H-Bridge in break mode
	MOTOR_STOP = 1,
	//! normal operation
	MOTOR_RUNNING  = 2,
	//! manual control without controller
	MOTOR_MANUAL = 3,
	//! something is wrong
	MOTOR_FAILURE = 4,
	//! harmonic balance test
	MOTOR_HARMONIC_BALANCE = 5
};

/*! \brief Motor identification
 * \ingroup MotorControl */
enum RegID
{
	MOTOR_1 = 0, MOTOR_2 = 1
};

/*! \brief Sensor value with identification of source
 * \ingroup MotorControl */
struct SensorActor
{
	//! identifier
	char id;
	//! value
	short value;
};

/*! \brief This structure holds run-time information and parameters of the motor regulator
 *  \ingroup MotorControl */
struct MotorControl
{
	//! regulator structure for each motor
	struct RegulatorParams reg;

	//! current consumption
	unsigned short current_act;

	//! total current consumption
	unsigned long current_total;

	//! QEI module error counter
	short qei_errors;
};

/*! \brief This structure holds run-time information and parameters of the motor regulators and position
 * \ingroup MotorControl */
struct DriveBlock
{
	//! the first motor structure
	struct MotorControl mot1;

	//! second motor structure
	struct MotorControl mot2;

	//! actual position according to wheel odometry
	struct Position position;

	//! actual battery voltage
	unsigned short batt_voltage;

	//! actual Motor control module state
	enum MotorState state;
};


/*! \brief This structure holds run-time information and parameters of the drive
 * \ingroup MotorControl */
struct DriveBlock myDrive;

/*! \brief This function initialize MotorControl module and task
 *
 *  Example usage:
 *  \code{c}
	//MotorControl module initialization
	if ( MotorControlInit( tskIDLE_PRIORITY+4) != pdPASS)
	{
		// error occurred during MotorControl module initialization!
	}
	\endcode
 * \param priority FreeRTOS priority at which the task should run
 * \ingroup MotorControl
 * \return pdPass value is returned if module task was created correctly
 * \warning This function require FreeRTOS environment */
signed portBASE_TYPE MotorControlInit( unsigned portBASE_TYPE priority);

/*! \brief This function changes between Motor control module modes
 *
 * Example usage:
 *  \code{c}
 *  // set state to running
	MotorControlSetState( MOTOR_RUNNING);
	\endcode
 * \param st desired state
 * \ingroup MotorControl
 *  */
void MotorControlSetState( enum MotorState st);

/*! \brief This function returns current state
 *
 * Example usage:
 *  \code{c}
    //
	if ( MotorControlGetState() != MOTOR_RUNNING )
	{
		// motor controller doesn't running
	}
	\endcode
 * \ingroup MotorControl
 * \return current MotorControl mode
 * \warning */
enum MotorState MotorControlGetState( void);

/*! \brief This function waits for new MotorControl period with new measured data
 *
 * This could be useful for logging or for synchronization with other parts of system
 *
 * Example usage:
 *  \code{c}
	if( MotorControlWaitData(50) == pdTRUE )
	{
		// we have new valid data
	}
	\endcode
 * \param timeout waiting timeout in tick
 * \return pdTrue is returned if correct
 * \ingroup MotorControl
 * */
signed portBASE_TYPE MotorControlWaitData(portTickType timeout);

/*! \brief This function can set a speed for both motors
 *
 * The input speed is signed short integer. Dimension of this speed is incremental sensor ticks peer sampling period.
 * eg. \f$ [v_1 =  \frac{ticks}{period}\f$
 *
 * Example usage:
 *  \code{c}
 *  // set new desired speed to 100 and 200 ticks per period
	MotorControlSetSpeed(100,200);
	\endcode
 * \param v1 desired speed for motor 1
 * \param v2 desired speed for motor 2
 * \ingroup MotorControl
 * */
void MotorControlSetWheelSpeed(signed short v1, signed short v2);

/*! \brief This function can set a speed for differential drive as forward and angular speed.
 *
 * Example usage:
 *  \code{c}
 *  // set new desired speed to 100 mm/s and 200 rad/s ticks per period
	MotorControlSetSpeed(100,200);
	\endcode
 * \param v forward speed
 * \param w rotation speed
 * \ingroup MotorControl
 * */
void MotorControlSetSpeed(float v, float w);


void RegulatorOdometryReset( void);

// FreeRTOS task
void MotorControl_task( void * param);


#endif//__MOTORCONTROL_H__
