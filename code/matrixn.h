#ifndef SIMULATOR_MATRIXN_H
#define SIMULATOR_MATRIXN_H

#include <stdlib.h>

//-----------------------------------------------------------------------------
// MatrixN
//-----------------------------------------------------------------------------

typedef struct MatrixN {
	unsigned int rows;
	unsigned int cols;
	float * values;
} MatrixN;

typedef struct MatrixNArray {
	MatrixN **start;
	size_t capacity;
	size_t size;
} MatrixNArray;

MatrixNArray* MatrixNArrayCreate();

void MatrixNArrayFree(MatrixNArray* array);

void MatrixNArrayPrint(MatrixNArray* array);

MatrixN* MatrixNCreate(MatrixNArray* array, unsigned int rows, unsigned int cols);

void MatrixNFree(MatrixN* matrix);

float* MatrixNGet(MatrixN * matrix, unsigned int row, unsigned int col);

void MatrixNPrint(MatrixN* array);

void MatrixNReshape(MatrixN * matrix, unsigned int rows, unsigned int cols);

float MatrixNNorm(MatrixN * matrix);

MatrixN* MatrixNTranspose(MatrixNArray* array, MatrixN * matrix);

MatrixN* MatrixNNegate(MatrixNArray* array, MatrixN * matrix);

MatrixN* MatrixNAdd(MatrixNArray* array, MatrixN * a,  MatrixN * b);

MatrixN* MatrixNMultiply(MatrixNArray* array, MatrixN * a,  MatrixN * b);

MatrixN* MatrixNMultiplyValue(MatrixNArray* array, MatrixN * matrix, float value);

MatrixN* MatrixNInverse(MatrixNArray* array, MatrixN * matrix);

MatrixN* MatrixNPseudoinverse(MatrixNArray* array, MatrixN * matrix);

#endif //SIMULATOR_MATRIXN_H
