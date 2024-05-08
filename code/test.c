#include <stdio.h>
#include <criterion/criterion.h>
#include "symdiff.h"

Test(symdiff, node) {
	SymbolNodeArray* array = SymbolNodeArrayCreate();

	SymbolNode* variable = SymbolNodeVariable(array); // v
	SymbolNode* t1 = SymbolNodeBinary(array, MULTIPLY, variable, variable); // v**2
	SymbolNode* expression = SymbolNodeBinary(array, ADD, t1, SymbolNodeConstant(array, 10)); // v**2 + 10

	const float value = SymbolNodeEvaluate(expression, array, variable, 100)->data.value; // (100)**2 + 10
	cr_expect_float_eq(10010, value, 0.0001);

	SymbolNode* derivate = SymbolNodeDifferentiate(expression, array, variable);
	const float valueD = SymbolNodeEvaluate(derivate, array, variable, 100)->data.value;
	cr_expect_float_eq(200, valueD, 0.0001);

	SymbolNode* derivate2 = SymbolNodeDifferentiate(derivate, array, variable);
	const float valueD2 = SymbolNodeEvaluate(derivate2, array, variable, 100)->data.value;
	cr_expect_float_eq(2, valueD2, 0.0001);

	SymbolNodeArrayFree(array);
}

Test(symdiff, matrix) {
	SymbolMatrixArray* arraySymbolMatrix = SymbolMatrixArrayCreate();

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

	const float value = SymbolNodeEvaluate(d->values[0], arraySymbolMatrix->nodeArray, a->values[0], 100)->data.value;
	cr_expect_float_eq(10010, value, 0.0001);

	SymbolMatrix* derivate = SymbolMatrixDifferentiateSymbolNode(d, arraySymbolMatrix, a->values[0]);
	const float valueD = SymbolNodeEvaluate(derivate->values[0], arraySymbolMatrix->nodeArray, a->values[0], 100)->data.value;
	cr_expect_float_eq(200, valueD, 0.0001);

	SymbolMatrix* derivate2 = SymbolMatrixDifferentiateSymbolNode(derivate, arraySymbolMatrix, a->values[0]);
	const float valueD2 = SymbolNodeEvaluate(derivate2->values[0], arraySymbolMatrix->nodeArray, a->values[0], 100)->data.value;
	cr_expect_float_eq(2, valueD2, 0.0001);

	SymbolMatrixArrayFree(arraySymbolMatrix);
}
