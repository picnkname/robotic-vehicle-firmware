/*!\file SimpleMatrix.h
 * \author Lukas Otava
 * \date 2013
 * \defgroup SimpleMatrix SimpleMatrix module
 *
 * This module contain function for basic matrix operations. Memory for matrixes is dynamically allocated.
 *
 * Cells are saved in linear array by rows.
 *
 *
 * * Example usage:
 *  \code{c}
	matrixType* a;
	matrixType* b;
	matrixType* c;

	a = matAlloc(2,3);
	b = matAlloc(3,2);
	c = matAlloc(2,2);

	matEye(a,2);
	matFill(b,3);

	matPrint(a);
	matPrint(b);
	matMul(c,a,b);
	matPrint(c);

	matPrint(a);
	CELL(a,0,0) = 10.0;
	CELL(a,1,2) = CELL(a,0,0);
	matPrint(a);

	matFree(a);
	matFree(b);
	matFree(c);
	\endcode
 *
 * */

#ifndef __SIMPLE_MATRIX_H__
#define __SIMPLE_MATRIX_H__

// for NULL definition
#include <stdlib.h>

//! \brief this is used to define precision and range of values processed by matrix operations (generally float/integer) \ingroup SimpleMatrix
typedef float  matrixValType;

//! \brief this is base processor type used to integer values (Cortex-M3 is long) \ingroup SimpleMatrix
typedef long matrixSizeType;

//! \brief defining this macro, matrix sizes will be checked in matrix multiplication operations \ingroup SimpleMatrix
#define MATRIX_CHECK_DIMENSIONS

//! \brief this parameter is used to create temporary vector in 3 matrix multiplication, this mechanism should decrease overall memory usage \ingroup SimpleMatrix
#define MATRIX_TEMPORARY_MAX_SIZE (4)

/*! \brief Matrix structure used by this module
 *  \ingroup SimpleMatrix */
typedef struct matrixTypeStruct
{
	//! pointer to linear array holding 2 dimensional matrix
	matrixValType * mat;
	//! row count
	matrixSizeType m;
	//! column count
	matrixSizeType n;
}
matrixType;

#ifndef FREERTOS

/*! \brief Test function for PC
 * \ingroup SimpleMatrix
 */
void matTest( void);
#endif


/*! \brief Printing function for PC testing
 * \ingroup SimpleMatrix
 * \param mt Pointer to the matrix to print
 */
inline void matPrint( matrixType *mt);


//! \brief This macro can be used for the matrix cell access
//! \ingroup SimpleMatrix
//! \param matrix Pointer to matrix
//! \param i The row matrix index
//! \param j The column matrix index
#define CELL(matrix,i,j)  (matrix->mat[(j)+((i)*matrix->n)])

/*! \brief This function allocate memory for one matrix with specified dimensions.
 *  \param row This parameter express count of matrix rows, must be >0.
 *  \param column This parameter express count of matrix columns, must be >0.
 *  \return This function returns pointer to new matrixType structure.
 *  \ingroup SimpleMatrix */
matrixType * matAlloc(matrixSizeType row, matrixSizeType column);

/*! \brief This function free allocated memory of specified matrix.
 * 	\param m This is pointer to matrixType structure of matrix that should be deleted.
 *  \ingroup SimpleMatrix */
void matFree(matrixType *m);

/*! \brief This function set all cells of matrix res to value of parameter value.
 *  \param res Pointer to matrix that will be filled.
 *  \param value Value to fill.
 *  \ingroup SimpleMatrix */
inline void matFill(matrixType *res, matrixValType value);

/*! \brief This function set cells on main diagonal to value, other cells will be zero.
 *  \param res Pointer to matrix that will be filled.
 *  \param value Value that will be one main diagonal.
 *  \ingroup SimpleMatrix */
inline void matEye(matrixType* res, matrixValType value);

/*! \brief This function add matrix to matrix.
 *  \param a Result matrix
 *  \param b Source matrix that will be added to matrix  a
 *  \return Returns 0 if everything is OK.
 *  \ingroup SimpleMatrix */
inline matrixSizeType matAdd(matrixType *a, matrixType *b);

/*! \brief This function subtract matrix from matrix.
 *  \param a Result matrix.
 *  \param b Source matrix that will be subtracted from matrix  a.
 *  \return Returns 0 if everything is OK.
 *  \ingroup SimpleMatrix */
inline matrixSizeType matSubtract(matrixType *a, matrixType *b);

/*! \brief Function providing 2 matrix product (multiplication)
 *  \param res Result matrix. ([res] = [a].[b])
 *  \param a Source matrix
 *  \param b Source matrix
 *  \return Returns 0 if everything is OK.
 *  \ingroup SimpleMatrix */
inline matrixSizeType matMul(matrixType *res, matrixType *a, matrixType *b);

/*! \brief Function providing 3 matrix product (multiplication)
 *  \param res Result matrix. ([res] = [a].[b].[c])
 *  \param a Source matrix
 *  \param b Source matrix
 *  \param c Source matrix
 *  \return Returns 0 if everything is OK.
 *  \ingroup SimpleMatrix */
inline matrixSizeType matMul3(matrixType *res, matrixType *a, matrixType *b, matrixType *c);

/*! \brief This function transpose matrix.
 *  \param res Resulting transposed matrix a.
 *  \param a Source matrix.
 *  \return Returns 0 if everything is OK.
 *  \ingroup SimpleMatrix */
inline matrixSizeType matTranspose(matrixType *res, matrixType *a);

/*! \brief This function multiply each cell by scale.
 *  \param a Scaled matrix.
 *  \param scale Multiplying constant.
 *  \ingroup SimpleMatrix */
inline void matScale(matrixType *a, matrixValType scale);

/*! \brief This function divide each cell by value.
 *  \param a Divided matrix.
 *  \param value Dividing constant.
 *  \ingroup SimpleMatrix */
inline matrixSizeType matDivConst(matrixType *a, matrixValType value);

/*! \brief This function add to each cell value value.
 *  \param a Matrix to modification.
 *  \param value Constant to add.
 *  \ingroup SimpleMatrix */
inline void matAddConst(matrixType *a, matrixValType value);

#endif//__SIMPLE_MATRIX_H__



