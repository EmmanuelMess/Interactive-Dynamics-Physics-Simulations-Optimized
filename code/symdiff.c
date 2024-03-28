#include "symdiff.h"

#include <stdio.h>
#include <raylib.h>

SymbolNodeArray SymbolNodeArrayCreate() {
	return (SymbolNodeArray) { .start = NULL, .size = 0, .lastPosition = 0};
}

SymbolNode* NodeArrayAddNode(SymbolNodeArray* array) {
	if(array->lastPosition == array->size) {
		array->size += 1024;
		array->start = reallocarray(array->start, array->size, sizeof(SymbolNode*));

		if (array->start == NULL) {
			TraceLog(LOG_ERROR, "No memory");
			exit(EXIT_FAILURE);
		}
	}

	SymbolNode* node = malloc(sizeof(SymbolNode));
	array->start[array->lastPosition] = node;
	array->lastPosition++;
	return node;
}

void SymbolNodeArrayFree(SymbolNodeArray* array) {
	for (unsigned int i = 0; i < array->start; ++i) {
		free(array->start[i]);
	}
	free(array->start);
}

void SymbolNodeArrayPrint(SymbolNodeArray* array) {
	for (unsigned int i = 0; i < array->lastPosition; ++i) {
		printf("%u:\n", i);
		SymbolNodePrint(array->start[i]);
	}
}

SymbolNode* SymbolNodeConstant(SymbolNodeArray* array, float value) {
	SymbolNode* node = NodeArrayAddNode(array);
	*node = (SymbolNode)  { .operation = CONSTANT, .data.value = value };
	return node;
}

SymbolNode *SymbolNodeVariable(SymbolNodeArray *array) {
	static unsigned int variableId = 0;
	SymbolNode* node = NodeArrayAddNode(array);
	*node = (SymbolNode)  { .operation = VARIABLE, .data.variableId = variableId++};
	return node;
}

SymbolNode* SymbolNodeBinary(SymbolNodeArray* array, Operation operation, SymbolNode* left, SymbolNode* right) {
	SymbolNode* node = NodeArrayAddNode(array);
	*node = (SymbolNode)  { .operation = operation, .data.children.left = left, .data.children.right = right };
	return node;
}

SymbolNode* SymbolNodeDifferentiate(SymbolNode* expression, SymbolNodeArray* array, SymbolNode* variable) {
	if(variable->operation != VARIABLE) {
		TraceLog(LOG_ERROR, "Tried to differentiate against expression that is not a variable");
		SymbolNodePrint(variable);
		exit(EXIT_FAILURE);
	}

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
			TraceLog(LOG_ERROR, "Unhandled operation %u", expression->operation);
			exit(EXIT_FAILURE);
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
			TraceLog(LOG_ERROR, "Unhandled operation %u", expression->operation);
			exit(EXIT_FAILURE);
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
			TraceLog(LOG_ERROR, "Unhandled operation %u", expression->operation);
			exit(EXIT_FAILURE);
	}
}

void SymbolNodePrint(SymbolNode* expression) {
	SymbolNodePrintInternal(expression);
	printf("\n");
}

SymbolMatrixArray SymbolMatrixArrayCreate(SymbolNodeArray *nodeArray) {
	return (SymbolMatrixArray) { .start = NULL, .nodeArray = nodeArray, .size = 0, .lastPosition = 0};
}

SymbolMatrix* SymbolMatrixArrayAddNode(SymbolMatrixArray* array) {
	if(array->lastPosition == array->size) {
		array->size += 1024;
		array->start = reallocarray(array->start, array->size, sizeof(SymbolNode*)); // TODO this is broken

		if (array->start == NULL) {
			TraceLog(LOG_ERROR, "No memory");
			exit(EXIT_FAILURE);
		}
	}


	SymbolMatrix* matrix = malloc(sizeof(SymbolMatrix));
	array->start[array->lastPosition] = matrix;
	array->lastPosition++;
	return matrix;
}

void SymbolMatrixArrayFree(SymbolMatrixArray* array) {
	for (unsigned int i = 0; i < array->lastPosition; ++i) {
		SymbolMatrixFree(array->start[i]);
	}

	free(array->start);
}

void SymbolMatrixArrayPrint(SymbolMatrixArray* array) {
	for (unsigned int i = 0; i < array->lastPosition; ++i) {
		printf("%u:\n", i);
		SymbolMatrixPrint(array->start[i]);
	}
}

SymbolMatrix *SymbolMatrixCreate(SymbolMatrixArray *array, unsigned int rows, unsigned int cols) {
	SymbolMatrix* matrix = SymbolMatrixArrayAddNode(array);
	*matrix = (SymbolMatrix) {
		.values = calloc(cols * rows, sizeof(SymbolNode*)),
		.cols = cols,
		.rows = rows,
	};
	return matrix;
}

void SymbolMatrixFree(SymbolMatrix* matrix) {
	free(matrix->values);
}

void SymbolMatrixSetNode(SymbolMatrix *matrix, unsigned int row, unsigned int col, SymbolNode *value) {
	matrix->values[row + col * matrix->cols] = value;
}

SymbolNode *SymbolMatrixNode(SymbolMatrix *matrix, unsigned int row, unsigned int col) {
	return matrix->values[row + col * matrix->cols];
}

SymbolMatrix* SymbolMatrixAdd(SymbolMatrixArray* array, SymbolMatrix* left, SymbolMatrix* right) {
	if(!(left->rows == right->rows && left->cols == right->cols)) {
		TraceLog(LOG_ERROR, "Matrix dimensions don't match!");
		exit(EXIT_FAILURE);
	}

	SymbolMatrix* matrix = SymbolMatrixCreate(array, left->rows, left->cols);

	for (unsigned int i = 0; i < left->rows * left->cols; ++i) {
		matrix->values[i] = SymbolNodeBinary(array->nodeArray, ADD, left->values[i], right->values[i]);
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
	if(!(left->rows == right->rows && left->cols == right->cols)) {
		TraceLog(LOG_ERROR, "Matrix dimensions don't match!");
		exit(EXIT_FAILURE);
	}

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
			SymbolNode* variable = SymbolMatrixNode(variableMatrix, row, col);
			SymbolNode* r = SymbolNodeDifferentiate(expression, array->nodeArray, variable);
			SymbolMatrixSetNode(result, row, col, r);
		}
	}
	return result;
}

SymbolMatrix* SymbolMatrixDifferentiateSymbolNode(SymbolMatrix* expression, SymbolMatrixArray* array, SymbolNode* variable) {
	SymbolMatrix * result = SymbolMatrixCreate(array, expression->rows, expression->cols);
	for (unsigned int col = 0; col < expression->cols; ++col) {
		for (unsigned int row = 0; row < expression->rows; ++row) {
			SymbolNode* valueExpression = SymbolMatrixNode(expression, row, col);
			SymbolNode* r = SymbolNodeDifferentiate(valueExpression, array->nodeArray, variable);
			SymbolMatrixSetNode(result, row, col, r);
		}
	}
	return result;
}

void SymbolMatrixPrintInternal(SymbolMatrix* expression) {
	for (unsigned int col = 0; col < expression->cols; ++col) {
		for (unsigned int row = 0; row < expression->rows; ++row) {
			printf("(%u, %u) ", row, col);
			SymbolNode* valueExpression = SymbolMatrixNode(expression, row, col);
			SymbolNodePrintInternal(valueExpression);
			printf("\n");
		}
	}
}

void SymbolMatrixPrint(SymbolMatrix* expression) {
	SymbolMatrixPrintInternal(expression);
}
