
#include "RLSE.h"

matrixType* phiT = NULL;
matrixType* temp_1n = NULL;
matrixType* temp11 = NULL;
matrixType* temp_nn = NULL;
matrixType* temp_nn_2 = NULL;

// wars - all system must be same order :-( in consequence to shared temporary matrices
void rlse_init( rlseType* rlses)
{
	//iterator variable declaration
	//matrixSizeType i;

	// individual rls matrix allocation
	rlses->P 	= matAlloc(RLSE_SYSTEM_PARAMETERS,RLSE_SYSTEM_PARAMETERS);
	rlses->th  	= matAlloc(RLSE_SYSTEM_PARAMETERS,1);
	rlses->K   	= matAlloc(RLSE_SYSTEM_PARAMETERS,1);
	rlses->phi 	= matAlloc(RLSE_SYSTEM_PARAMETERS,1);
#if RLSE_IMV_TYPE == 1
	rlses->DZ 	= matAlloc(RLSE_SYSTEM_PARAMETERS,1);
#endif

	//rlses->delta = matAlloc(RLSE_SYSTEM_PARAMETERS,1);

	// shared temporary matrix allocation
	if (phiT == NULL)
	{
		phiT		= matAlloc(1,RLSE_SYSTEM_PARAMETERS);
		temp_1n 	= matAlloc(1,RLSE_SYSTEM_PARAMETERS);
		temp11		= matAlloc(1,1);
		temp_nn		= matAlloc(RLSE_SYSTEM_PARAMETERS,RLSE_SYSTEM_PARAMETERS);
		temp_nn_2	= matAlloc(RLSE_SYSTEM_PARAMETERS,RLSE_SYSTEM_PARAMETERS);
	}

	rlse_reinit(rlses);
	/*// matrix initialization
	matEye(rlses->th, 1.0);
	matEye(rlses->P,  RLSE_P_MATRIX_INIT);

	// past value vector init
	for( i = 0; i < (RLSE_SYSTEM_PARAMETERS-1); i++)
	{
		rlses->past_values[i] = 0;
	}*/
}

void rlse_reinit( rlseType* rlses)
{
	//iterator variable declaration
	matrixSizeType i;

	// matrix initialization
	matEye(rlses->th, 1.0);
	matEye(rlses->P,  RLSE_P_MATRIX_INIT);

	// past value vector init
	for( i = 0; i < (RLSE_SYSTEM_PARAMETERS-1); i++)
	{
		rlses->past_values[i] = 0;
	}
}

void rlse_update( rlseType* rlses, matrixValType y, matrixValType u, matrixSizeType condition)
{
	matrixValType eps;
	//matrixSizeType i;

	//for(i = 0; i < (RLSE_SYSTEM_PARAMETERS/2); i++)
	//	rlses->phi->mat[i] = -rlses->past_values[i];

	//update vstup�
	rlses->phi->mat[0] = -rlses->past_values[Y1];//ya1;
	rlses->phi->mat[1] = -rlses->past_values[Y2];

	rlses->phi->mat[2] =  u;
	//for (i = RLSE_SYSTEM_PARAMETERS; i <  RLSE_SYSTEM_PARAMETERS; i++ )
	//	rlses->phi->mat[i] =  rlses->past_values[i-1];
	rlses->phi->mat[3] =  rlses->past_values[U1];

#if RLSE_IMV_TYPE == 1
	rlses->DZ->mat[0] = -rlses->past_values[Y3];
	rlses->DZ->mat[1] = -rlses->past_values[Y4];
	rlses->DZ->mat[2] =  u;
	rlses->DZ->mat[3] =  rlses->past_values[U1];
#endif

	//epsilon
	matTranspose(phiT,rlses->phi);
	matMul(temp11,phiT,rlses->th);
	eps = y - temp11->mat[0];

	condition = ((eps>20.0)||(eps<-20.0))&&condition;
	if (condition)
	{
		rlses->condition = RLSE_MINIMAL_UPDATES;
	}


	if (rlses->condition > 0)
	{
		rlses->condition--;


#if RLSE_IMV_TYPE == 1
		// IMV method

		//K
		matMul3(temp11,phiT,rlses->P,rlses->phi);
		temp11->mat[0] += RLSE_EXP_FORGETING_COEF; //(phi'*P*phi)
		matMul(rlses->K,rlses->P,rlses->DZ);
		matDivConst(rlses->K,temp11->mat[0]);

		//P
		matMul3(temp_nn_2,rlses->K,phiT,rlses->P);
		matSubtract(rlses->P,temp_nn_2);
		matDivConst(rlses->P,RLSE_EXP_FORGETING_COEF);

		//theta
		matScale(rlses->K,eps);
		matAdd(rlses->th, rlses->K);

#else
		//exponential forgetting
		//K
		matMul3(temp11,phiT,rlses->P,rlses->phi);
		temp11->mat[0] += RLSE_EXP_FORGETING_COEF; //(phi'*P*phi)
		matMul(rlses->K,rlses->P,rlses->phi);
		matDivConst(rlses->K,temp11->mat[0]);

		//P
		matMul3(temp_nn_2,rlses->K,phiT,rlses->P);
		matSubtract(rlses->P,temp_nn_2);
		matDivConst(rlses->P,RLSE_EXP_FORGETING_COEF);

		//theta
		matScale(rlses->K,eps);
		matAdd(rlses->th, rlses->K);
#endif
	}

	// update minul�ch vstup�
	rlses->past_values[Y4] = rlses->past_values[Y3];
	rlses->past_values[Y3] = rlses->past_values[Y2];
	rlses->past_values[Y2] = rlses->past_values[Y1];
	rlses->past_values[Y1] = y;
	rlses->past_values[U1] = u;
}

#define REG_PERIOD (0.02)

void compute_params( matrixType *th, regParamType *reg)
{
	float KP1, KP2,b, c, d, alfa, omega, Tk;

	KP1	= (1-th->mat[1])/th->mat[3];
	KP2 = (th->mat[0] - th->mat[1] - 1)/(th->mat[3]- th->mat[2]);
	b = (th->mat[2]*KP1) + th->mat[0];
	c = (th->mat[3]*KP1) + th->mat[1];
	d = (b*b) - 4*c;
	alfa = -b/2;
	omega = (acosf(alfa)/REG_PERIOD);
	Tk = 2.0*M_PI/omega;

	if (d>0)
	{
		KP1 = KP2;
		Tk  = (2*REG_PERIOD);
	}


	reg->Kkrit = KP1;
	reg->Tkrit = Tk;


	//Z-N n�vrh
	reg->Kr = 0.3* KP1;
	reg->Ti = REG_PERIOD/(Tk);
	reg->Td = (0.125/REG_PERIOD * Tk);

	/*//Z-N n�vrh
	reg->Kr = 0.6* KP1;
	reg->Ti = REG_PERIOD/(0.5*Tk);
	reg->Td = (0.125/REG_PERIOD * Tk);*/
}
