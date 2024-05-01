#include <stdio.h>
#include "symdiff.h"


void testSymdiff() {
	{
		SymbolNodeArray* array = SymbolNodeArrayCreate();

		SymbolNode* variable = SymbolNodeVariable(array);
		SymbolNode* expression = SymbolNodeBinary(array, ADD, SymbolNodeBinary(array, MULTIPLY, variable, variable),
		                                          SymbolNodeConstant(array, 10));

		const float value = SymbolNodeEvaluate(expression, array, variable, 100)->data.value;

		SymbolNodePrint(expression);

		printf("R %f\n", value);

		SymbolNode* derivate = SymbolNodeDifferentiate(expression, array, variable);

		const float valueD = SymbolNodeEvaluate(derivate, array, variable, 100)->data.value;

		SymbolNodePrint(derivate);
		printf("D %f\n", valueD);

		SymbolNode* derivate2 = SymbolNodeDifferentiate(derivate, array, variable);

		const float valueD2 = SymbolNodeEvaluate(derivate2, array, variable, 100)->data.value;

		SymbolNodePrint(derivate2);
		printf("D %f\n", valueD2);

		SymbolNodeArrayFree(array);
	}

	{
		SymbolNodeArray* array = SymbolNodeArrayCreate();
		SymbolMatrixArray* arraySymbolMatrix = SymbolMatrixArrayCreate(array);

		SymbolMatrix* a = SymbolMatrixCreate(arraySymbolMatrix, 2, 2);                 // a
		SymbolMatrixSetNode(a, 0, 0, SymbolNodeVariable(array));
		SymbolMatrixSetNode(a, 1, 0, SymbolNodeVariable(array));
		SymbolMatrixSetNode(a, 0, 1, SymbolNodeVariable(array));
		SymbolMatrixSetNode(a, 1, 1, SymbolNodeVariable(array));
		SymbolMatrix* b = SymbolMatrixMultiplyElementWise(arraySymbolMatrix, a, a);    // a * a
		SymbolMatrix* c = SymbolMatrixCreate(arraySymbolMatrix, 2, 2);                 // 10
		SymbolMatrixSetNode(c, 0, 0, SymbolNodeConstant(array, 10));
		SymbolMatrixSetNode(c, 1, 0, SymbolNodeConstant(array, 10));
		SymbolMatrixSetNode(c, 0, 1, SymbolNodeConstant(array, 10));
		SymbolMatrixSetNode(c, 1, 1, SymbolNodeConstant(array, 10));
		SymbolMatrix* d = SymbolMatrixAdd(arraySymbolMatrix, b, c);                    // a * a + 10

		SymbolMatrixPrint(d);

		const float value = SymbolNodeEvaluate(d->values[0], array, a->values[0], 100)->data.value;

		printf("R %f\n", value);

		SymbolMatrix* derivate = SymbolMatrixDifferentiateSymbolNode(d, arraySymbolMatrix, a->values[0]);

		const float valueD = SymbolNodeEvaluate(derivate->values[0], array, a->values[0], 100)->data.value;

		SymbolMatrixPrint(derivate);
		printf("D %f\n", valueD);

		SymbolMatrix* derivate2 = SymbolMatrixDifferentiateSymbolNode(derivate, arraySymbolMatrix, a->values[0]);

		const float valueD2 = SymbolNodeEvaluate(derivate2->values[0], array, a->values[0], 100)->data.value;

		SymbolMatrixPrint(derivate2);
		printf("D %f\n", valueD2);

		SymbolNodeArrayFree(array);
		SymbolMatrixArrayFree(arraySymbolMatrix);
	}
}

int main() {
	testSymdiff();
}
