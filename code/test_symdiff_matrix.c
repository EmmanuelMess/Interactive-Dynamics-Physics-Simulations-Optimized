#include <criterion/criterion.h>
#include <criterion/new/assert.h>

#include "symdiff.h"

static SymbolMatrixArray* arraySymbolMatrix;

static void setup() {
	arraySymbolMatrix = SymbolMatrixArrayCreate();
}

static void teardown() {
	SymbolMatrixArrayFree(arraySymbolMatrix);
}

Test(symdiff_matrix, general, .init = setup, .fini = teardown) {
	SymbolMatrix* a = SymbolMatrixCreate(arraySymbolMatrix, 2, 2);                 // a
	SymbolMatrixSet(a, 0, 0, SymbolNodeVariable(arraySymbolMatrix->nodeArray));
	SymbolMatrixSet(a, 1, 0, SymbolNodeVariable(arraySymbolMatrix->nodeArray));
	SymbolMatrixSet(a, 0, 1, SymbolNodeVariable(arraySymbolMatrix->nodeArray));
	SymbolMatrixSet(a, 1, 1, SymbolNodeVariable(arraySymbolMatrix->nodeArray));
	SymbolMatrix* b = SymbolMatrixMultiplyElementWise(arraySymbolMatrix, a, a);    // a * a
	SymbolMatrix* c = SymbolMatrixCreate(arraySymbolMatrix, 2, 2);                 // 10
	SymbolMatrixSet(c, 0, 0, SymbolNodeConstant(arraySymbolMatrix->nodeArray, 10));
	SymbolMatrixSet(c, 1, 0, SymbolNodeConstant(arraySymbolMatrix->nodeArray, 10));
	SymbolMatrixSet(c, 0, 1, SymbolNodeConstant(arraySymbolMatrix->nodeArray, 10));
	SymbolMatrixSet(c, 1, 1, SymbolNodeConstant(arraySymbolMatrix->nodeArray, 10));
	SymbolMatrix* d = SymbolMatrixAdd(arraySymbolMatrix, b, c);                    // a * a + 10

	for(unsigned int i = 0; i < d->rows * d->cols; i++) {
		const float value = SymbolNodeEvaluate(d->values[i], arraySymbolMatrix->nodeArray, a->values[i],
		                                       100)->data.value;
		cr_assert(ieee_ulp_eq(flt, 10010, value, 4));


		SymbolMatrix *derivate = SymbolMatrixDifferentiateSymbolNode(d, arraySymbolMatrix, a->values[i]);
		const float valueD = SymbolNodeEvaluate(derivate->values[i], arraySymbolMatrix->nodeArray, a->values[i],
		                                        100)->data.value;
		cr_assert(ieee_ulp_eq(flt, 200, valueD, 4));

		SymbolMatrix *derivate2 = SymbolMatrixDifferentiateSymbolNode(derivate, arraySymbolMatrix, a->values[i]);
		const float valueD2 = SymbolNodeEvaluate(derivate2->values[i], arraySymbolMatrix->nodeArray, a->values[i],
		                                         100)->data.value;
		cr_assert(ieee_ulp_eq(flt, 2, valueD2, 4));
	}
}
