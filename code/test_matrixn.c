#include <criterion/criterion.h>
#include <criterion/new/assert.h>

#include "matrixn.h"

static MatrixNArray* arrayMatrixN;

static MatrixN * Generate(unsigned int rows, unsigned int cols) {
	MatrixN* matrix = MatrixNCreate(arrayMatrixN, rows, cols);

	for (unsigned int r = 0; r < rows; ++r) {
		for (unsigned int c = 0; c < cols; ++c) {
			*MatrixNGet(matrix, r, c) = 47.0f * r * c;
		}
	}

	return matrix;
}

static void setup() {
	arrayMatrixN = MatrixNArrayCreate();
}

static void teardown() {
	MatrixNArrayFree(arrayMatrixN);
}

Test(matrixn, transpose_inv_1, .init = setup, .fini = teardown) {
	MatrixN* matrix = Generate(4, 1);
	MatrixN* transpose = MatrixNTranspose(arrayMatrixN, MatrixNTranspose(arrayMatrixN, matrix));

	for (unsigned int i = 0; i < matrix->rows; ++i) {
		for (unsigned int j = 0; j < matrix->cols; ++j) {
			cr_assert(ieee_ulp_eq(flt, *MatrixNGet(matrix, i, j), *MatrixNGet(transpose, i, j), 4));
		}
	}
}

Test(matrixn, transpose_inv_2, .init = setup, .fini = teardown) {
	MatrixN* matrix = Generate(4, 3);
	MatrixN* transpose = MatrixNTranspose(arrayMatrixN, MatrixNTranspose(arrayMatrixN, matrix));

	for (unsigned int i = 0; i < matrix->rows; ++i) {
		for (unsigned int j = 0; j < matrix->cols; ++j) {
			cr_assert(ieee_ulp_eq(flt, *MatrixNGet(matrix, i, j), *MatrixNGet(transpose, i, j), 4));
		}
	}
}

Test(matrixn, negate_inv_1, .init = setup, .fini = teardown) {
	MatrixN* matrix = Generate(4, 1);
	MatrixN* negated = MatrixNNegate(arrayMatrixN, MatrixNNegate(arrayMatrixN, matrix));

	for (unsigned int i = 0; i < matrix->rows; ++i) {
		for (unsigned int j = 0; j < matrix->cols; ++j) {
			cr_assert(ieee_ulp_eq(flt, *MatrixNGet(matrix, i, j), *MatrixNGet(negated, i, j), 4));
		}
	}
}

Test(matrixn, negate_inv_2, .init = setup, .fini = teardown) {
	MatrixN* matrix = Generate(15, 10);
	MatrixN* negated = MatrixNNegate(arrayMatrixN, MatrixNNegate(arrayMatrixN, matrix));

	for (unsigned int i = 0; i < matrix->rows; ++i) {
		for (unsigned int j = 0; j < matrix->cols; ++j) {
			cr_assert(ieee_ulp_eq(flt, *MatrixNGet(matrix, i, j), *MatrixNGet(negated, i, j), 4));
		}
	}
}

Test(matrixn, add_inv_1, .init = setup, .fini = teardown) {
	MatrixN* matrix = Generate(4, 1);
	MatrixN* zero = MatrixNAdd(arrayMatrixN, matrix, MatrixNNegate(arrayMatrixN, matrix));

	for (unsigned int i = 0; i < matrix->rows; ++i) {
		for (unsigned int j = 0; j < matrix->cols; ++j) {
			cr_assert(ieee_ulp_eq(flt, *MatrixNGet(zero, i, j), 0.0f, 4));
		}
	}
}

Test(matrixn, add_inv_2, .init = setup, .fini = teardown) {
	MatrixN* matrix = Generate(15, 10);
	MatrixN* zero = MatrixNAdd(arrayMatrixN, matrix, MatrixNNegate(arrayMatrixN, matrix));

	for (unsigned int i = 0; i < matrix->rows; ++i) {
		for (unsigned int j = 0; j < matrix->cols; ++j) {
			cr_assert(ieee_ulp_eq(flt, *MatrixNGet(zero, i, j), 0.0f, 4));
		}
	}
}

Test(matrixn, inverse_1, .init = setup, .fini = teardown) {
	MatrixN* matrix = MatrixNCreate(arrayMatrixN, 4, 4);
	*MatrixNGet(matrix, 0, 0) = -1; *MatrixNGet(matrix, 0, 1) = -2; *MatrixNGet(matrix, 0, 2) = 3; *MatrixNGet(matrix, 0, 3) = 2;
	*MatrixNGet(matrix, 1, 0) = -4; *MatrixNGet(matrix, 1, 1) = -1; *MatrixNGet(matrix, 1, 2) = 6; *MatrixNGet(matrix, 1, 3) = 2;
	*MatrixNGet(matrix, 2, 0) =  7; *MatrixNGet(matrix, 2, 1) = -8; *MatrixNGet(matrix, 2, 2) = 9; *MatrixNGet(matrix, 2, 3) = 1;
	*MatrixNGet(matrix, 3, 0) =  1; *MatrixNGet(matrix, 3, 1) = -2; *MatrixNGet(matrix, 3, 2) = 1; *MatrixNGet(matrix, 3, 3) = 3;

	MatrixN* inverse = MatrixNInverse(arrayMatrixN, matrix);

	MatrixN* realInverse = MatrixNCreate(arrayMatrixN, 4, 4);
	*MatrixNGet(realInverse, 0, 0) = -21.0f/23.0f; *MatrixNGet(realInverse, 0, 1) = 17.0f/69.0f; *MatrixNGet(realInverse, 0, 2) = 13.0f/138.0f; *MatrixNGet(realInverse, 0, 3) = 19.0f/46.0f;
	*MatrixNGet(realInverse, 1, 0) = -38.0f/23.0f; *MatrixNGet(realInverse, 1, 1) = 15.0f/23.0f; *MatrixNGet(realInverse, 1, 2) =   1.0f/23.0f; *MatrixNGet(realInverse, 1, 3) = 15.0f/23.0f;
	*MatrixNGet(realInverse, 2, 0) = -16.0f/23.0f; *MatrixNGet(realInverse, 2, 1) = 25.0f/69.0f; *MatrixNGet(realInverse, 2, 2) = 11.0f/138.0f; *MatrixNGet(realInverse, 2, 3) =  9.0f/46.0f;
	*MatrixNGet(realInverse, 3, 0) = -13.0f/23.0f; *MatrixNGet(realInverse, 3, 1) = 16.0f/69.0f; *MatrixNGet(realInverse, 3, 2) =  -2.0f/69.0f; *MatrixNGet(realInverse, 3, 3) = 13.0f/23.0f;

	for (unsigned int i = 0; i < matrix->rows; ++i) {
		for (unsigned int j = 0; j < matrix->cols; ++j) {
			cr_assert(ieee_ulp_eq(flt, *MatrixNGet(inverse, i, j), *MatrixNGet(realInverse, i, j), 4));
		}
	}
}

Test(matrixn, inverse_2, .init = setup, .fini = teardown) {
	MatrixN* matrix = MatrixNCreate(arrayMatrixN, 1, 1);
	*MatrixNGet(matrix, 0, 0) = 2;
	MatrixN* inverse = MatrixNInverse(arrayMatrixN, matrix);

	cr_assert(ieee_ulp_eq(flt, *MatrixNGet(inverse, 0, 0), 0.5f, 4));
}

Test(matrixn, inverse_3, .init = setup, .fini = teardown) {
	MatrixN* matrix = MatrixNCreate(arrayMatrixN, 3, 3);
	*MatrixNGet(matrix, 0, 0) =  0; *MatrixNGet(matrix, 0, 1) = -3; *MatrixNGet(matrix, 0, 2) = -2;
	*MatrixNGet(matrix, 1, 0) =  1; *MatrixNGet(matrix, 1, 1) = -4; *MatrixNGet(matrix, 1, 2) = -2;
	*MatrixNGet(matrix, 2, 0) = -3; *MatrixNGet(matrix, 2, 1) =  4; *MatrixNGet(matrix, 2, 2) =  1;
	MatrixN* inverse = MatrixNInverse(arrayMatrixN, matrix);

	MatrixN* realInverse = MatrixNCreate(arrayMatrixN, 3, 3);
	*MatrixNGet(realInverse, 0, 0) =  4; *MatrixNGet(realInverse, 0, 1) = -5; *MatrixNGet(realInverse, 0, 2) = -2;
	*MatrixNGet(realInverse, 1, 0) =  5; *MatrixNGet(realInverse, 1, 1) = -6; *MatrixNGet(realInverse, 1, 2) = -2;
	*MatrixNGet(realInverse, 2, 0) = -8; *MatrixNGet(realInverse, 2, 1) =  9; *MatrixNGet(realInverse, 2, 2) =  3;

	for (unsigned int i = 0; i < matrix->rows; ++i) {
		for (unsigned int j = 0; j < matrix->cols; ++j) {
			cr_assert(ieee_ulp_eq(flt, *MatrixNGet(inverse, i, j), *MatrixNGet(realInverse, i, j), 4));
		}
	}
}

Test(matrixn, inverse_inv_1, .init = setup, .fini = teardown) {
	MatrixN* matrix = MatrixNCreate(arrayMatrixN, 1, 1);
	*MatrixNGet(matrix, 0, 0) = 2;
	MatrixN* inversed = MatrixNInverse(arrayMatrixN, MatrixNInverse(arrayMatrixN, matrix));

	for (unsigned int i = 0; i < matrix->rows; ++i) {
		for (unsigned int j = 0; j < matrix->cols; ++j) {
			cr_assert(ieee_ulp_eq(flt, *MatrixNGet(matrix, i, j), *MatrixNGet(inversed, i, j), 4));
		}
	}
}

Test(matrixn, inverse_inv_2, .init = setup, .fini = teardown) {
	MatrixN* matrix = MatrixNCreate(arrayMatrixN, 3, 3);
	*MatrixNGet(matrix, 0, 0) = 0; *MatrixNGet(matrix, 0, 1) = -3; *MatrixNGet(matrix, 0, 2) = -2;
	*MatrixNGet(matrix, 1, 0) = 1; *MatrixNGet(matrix, 1, 1) = -4; *MatrixNGet(matrix, 1, 2) = -2;
	*MatrixNGet(matrix, 2, 0) = -3; *MatrixNGet(matrix, 2, 1) = 4; *MatrixNGet(matrix, 2, 2) = 1;
	MatrixN* inversed = MatrixNInverse(arrayMatrixN, MatrixNInverse(arrayMatrixN, matrix));

	for (unsigned int i = 0; i < matrix->rows; ++i) {
		for (unsigned int j = 0; j < matrix->cols; ++j) {
			cr_assert(ieee_ulp_eq(flt, *MatrixNGet(matrix, i, j), *MatrixNGet(inversed, i, j), 4));
		}
	}
}

Test(matrixn, pseudoinverse_1, .init = setup, .fini = teardown) {
	MatrixN* matrix = MatrixNCreate(arrayMatrixN, 4, 4);
	*MatrixNGet(matrix, 0, 0) = -1; *MatrixNGet(matrix, 0, 1) = -2; *MatrixNGet(matrix, 0, 2) = 3; *MatrixNGet(matrix, 0, 3) = 2;
	*MatrixNGet(matrix, 1, 0) = -4; *MatrixNGet(matrix, 1, 1) = -1; *MatrixNGet(matrix, 1, 2) = 6; *MatrixNGet(matrix, 1, 3) = 2;
	*MatrixNGet(matrix, 2, 0) =  7; *MatrixNGet(matrix, 2, 1) = -8; *MatrixNGet(matrix, 2, 2) = 9; *MatrixNGet(matrix, 2, 3) = 1;
	*MatrixNGet(matrix, 3, 0) =  1; *MatrixNGet(matrix, 3, 1) = -2; *MatrixNGet(matrix, 3, 2) = 1; *MatrixNGet(matrix, 3, 3) = 3;

	MatrixN* inverse = MatrixNPseudoinverse(arrayMatrixN, matrix);

	MatrixN* realInverse = MatrixNCreate(arrayMatrixN, 4, 4);
	*MatrixNGet(realInverse, 0, 0) = -21.0f/23.0f; *MatrixNGet(realInverse, 0, 1) = 17.0f/69.0f; *MatrixNGet(realInverse, 0, 2) = 13.0f/138.0f; *MatrixNGet(realInverse, 0, 3) = 19.0f/46.0f;
	*MatrixNGet(realInverse, 1, 0) = -38.0f/23.0f; *MatrixNGet(realInverse, 1, 1) = 15.0f/23.0f; *MatrixNGet(realInverse, 1, 2) =   1.0f/23.0f; *MatrixNGet(realInverse, 1, 3) = 15.0f/23.0f;
	*MatrixNGet(realInverse, 2, 0) = -16.0f/23.0f; *MatrixNGet(realInverse, 2, 1) = 25.0f/69.0f; *MatrixNGet(realInverse, 2, 2) = 11.0f/138.0f; *MatrixNGet(realInverse, 2, 3) =  9.0f/46.0f;
	*MatrixNGet(realInverse, 3, 0) = -13.0f/23.0f; *MatrixNGet(realInverse, 3, 1) = 16.0f/69.0f; *MatrixNGet(realInverse, 3, 2) =  -2.0f/69.0f; *MatrixNGet(realInverse, 3, 3) = 13.0f/23.0f;

	for (unsigned int i = 0; i < matrix->rows; ++i) {
		for (unsigned int j = 0; j < matrix->cols; ++j) {
			cr_assert(ieee_ulp_eq(flt, *MatrixNGet(inverse, i, j), *MatrixNGet(realInverse, i, j), 4));
		}
	}
}

Test(matrixn, pseudoinverse_2, .init = setup, .fini = teardown) {
	MatrixN* matrix = MatrixNCreate(arrayMatrixN, 1, 1);
	*MatrixNGet(matrix, 0, 0) = 2;
	MatrixN* inverse = MatrixNPseudoinverse(arrayMatrixN, matrix);

	cr_assert(ieee_ulp_eq(flt, *MatrixNGet(inverse, 0, 0), 0.5f, 4));
}

Test(matrixn, pseudoinverse_3, .init = setup, .fini = teardown) {
	MatrixN* matrix = MatrixNCreate(arrayMatrixN, 3, 3);
	*MatrixNGet(matrix, 0, 0) =  0; *MatrixNGet(matrix, 0, 1) = -3; *MatrixNGet(matrix, 0, 2) = -2;
	*MatrixNGet(matrix, 1, 0) =  1; *MatrixNGet(matrix, 1, 1) = -4; *MatrixNGet(matrix, 1, 2) = -2;
	*MatrixNGet(matrix, 2, 0) = -3; *MatrixNGet(matrix, 2, 1) =  4; *MatrixNGet(matrix, 2, 2) =  1;
	MatrixN* inverse = MatrixNPseudoinverse(arrayMatrixN, matrix);

	MatrixN* realInverse = MatrixNCreate(arrayMatrixN, 3, 3);
	*MatrixNGet(realInverse, 0, 0) =  4; *MatrixNGet(realInverse, 0, 1) = -5; *MatrixNGet(realInverse, 0, 2) = -2;
	*MatrixNGet(realInverse, 1, 0) =  5; *MatrixNGet(realInverse, 1, 1) = -6; *MatrixNGet(realInverse, 1, 2) = -2;
	*MatrixNGet(realInverse, 2, 0) = -8; *MatrixNGet(realInverse, 2, 1) =  9; *MatrixNGet(realInverse, 2, 2) =  3;

	for (unsigned int i = 0; i < matrix->rows; ++i) {
		for (unsigned int j = 0; j < matrix->cols; ++j) {
			cr_assert(ieee_ulp_eq(flt, *MatrixNGet(inverse, i, j), *MatrixNGet(realInverse, i, j), 4));
		}
	}
}

Test(matrixn, pseudoinverse_inv_1, .init = setup, .fini = teardown) {
	MatrixN* matrix = MatrixNCreate(arrayMatrixN, 1, 1);
	*MatrixNGet(matrix, 0, 0) = 2;
	MatrixN* inversed = MatrixNPseudoinverse(arrayMatrixN, MatrixNPseudoinverse(arrayMatrixN, matrix));

	for (unsigned int i = 0; i < matrix->rows; ++i) {
		for (unsigned int j = 0; j < matrix->cols; ++j) {
			cr_assert(ieee_ulp_eq(flt, *MatrixNGet(matrix, i, j), *MatrixNGet(inversed, i, j), 4));
		}
	}
}

Test(matrixn, pseudoinverse_inv_2, .init = setup, .fini = teardown) {
	MatrixN* matrix = MatrixNCreate(arrayMatrixN, 3, 3);
	*MatrixNGet(matrix, 0, 0) = 0; *MatrixNGet(matrix, 0, 1) = -3; *MatrixNGet(matrix, 0, 2) = -2;
	*MatrixNGet(matrix, 1, 0) = 1; *MatrixNGet(matrix, 1, 1) = -4; *MatrixNGet(matrix, 1, 2) = -2;
	*MatrixNGet(matrix, 2, 0) = -3; *MatrixNGet(matrix, 2, 1) = 4; *MatrixNGet(matrix, 2, 2) = 1;
	MatrixN* inversed = MatrixNPseudoinverse(arrayMatrixN, MatrixNPseudoinverse(arrayMatrixN, matrix));

	for (unsigned int i = 0; i < matrix->rows; ++i) {
		for (unsigned int j = 0; j < matrix->cols; ++j) {
			cr_assert(ieee_ulp_eq(flt, *MatrixNGet(matrix, i, j), *MatrixNGet(inversed, i, j), 4));
		}
	}
}
