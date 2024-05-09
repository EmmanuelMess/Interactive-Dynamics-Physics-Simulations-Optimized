#include "symdiff.h"

#include <stdio.h>
#include <raylib.h>
#include <string.h>
#include <config.h>
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
		TraceLog(LOG_DEBUG, "%u:", i);
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
	assert(left != NULL, "Operand is NULL!");
	assert(right != NULL, "Operand is NULL!");

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
		case SUSTRACT: {
			return SymbolNodeBinary(
				array,
				SUSTRACT,
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
		case SUSTRACT:
		case MULTIPLY: {
			SymbolNode *left = SymbolNodeEvaluate(expression->data.children.left, array, variable, value);
			SymbolNode *right = SymbolNodeEvaluate(expression->data.children.right, array, variable, value);
			if(left->operation == CONSTANT && right->operation == CONSTANT) {
				switch(expression->operation) {
					case ADD:
						return SymbolNodeConstant(array, left->data.value + right->data.value);
					case SUSTRACT:
						return SymbolNodeConstant(array, left->data.value - right->data.value);
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

static void SymbolNodePrintInternal(SymbolNode* expression, char **end) {
	switch(expression->operation) {
		case CONSTANT:
			sprintf(*end, "%f", expression->data.value); *end += strlen(*end);
			break;
		case VARIABLE:
			sprintf(*end, "x_%u", expression->data.variableId); *end += strlen(*end);
			break;
		case ADD:
		case SUSTRACT:
		case MULTIPLY: {
			SymbolNodePrintInternal(expression->data.children.left, end);
			switch (expression->operation) {
				case ADD:
					sprintf(*end, "+"); *end += strlen(*end);
					break;
				case SUSTRACT:
					sprintf(*end, "-"); *end += strlen(*end);
					break;
				case MULTIPLY:
					sprintf(*end, "*"); *end += strlen(*end);
					break;
				default:
					__builtin_unreachable(); // This should be impossible
			}
			SymbolNodePrintInternal(expression->data.children.right, end);
			break;
		}
		default:
			assert(false, "Unhandled operation!");
	}
}

void SymbolNodePrint(SymbolNode* expression) {
	char buffer[MAX_TRACELOG_MSG_LENGTH] = { 0 };
	char* end = buffer;

	SymbolNodePrintInternal(expression, &end);
	TraceLog(LOG_DEBUG, buffer);
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
		matrix->values[i] = SymbolNodeBinary(array->nodeArray, SUSTRACT, left->values[i], right->values[i]);
	}

	return matrix;
}

SymbolMatrix* SymbolMatrixMultiplyValue(SymbolMatrixArray* array, SymbolMatrix* left, SymbolNode* right) {
	SymbolMatrix* matrix = SymbolMatrixCreate(array, left->rows, left->cols);

	for (unsigned int i = 0; i < left->rows * left->cols; ++i) {
		matrix->values[i] = SymbolNodeBinary(array->nodeArray, MULTIPLY, left->values[i], right);
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
	char buffer[MAX_TRACELOG_MSG_LENGTH] = { 0 };
	char* end = buffer;

	for (unsigned int col = 0; col < expression->cols; ++col) {
		for (unsigned int row = 0; row < expression->rows; ++row) {
			sprintf(end, "(%u, %u) ", row, col); end += strlen(end);
			SymbolNode* valueExpression = SymbolMatrixGet(expression, row, col);
			SymbolNodePrintInternal(valueExpression, &end);
			TraceLog(LOG_DEBUG, buffer); end = buffer;
		}
	}
}

void SymbolMatrixPrint(SymbolMatrix* expression) {
	SymbolMatrixPrintInternal(expression);
}
