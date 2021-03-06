#include "Regulator.h"

#include <math.h>

short RegulatorAction(struct RegulatorParams * rp, short measurement, unsigned char manualMode)
{
	float flOut;
	short siOut;

	// measurement store
	rp->measured = measurement;

	// error value
	rp->error = (rp->desired - rp->measured);

	// modified PID algorithm

	// regulator output (action value)
	flOut = (rp->Kr *((rp->Beta*(float)rp->desired) - (float)rp->measured))
			+ rp->sum;

#if REGULATOR_PI_VERSION_ONLY != 1
	// derivation filter constant
	rp->Dexp = expf(-rp->N/rp->Td);

	//derivation value
	flOut += (rp->N*rp->Kr*( -rp->measured + ( rp->der * rp->Dexp ) - rp->der ) );

	// derivation update Xd2 = ( Xd2 * exp(-1*Ts*N)/Td))-in;
	rp->der = (rp->der * rp->Dexp) - rp->measured;
#endif

	// integration update
	rp->sum += ( rp->Kr * rp->Ti * rp->error ) + ( rp->Kip * rp->saturationDiff / rp->outputScale );


	// manual/automatic mode switch
	if (manualMode > 0 ) // manual mode
	{
		// converter gain correction
		siOut = (short)( rp->desired * rp->outputScale);
		// save data
		rp->action = rp->desired;
	}
	else // automatic mode
	{
		// converter gain correction
		siOut = (short)( flOut * rp->outputScale);

		// save data
		rp->action = (short)flOut;
	}

	// output non-linearity saturation model
	if (siOut >  rp->limit)
	{
		siOut =  rp->limit;
	}
	#if REG_UNIPOLAR_LIMIT == 1
		if (siOut < 0 )
		{
			siOut = 0;
		}
	#else
		if (siOut < -rp->limit)
		{
			siOut = -rp->limit;
		}
	#endif

	// over-integration protection feedback
	rp->saturationDiff = (float)siOut - (flOut * rp->outputScale);

	if (rp->saturationDiff >  rp->limit)
	{
		rp->saturationDiff =  rp->limit;
	}
	if (rp->saturationDiff <  -rp->limit)
	{
		rp->saturationDiff =  -rp->limit;
	}

	return siOut;
}

void RegulatorResetStates(struct RegulatorParams * rp)
{
	// we only clear state variables
	rp->der = 0;
	rp->sum = 0;
	rp->saturationDiff = 0;
	rp->desired = 0;
	rp->action = 0;
}

void RegulatorSetPID(struct RegulatorParams * rp, float Kr, float Ti, float Td)
{
	// we only transfer values into structure
	rp->Kr = Kr;
	rp->Ti = Ti;
	rp->Td = Td;
}

void RegulatorSetParams(struct RegulatorParams * rp, float Beta, float Kip, float N)
{
	// we only transfer values into structure
	rp->Beta = Beta;
	rp->Kip = Kip;
	rp->N = N;
}

void RegulatorSetScaleLimit(struct RegulatorParams * rp, float outputScale, unsigned short limit)
{
	// we only transfer values into structure
	rp->limit = limit;
	rp->outputScale  = outputScale;
}

void RegulatorSetDesired(struct  RegulatorParams * rp, short desiredValue)
{
	// we only transfer value into structure
	rp->desired = desiredValue;
}
