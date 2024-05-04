#include "symdiff.h"

#include <stdio.h>
#include <raylib.h>
#include "custom_assert.h"

SymbolNodeArray* SymbolNodeArrayCreate() {
	SymbolNodeArray* array = malloc(sizeof(SymbolNodeArray));
	*array = (SymbolNodeArray) { .start = NULL, .capacity = 0, .size = 0};
	return array;
}

void SymbolNodeArrayFree(SymbolNodeArray* array) {
	for (unsigned int i = 0; i < array->size; ++i) {
		free(array->start[i]);
	}
	free(array->start);
	free(array);
}

SymbolNode* NodeArrayAdd(SymbolNodeArray* array) {
	if(array->size == array->capacity) {
		array->capacity++;
		array->start = reallocarray(array->start, array->capacity, sizeof(SymbolNode*));

		assert(array->start != NULL, "No memory");
	}

	SymbolNode* node = malloc(sizeof(SymbolNode));
	array->start[array->size] = node;
	array->size++;
	return node;
}

void SymbolNodeArrayPrint(SymbolNodeArray* array) {
	for (unsigned int i = 0; i < array->size; ++i) {
		printf("%u:\n", i);
		SymbolNodePrint(array->start[i]);
	}
}

SymbolNode* SymbolNodeConstant(SymbolNodeArray* array, float value) {
	SymbolNode* node = NodeArrayAdd(array);
	*node = (SymbolNode)  { .operation = CONSTANT, .data.value = value };
	return node;
}

SymbolNode *SymbolNodeVariable(SymbolNodeArray *array) {
	static unsigned int variableId = 0;
	SymbolNode* node = NodeArrayAdd(array);
	*node = (SymbolNode)  { .operation = VARIABLE, .data.variableId = variableId};
	variableId++;
	return node;
}

SymbolNode* SymbolNodeBinary(SymbolNodeArray* array, Operation operation, SymbolNode* left, SymbolNode* right) {
	assert(left != NULL, "Tried to operate on null!");
	assert(right != NULL, "Tried to operate on null!");

	SymbolNode* node = NodeArrayAdd(array);
	*node = (SymbolNode)  { .operation = operation, .data.children.left = left, .data.children.right = right };
	return node;
}

SymbolNode* SymbolNodeDifferentiate(SymbolNode* expression, SymbolNodeArray* array, SymbolNode* variable) {
	assert(variable->operation == VARIABLE, "Tried to differentiate against expression that is not a variable!");

	switch(expression->operation) {
		case CONSTANT: {
			return SymbolNodeConstant(array, 0.0f);
		}
		case VARIABLE: {
			if(expression->data.variableId == variable->data.variableId) {
				return SymbolNodeConstant(array, 1.0f);
			} else {
				return SymbolNodeConstant(array, 0.0f);
			}
		}
		case ADD: {
			return SymbolNodeBinary(
				array,
				ADD,
				SymbolNodeDifferentiate(expression->data.children.left, array, variable),
				SymbolNodeDifferentiate(expression->data.children.right, array, variable)
			);
		}
		case MULTIPLY: {
			return SymbolNodeBinary(
				array,
				ADD,
				SymbolNodeBinary(
					array, MULTIPLY,
					expression->data.children.left,
					SymbolNodeDifferentiate(expression->data.children.right, array, variable)
				),
				SymbolNodeBinary(
					array, MULTIPLY,
					SymbolNodeDifferentiate(expression->data.children.left, array, variable),
					expression->data.children.right
				)
			);
		}
		default:
			assert(false, "Unhandled operation!");
	}
}

SymbolNode* SymbolNodeEvaluate(SymbolNode* expression, SymbolNodeArray* array, SymbolNode *variable, float value) {
	switch(expression->operation) {
		case CONSTANT:
			return expression;
		case VARIABLE:
			if(expression->data.variableId == variable->data.variableId) {
				return SymbolNodeConstant(array, value);
			} else {
				return expression;
			}
		case ADD:
		case MULTIPLY: {
			SymbolNode *left = SymbolNodeEvaluate(expression->data.children.left, array, variable, value);
			SymbolNode *right = SymbolNodeEvaluate(expression->data.children.right, array, variable, value);
			if(left->operation == CONSTANT && right->operation == CONSTANT) {
				switch(expression->operation) {
					case ADD:
						return SymbolNodeConstant(array, left->data.value + right->data.value);
					case MULTIPLY:
						return SymbolNodeConstant(array, left->data.value * right->data.value);
					default:
						__builtin_unreachable(); // This should be impossible
				}
			}

			return SymbolNodeBinary(array, expression->operation, left, right);
		}
		default: {
			assert(false, "Unhandled operation!");
		}
	}
}

void SymbolNodePrintInternal(SymbolNode* expression) {
	switch(expression->operation) {
		case CONSTANT:
			printf("%f", expression->data.value);
			break;
		case VARIABLE:
			printf("x_%u", expression->data.variableId);
			break;
		case ADD:
			SymbolNodePrintInternal(expression->data.children.left);
			printf("+");
			SymbolNodePrintInternal(expression->data.children.right);
			break;
		case MULTIPLY:
			SymbolNodePrintInternal(expression->data.children.left);
			printf("*");
			SymbolNodePrintInternal(expression->data.children.right);
			break;
		default:
			assert(false, "Unhandled operation!");
	}
}

void SymbolNodePrint(SymbolNode* expression) {
	SymbolNodePrintInternal(expression);
	printf("\n");
}

//-----------------------------------------------------------------------------
// SymbolMatrix
//-----------------------------------------------------------------------------

SymbolMatrixArray* SymbolMatrixArrayCreate() {
	SymbolMatrixArray* array = malloc(sizeof(SymbolMatrixArray));
	*array = (SymbolMatrixArray) { .start = NULL, .nodeArray = SymbolNodeArrayCreate(), .capacity = 0, .size = 0};
	return array;
}

void SymbolMatrixArrayFree(SymbolMatrixArray* array) {
	SymbolNodeArrayFree(array->nodeArray);

	for (unsigned int i = 0; i < array->size; ++i) {
		SymbolMatrixFree(array->start[i]);
		free(array->start[i]);
	}

	free(array->start);
	free(array);
}

SymbolMatrix* SymbolMatrixArrayAdd(SymbolMatrixArray* array) {
	if(array->size == array->capacity) {
		array->capacity++;
		array->start = reallocarray(array->start, array->capacity, sizeof(SymbolNode*));

		assert(array->start != NULL, "No memory!");
	}


	SymbolMatrix* matrix = malloc(sizeof(SymbolMatrix));
	array->start[array->size] = matrix;
	array->size++;
	return matrix;
}

void SymbolMatrixArrayPrint(SymbolMatrixArray* array) {
	for (unsigned int i = 0; i < array->size; ++i) {
		printf("%u:\n", i);
		SymbolMatrixPrint(array->start[i]);
	}
}

SymbolMatrix *SymbolMatrixCreate(SymbolMatrixArray *array, unsigned int rows, unsigned int cols) {
	SymbolMatrix* matrix = SymbolMatrixArrayAdd(array);
	*matrix = (SymbolMatrix) {
		.values = calloc(cols * rows, sizeof(SymbolNode*)),
		.cols = cols,
		.rows = rows,
	};
	return matrix;
}

void SymbolMatrixFree(SymbolMatrix *matrix) {
	free(matrix->values);
}

void SymbolMatrixSet(SymbolMatrix *matrix, unsigned int row, unsigned int col, SymbolNode *value) {
	assert(row < matrix->rows && col < matrix->cols, "Indexing nonexistent element!");

	matrix->values[row + matrix->rows * col] = value;
}

SymbolNode *SymbolMatrixGet(SymbolMatrix *matrix, unsigned int row, unsigned int col) {
	assert(row < matrix->rows && col < matrix->cols, "Indexing nonexistent element!");

	return matrix->values[row + matrix->rows * col];
}

SymbolMatrix* SymbolMatrixTranspose(SymbolMatrix* matrix) {
	for (unsigned int i = 0; i < matrix->rows / 2; ++i) {
		for (unsigned int j = 0; j < matrix->cols / 2; ++j) {
			SymbolNode* t = SymbolMatrixGet(matrix, i, j);
			SymbolMatrixSet(matrix, i, j, SymbolMatrixGet(matrix, j, i));
			SymbolMatrixSet(matrix, j, i, t);
		}
	}

	return matrix;
}

SymbolMatrix* SymbolMatrixAdd(SymbolMatrixArray* array, SymbolMatrix* left, SymbolMatrix* right) {
	assert(left->rows == right->rows && left->cols == right->cols, "Matrix dimensions don't match!");

	SymbolMatrix* matrix = SymbolMatrixCreate(array, left->rows, left->cols);

	for (unsigned int i = 0; i < left->rows * left->cols; ++i) {
		matrix->values[i] = SymbolNodeBinary(array->nodeArray, ADD, left->values[i], right->values[i]);
	}

	return matrix;
}

SymbolMatrix* SymbolMatrixSubtract(SymbolMatrixArray* array, SymbolMatrix* left, SymbolMatrix* right) {
	assert(left->rows == right->rows && left->cols == right->cols, "Matrix dimensions don't match!");

	SymbolMatrix* matrix = SymbolMatrixCreate(array, left->rows, left->cols);

	for (unsigned int i = 0; i < left->rows * left->cols; ++i) {
		// TODO define an operation SymbolNodeBinary SUBTRACT
		SymbolNode* t = SymbolNodeBinary(array->nodeArray, MULTIPLY, SymbolNodeConstant(array->nodeArray, -1), right->values[i]);
		matrix->values[i] = SymbolNodeBinary(array->nodeArray, ADD, left->values[i], t);
	}

	return matrix;
}

SymbolMatrix* SymbolMatrixMultiplyValue(SymbolMatrixArray* array, SymbolMatrix* left, SymbolNode* right) {
	SymbolMatrix* matrix = SymbolMatrixCreate(array, left->rows, left->cols);

	for (unsigned int i = 0; i < left->rows * left->cols; ++i) {
		matrix->values[i] = SymbolNodeBinary(array->nodeArray, ADD, left->values[i], right);
	}

	return matrix;
}

SymbolMatrix* SymbolMatrixMultiplyElementWise(SymbolMatrixArray* array, SymbolMatrix* left, SymbolMatrix* right) {
	assert(left->rows == right->rows && left->cols == right->cols, "Matrix dimensions don't match!");


	SymbolMatrix* matrix = SymbolMatrixCreate(array, left->rows, left->cols);

	for (unsigned int i = 0; i < left->rows * left->cols; ++i) {
		matrix->values[i] = SymbolNodeBinary(array->nodeArray, MULTIPLY, left->values[i], right->values[i]);
	}

	return matrix;
}

SymbolMatrix* SymbolMatrixMultiply(SymbolMatrixArray* array, SymbolMatrix* left, SymbolMatrix* right) {
	assert(left->cols == right->rows, "Matrix dimensions don't match!");

	SymbolMatrix* sumMatrix = SymbolMatrixCreate(array, left->rows, right->cols);

	for (unsigned int i = 0; i < sumMatrix->rows; ++i) {
		for (unsigned int j = 0; i < sumMatrix->cols; ++j) {
			SymbolNode *r = SymbolNodeConstant(array->nodeArray, 0);

			for (unsigned int k = 0; i < left->cols; ++k) {
				SymbolNode* lNode = SymbolMatrixGet(left, i, k);
				SymbolNode* rNode = SymbolMatrixGet(right, k, j);
				SymbolNode* sum = SymbolNodeBinary(array->nodeArray, ADD, lNode, rNode);
				r = SymbolNodeBinary(array->nodeArray, ADD, r, sum);
			}

			SymbolMatrixSet(sumMatrix, i, j, r);
		}
	}

	return sumMatrix;
}

SymbolMatrix* SymbolNodeDifferentiateSymbolMatrix(SymbolNode* expression, SymbolMatrixArray* array, SymbolMatrix* variableMatrix) {
	SymbolMatrix * result = SymbolMatrixCreate(array, variableMatrix->rows, variableMatrix->cols);
	for (unsigned int col = 0; col < variableMatrix->cols; ++col) {
		for (unsigned int row = 0; row < variableMatrix->rows; ++row) {
			SymbolNode* variable = SymbolMatrixGet(variableMatrix, row, col);
			SymbolNode* r = SymbolNodeDifferentiate(expression, array->nodeArray, variable);
			SymbolMatrixSet(result, row, col, r);
		}
	}
	return result;
}

SymbolMatrix* SymbolMatrixDifferentiateSymbolNode(SymbolMatrix* expression, SymbolMatrixArray* array, SymbolNode* variable) {
	SymbolMatrix * result = SymbolMatrixCreate(array, expression->rows, expression->cols);
	for (unsigned int col = 0; col < expression->cols; ++col) {
		for (unsigned int row = 0; row < expression->rows; ++row) {
			SymbolNode* valueExpression = SymbolMatrixGet(expression, row, col);
			SymbolNode* r = SymbolNodeDifferentiate(valueExpression, array->nodeArray, variable);
			SymbolMatrixSet(result, row, col, r);
		}
	}
	return result;
}

void SymbolMatrixPrintInternal(SymbolMatrix* expression) {
	for (unsigned int col = 0; col < expression->cols; ++col) {
		for (unsigned int row = 0; row < expression->rows; ++row) {
			printf("(%u, %u) ", row, col);
			SymbolNode* valueExpression = SymbolMatrixGet(expression, row, col);
			SymbolNodePrintInternal(valueExpression);
			printf("\n");
		}
	}
}

void SymbolMatrixPrint(SymbolMatrix* expression) {
	SymbolMatrixPrintInternal(expression);
}
