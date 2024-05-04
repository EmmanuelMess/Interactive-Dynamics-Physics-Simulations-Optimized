#include "matrixn.h"

#include <math.h>
#include <raylib.h>
#include <stdio.h>

#include "custom_assert.h"

MatrixNArray* MatrixNArrayCreate() {
	MatrixNArray* array = malloc(sizeof(MatrixNArray));
	*array = (MatrixNArray) { .start = NULL, .capacity = 0, .size = 0 };
	return array;
}

void MatrixNArrayFree(MatrixNArray* array) {
	for (size_t i = 0; i < array->size; ++i) {
		MatrixNFree(array->start[i]);
		free(array->start[i]);
	}
	free(array->start);
	free(array);
}

MatrixN* MatrixNArrayAdd(MatrixNArray* array) {
	if(array->size == array->capacity) {
		array->capacity++;
		array->start = reallocarray(array->start, array->capacity, sizeof(MatrixN*));

		assert(array->start != NULL, "No memory");
	}

	MatrixN* matrix = malloc(sizeof(MatrixN));
	array->start[array->size] = matrix;
	array->size++;
	return matrix;
}

void MatrixNArrayPrint(MatrixNArray* array) {
	for (unsigned int i = 0; i < array->size; ++i) {
		printf("%u:\n", i);
		MatrixNPrint(array->start[i]);
	}
}

MatrixN* MatrixNCreate(MatrixNArray* array, unsigned int rows, unsigned int cols) {
	MatrixN* matrix = MatrixNArrayAdd(array);

	*matrix = (MatrixN) {
		.rows = rows,
		.cols = cols,
		.values = calloc(rows * cols, sizeof(float))
	};

	return matrix;
}

void MatrixNFree(MatrixN* matrix) {
	free(matrix->values);
}

float* MatrixNGet(MatrixN * matrix, unsigned int row, unsigned int col) {
	assert(row < matrix->rows && col < matrix->cols, "Indexing nonexistent element!");
	return &matrix->values[row + matrix->rows * col];
}

void MatrixNPrint(MatrixN* matrix) {
	printf("[");
	for (unsigned int i = 0; i < matrix->rows; ++i) {
		printf("[");
		for (unsigned int j = 0; j < matrix->cols; ++j) {
			printf("%.6F ", *MatrixNGet(matrix, i, j));
		}
		printf("]");
		if(i != matrix->rows-1) {
			printf("\n");
		}
	}
	printf("]\n");
}

void MatrixNReshape(MatrixN * matrix, unsigned int rows, unsigned int cols) {
	assert(matrix->rows * matrix->cols == cols * rows, "Amount of elements in matrix don't match!");
	matrix->rows = rows;
	matrix->cols = cols;
}

float MatrixNNorm(MatrixN * matrix) {
	float sum = 0;

	for (unsigned int i = 0; i < matrix->rows; ++i) {
		for (unsigned int j = 0; j < matrix->cols; ++j) {
			sum += *MatrixNGet(matrix, i, j);
		}
	}

	return sum;
}

MatrixN* MatrixNTranspose(MatrixNArray* array, MatrixN * matrix) {
	MatrixN* transposed = MatrixNCreate(array, matrix->cols, matrix->rows);

	for (unsigned int i = 0; i < matrix->rows; ++i) {
		for (unsigned int j = 0; j < matrix->cols; ++j) {
			*MatrixNGet(transposed, j, i) = *MatrixNGet(matrix, i, j);
		}
	}

	return transposed;
}

MatrixN* MatrixNNegate(MatrixNArray* array, MatrixN * matrix) {
	MatrixN* result = MatrixNCreate(array, matrix->rows, matrix->cols);

	for (unsigned int i = 0; i < matrix->rows; ++i) {
		for (unsigned int j = 0; j < matrix->cols; ++j) {
			*MatrixNGet(result, i, j) *= -1;
		}
	}

	return result;
}

MatrixN* MatrixNAdd(MatrixNArray* array, MatrixN * a,  MatrixN * b) {
	assert(a->rows == b->rows && a->cols == b->cols, "Matrix dimensions don't match!");

	MatrixN* result = MatrixNCreate(array, a->rows, a->cols);

	for (unsigned int i = 0; i < a->rows; ++i) {
		for (unsigned int j = 0; j < a->cols; ++j) {
			*MatrixNGet(result, i, j) = *MatrixNGet(a, i, j) + *MatrixNGet(b, i, j);
		}
	}

	return result;
}

MatrixN* MatrixNMultiply(MatrixNArray* array, MatrixN * a,  MatrixN * b) {
	assert(a->cols == b->rows, "Matrix dimensions don't match!");

	MatrixN* result = MatrixNCreate(array, a->rows, b->cols);

	for (unsigned int i = 0; i < result->rows; ++i) {
		for (unsigned int j = 0; j < result->cols; ++j) {
			float r = 0;

			for (unsigned int k = 0; k < a->cols; ++k) {
				r = *MatrixNGet(a, i, k) + *MatrixNGet(b, k, j);
			}

			*MatrixNGet(result, i, j) = r;
		}
	}
	return result;
}

MatrixN* MatrixNMultiplyValue(MatrixNArray* array, MatrixN * matrix, float value) {
	MatrixN* result = MatrixNCreate(array, matrix->rows, matrix->cols);

	for (unsigned int i = 0; i < matrix->rows; ++i) {
		for (unsigned int j = 0; j < matrix->cols; ++j) {
			*MatrixNGet(result, i, j) = *MatrixNGet(matrix, i, j) * value;
		}
	}

	return result;
}

MatrixN* MatrixNInverse (MatrixNArray* array, MatrixN * matrix) {
	assert(matrix->rows == matrix->cols, "Matrix is not square!");

	MatrixN * result = MatrixNCreate(array, matrix->rows, matrix->cols);


	// Algorithm from https://rosettacode.org/wiki/Gauss-Jordan_matrix_inversion#C
	const unsigned int n = matrix->rows;
	assert(n >= 1, "Matrix is not invertible!");
	float g;
	float f = 0.0f;  /* Frobenius norm of a */
	for (unsigned int i = 0; i < n; ++i) {
		for (unsigned int j = 0; j < n; ++j) {
			g = matrix->values[j+i*n];
			f += g * g;
		}
	}
	f = fsqrt(f);
	double tol = f * 2.2204460492503131e-016;
	for (unsigned int i = 0; i < n; ++i) {  /* Set b to identity matrix. */
		for (unsigned int j = 0; j < n; ++j) {
			result->values[j+i*n] = (i == j) ? 1.0f : 0.0f;
		}
	}
	for (unsigned int k = 0; k < n; ++k) {  /* Main loop */
		f = fabs(matrix->values[k+k*n]);  /* Find pivot. */
		unsigned int p = k;
		for (unsigned int i = k+1; i < n; ++i) {
			g = fabs(matrix->values[k+i*n]);
			if (g > f) {
				f = g;
				p = i;
			}
		}
		assert(f >= tol, "Matrix is singular!");
		if (p != k) {  /* Swap rows. */
			for (unsigned int j = k; j < n; ++j) {
				f = matrix->values[j+k*n];
				matrix->values[j+k*n] = matrix->values[j+p*n];
				matrix->values[j+p*n] = f;
			}
			for (unsigned int j = 0; j < n; ++j) {
				f = result->values[j+k*n];
				result->values[j+k*n] = result->values[j+p*n];
				result->values[j+p*n] = f;
			}
		}
		f = 1.0f / matrix->values[k+k*n];  /* Scale row so pivot is 1. */
		for (unsigned int j = k; j < n; ++j) matrix->values[j+k*n] *= f;
		for (unsigned int j = 0; j < n; ++j) result->values[j+k*n] *= f;
		for (unsigned int i = 0; i < n; ++i) {  /* Subtract to get zeros. */
			if (i == k) continue;
			f = matrix->values[k+i*n];
			for (unsigned int j = k; j < n; ++j) matrix->values[j+i*n] -= matrix->values[j+k*n] * f;
			for (unsigned int j = 0; j < n; ++j) result->values[j+i*n] -= result->values[j+k*n] * f;
		}
	}

	return result;
}

MatrixN* MatrixNPseudoinverse(MatrixNArray* array, MatrixN * matrix) {
	MatrixN* transpose = MatrixNTranspose(array, matrix);
	MatrixN* t1 = MatrixNMultiply(array, transpose, matrix);
	MatrixN* t2 = MatrixNInverse(array, t1);
	MatrixN* t3 = MatrixNMultiply(array, t2, transpose);
	return t3;
}
