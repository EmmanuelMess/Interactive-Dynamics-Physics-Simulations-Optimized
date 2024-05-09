#include <criterion/criterion.h>
#include <criterion/new/assert.h>

#include "symdiff.h"

static SymbolNodeArray* symbolNodeArray;

static void setup() {
	symbolNodeArray = SymbolNodeArrayCreate();
}

static void teardown() {
	SymbolNodeArrayFree(symbolNodeArray);
}

Test(symdiff_node, constant, .init = setup, .fini = teardown) {
	SymbolNode* variable = SymbolNodeVariable(symbolNodeArray); // v
	SymbolNode* t1 = SymbolNodeConstant(symbolNodeArray, 10);
	SymbolNode* expression = SymbolNodeBinary(symbolNodeArray, ADD, variable, t1); // v + 10

	const float value = SymbolNodeEvaluate(expression, symbolNodeArray, variable, 100)->data.value;
	cr_assert(ieee_ulp_eq(flt, 110, value, 4));

	SymbolNode* derivate = SymbolNodeDifferentiate(expression, symbolNodeArray, variable);
	const float valueD = SymbolNodeEvaluate(derivate, symbolNodeArray, variable, 100)->data.value;
	cr_assert(ieee_ulp_eq(flt, 1, valueD, 4));

	SymbolNode* derivate2 = SymbolNodeDifferentiate(derivate, symbolNodeArray, variable);
	const float valueD2 = SymbolNodeEvaluate(derivate2, symbolNodeArray, variable, 100)->data.value;
	cr_assert(ieee_ulp_eq(flt, 0, valueD2, 4));
}

Test(symdiff_node, variable, .init = setup, .fini = teardown) {
	SymbolNode* variable = SymbolNodeVariable(symbolNodeArray); // v
	SymbolNode* expression = SymbolNodeBinary(symbolNodeArray, ADD, variable, variable); // v + v

	const float value = SymbolNodeEvaluate(expression, symbolNodeArray, variable, 100)->data.value;
	cr_assert(ieee_ulp_eq(flt, 200, value, 4));

	SymbolNode* derivate = SymbolNodeDifferentiate(expression, symbolNodeArray, variable);
	const float valueD = SymbolNodeEvaluate(derivate, symbolNodeArray, variable, 100)->data.value;
	cr_assert(ieee_ulp_eq(flt, 2, valueD, 4));

	SymbolNode* derivate2 = SymbolNodeDifferentiate(derivate, symbolNodeArray, variable);
	const float valueD2 = SymbolNodeEvaluate(derivate2, symbolNodeArray, variable, 100)->data.value;
	cr_assert(ieee_ulp_eq(flt, 0, valueD2, 4));
}

Test(symdiff_node, add, .init = setup, .fini = teardown) {
	SymbolNode* variable = SymbolNodeVariable(symbolNodeArray); // v
	SymbolNode* t1 = SymbolNodeBinary(symbolNodeArray, ADD, variable, variable); // v + v
	SymbolNode* t2 = SymbolNodeConstant(symbolNodeArray, 10); // 10
	SymbolNode* t3 = SymbolNodeConstant(symbolNodeArray, 20); // 20
	SymbolNode* t4 = SymbolNodeBinary(symbolNodeArray, ADD, t2, t3); // 10 + 20
	SymbolNode* expression = SymbolNodeBinary(symbolNodeArray, ADD, t1, t4); // v + v + 10 + 20

	const float value = SymbolNodeEvaluate(expression, symbolNodeArray, variable, 100)->data.value;
	cr_assert(ieee_ulp_eq(flt, 230, value, 4));

	SymbolNode* derivate = SymbolNodeDifferentiate(expression, symbolNodeArray, variable);
	const float valueD = SymbolNodeEvaluate(derivate, symbolNodeArray, variable, 100)->data.value;
	cr_assert(ieee_ulp_eq(flt, 2, valueD, 4));

	SymbolNode* derivate2 = SymbolNodeDifferentiate(derivate, symbolNodeArray, variable);
	const float valueD2 = SymbolNodeEvaluate(derivate2, symbolNodeArray, variable, 100)->data.value;
	cr_assert(ieee_ulp_eq(flt, 0, valueD2, 4));
}

Test(symdiff_node, sustract, .init = setup, .fini = teardown) {
	SymbolNode* variable = SymbolNodeVariable(symbolNodeArray); // v
	SymbolNode* t1 = SymbolNodeBinary(symbolNodeArray, SUSTRACT, variable, variable); // v - v
	SymbolNode* t2 = SymbolNodeConstant(symbolNodeArray, 10); // 10
	SymbolNode* t3 = SymbolNodeConstant(symbolNodeArray, 20); // 20
	SymbolNode* t4 = SymbolNodeBinary(symbolNodeArray, SUSTRACT, t2, t3); // 10 + 20
	SymbolNode* expression = SymbolNodeBinary(symbolNodeArray, ADD, t1, t4); // v - v + 10 - 20

	const float value = SymbolNodeEvaluate(expression, symbolNodeArray, variable, 100)->data.value;
	cr_assert(ieee_ulp_eq(flt, -10, value, 4));

	SymbolNode* derivate = SymbolNodeDifferentiate(expression, symbolNodeArray, variable);
	const float valueD = SymbolNodeEvaluate(derivate, symbolNodeArray, variable, 100)->data.value;
	cr_assert(ieee_ulp_eq(flt, 0, valueD, 4));

	SymbolNode* derivate2 = SymbolNodeDifferentiate(derivate, symbolNodeArray, variable);
	const float valueD2 = SymbolNodeEvaluate(derivate2, symbolNodeArray, variable, 100)->data.value;
	cr_assert(ieee_ulp_eq(flt, 0, valueD2, 4));
}

Test(symdiff_node, multiply, .init = setup, .fini = teardown) {
	SymbolNode* variable = SymbolNodeVariable(symbolNodeArray); // v
	SymbolNode* t1 = SymbolNodeBinary(symbolNodeArray, MULTIPLY, variable, variable); // v * v
	SymbolNode* t2 = SymbolNodeConstant(symbolNodeArray, 10); // 10
	SymbolNode* t3 = SymbolNodeConstant(symbolNodeArray, 20); // 20
	SymbolNode* t4 = SymbolNodeBinary(symbolNodeArray, MULTIPLY, t2, t3); // 10 * 20
	SymbolNode* expression = SymbolNodeBinary(symbolNodeArray, ADD, t1, t4); // v * v + 10 * 20

	const float value = SymbolNodeEvaluate(expression, symbolNodeArray, variable, 100)->data.value;
	cr_assert(ieee_ulp_eq(flt, 10200, value, 4));

	SymbolNode* derivate = SymbolNodeDifferentiate(expression, symbolNodeArray, variable);
	const float valueD = SymbolNodeEvaluate(derivate, symbolNodeArray, variable, 100)->data.value;
	cr_assert(ieee_ulp_eq(flt, 200, valueD, 4));

	SymbolNode* derivate2 = SymbolNodeDifferentiate(derivate, symbolNodeArray, variable);
	const float valueD2 = SymbolNodeEvaluate(derivate2, symbolNodeArray, variable, 100)->data.value;
	cr_assert(ieee_ulp_eq(flt, 2, valueD2, 4));

	SymbolNode* derivate3 = SymbolNodeDifferentiate(derivate2, symbolNodeArray, variable);
	const float valueD3 = SymbolNodeEvaluate(derivate3, symbolNodeArray, variable, 100)->data.value;
	cr_assert(ieee_ulp_eq(flt, 0, valueD3, 4));
}

Test(symdiff_node, general, .init = setup, .fini = teardown) {
	// From https://www.cs.utexas.edu/users/novak/asg-symdif.html
	// x3 + 2*x2 - 4*x + 3
	SymbolNode* variable = SymbolNodeVariable(symbolNodeArray); // v
	SymbolNode* t1 = SymbolNodeBinary(symbolNodeArray, MULTIPLY, variable, variable); // v**2
	SymbolNode* t2 = SymbolNodeBinary(symbolNodeArray, MULTIPLY, t1, variable); // v**3
	SymbolNode* t3 = SymbolNodeConstant(symbolNodeArray, 2); // 2
	SymbolNode* t4 = SymbolNodeBinary(symbolNodeArray, MULTIPLY, t3, t1); // 2 * v**2
	SymbolNode* t5 = SymbolNodeConstant(symbolNodeArray, 4); // 4
	SymbolNode* t6 = SymbolNodeBinary(symbolNodeArray, MULTIPLY, t5, variable); // 4 * v
	SymbolNode* t7 = SymbolNodeConstant(symbolNodeArray, 3); // 3

	SymbolNode* t8 = SymbolNodeBinary(symbolNodeArray, ADD, t2, t4); // v**3 + 2 * v**2
	SymbolNode* t9 = SymbolNodeBinary(symbolNodeArray, SUSTRACT, t8, t6); // v**3 + 2 * v**2 - 4 * v
	SymbolNode* expression = SymbolNodeBinary(symbolNodeArray, ADD, t9, t7); // v**3 + 2 * v**2 - 4 * v + 3

	const float value = SymbolNodeEvaluate(expression, symbolNodeArray, variable, 100)->data.value; // 100**3 + 2 * 100**2 - 4 * 100 + 3
	cr_assert(ieee_ulp_eq(flt, 1019603, value, 4));

	SymbolNode* derivate = SymbolNodeDifferentiate(expression, symbolNodeArray, variable); // 3 * v**2 + 4 * v - 4
	const float valueD = SymbolNodeEvaluate(derivate, symbolNodeArray, variable, 100)->data.value; // 3 * 100**2 + 4 * 100 - 4
	cr_assert(ieee_ulp_eq(flt, 30396, valueD, 4));

	SymbolNode* derivate2 = SymbolNodeDifferentiate(derivate, symbolNodeArray, variable); // 6 * v + 4
	const float valueD2 = SymbolNodeEvaluate(derivate2, symbolNodeArray, variable, 100)->data.value; // 6 * 100 + 4
	cr_assert(ieee_ulp_eq(flt, 604, valueD2, 4));
}
