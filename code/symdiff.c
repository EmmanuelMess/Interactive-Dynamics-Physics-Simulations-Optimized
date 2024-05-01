#include "symdiff.h"

#include <stdio.h>
#include <raylib.h>

SymbolNodeArray* SymbolNodeArrayCreate() {
	SymbolNodeArray* array = malloc(sizeof(SymbolNodeArray));
	*array = (SymbolNodeArray) { .start = NULL, .size = 0, .last = 0};
	return array;
}

void SymbolNodeArrayFree(SymbolNodeArray* array) {
	for (unsigned int i = 0; i < array->last; ++i) {
		free(array->start[i]);
	}
	free(array->start);
	free(array);
}

SymbolNode* NodeArrayAdd(SymbolNodeArray* array) {
	if(array->last == array->size) {
		array->size++;
		array->start = reallocarray(array->start, array->size, sizeof(SymbolNode*));

		if (array->start == NULL) {
			TraceLog(LOG_ERROR, "No memory");
			exit(EXIT_FAILURE);
		}
	}

	SymbolNode* node = malloc(sizeof(SymbolNode));
	array->start[array->last] = node;
	array->last++;
	return node;
}

void SymbolNodeArrayPrint(SymbolNodeArray* array) {
	for (unsigned int i = 0; i < array->last; ++i) {
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
	SymbolNode* node = NodeArrayAdd(array);
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

//-----------------------------------------------------------------------------
// SymbolMatrix
//-----------------------------------------------------------------------------

SymbolMatrixArray* SymbolMatrixArrayCreate(SymbolNodeArray *nodeArray) {
	SymbolMatrixArray* array = malloc(sizeof(SymbolMatrixArray));
	*array = (SymbolMatrixArray) { .start = NULL, .nodeArray = nodeArray, .size = 0, .last = 0};
	return array;
}

void SymbolMatrixArrayFree(SymbolMatrixArray* array) {
	for (unsigned int i = 0; i < array->last; ++i) {
		SymbolMatrixFree(array->start[i]);
	}

	free(array->start);
	free(array);
}

SymbolMatrix* SymbolMatrixArrayAdd(SymbolMatrixArray* array) {
	if(array->last == array->size) {
		array->size++;
		array->start = reallocarray(array->start, array->size, sizeof(SymbolNode*));

		if (array->start == NULL) {
			TraceLog(LOG_ERROR, "No memory");
			exit(EXIT_FAILURE);
		}
	}


	SymbolMatrix* matrix = malloc(sizeof(SymbolMatrix));
	array->start[array->last] = matrix;
	array->last++;
	return matrix;
}

void SymbolMatrixArrayPrint(SymbolMatrixArray* array) {
	for (unsigned int i = 0; i < array->last; ++i) {
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

void SymbolMatrixFree(SymbolMatrix* matrix) {
	free(matrix->values);
}

void SymbolMatrixSetNode(SymbolMatrix *matrix, unsigned int row, unsigned int col, SymbolNode *value) {
	matrix->values[row + col * matrix->cols] = value;
}

SymbolNode *SymbolMatrixGetNode(SymbolMatrix *matrix, unsigned int row, unsigned int col) {
	return matrix->values[row + col * matrix->cols];
}

SymbolMatrix* SymbolMatrixTranspose(SymbolMatrix* matrix) {
	for (unsigned int i = 0; i < matrix->rows / 2; ++i) {
		for (unsigned int j = 0; j < matrix->cols / 2; ++j) {
			SymbolNode* t = SymbolMatrixGetNode(matrix, i, j);
			SymbolMatrixSetNode(matrix, i, j, SymbolMatrixGetNode(matrix, j, i));
			SymbolMatrixSetNode(matrix, j, i, t);
		}
	}

	return matrix;
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

SymbolMatrix* SymbolMatrixMultiply(SymbolMatrixArray* array, SymbolMatrix* left, SymbolMatrix* right) {
	if(left->cols != right->rows) {
		TraceLog(LOG_ERROR, "Matrix dimensions don't match!");
		exit(EXIT_FAILURE);
	}

	SymbolMatrix* sumMatrix = SymbolMatrixCreate(array, left->rows, right->cols);

	for (unsigned int i = 0; i < sumMatrix->rows; ++i) {
		for (unsigned int j = 0; i < sumMatrix->cols; ++j) {
			SymbolNode *r = SymbolNodeConstant(array->nodeArray, 0);

			for (unsigned int k = 0; i < left->cols; ++k) {
				SymbolNode* lNode = SymbolMatrixGetNode(left, i, k);
				SymbolNode* rNode = SymbolMatrixGetNode(right, k, j);
				SymbolNode* sum = SymbolNodeBinary(array->nodeArray, ADD, lNode, rNode);
				r = SymbolNodeBinary(array->nodeArray, ADD, r, sum);
			}

			SymbolMatrixSetNode(sumMatrix, i, j, r);
		}
	}

	return sumMatrix;
}

SymbolMatrix* SymbolNodeDifferentiateSymbolMatrix(SymbolNode* expression, SymbolMatrixArray* array, SymbolMatrix* variableMatrix) {
	SymbolMatrix * result = SymbolMatrixCreate(array, variableMatrix->rows, variableMatrix->cols);
	for (unsigned int col = 0; col < variableMatrix->cols; ++col) {
		for (unsigned int row = 0; row < variableMatrix->rows; ++row) {
			SymbolNode* variable = SymbolMatrixGetNode(variableMatrix, row, col);
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
			SymbolNode* valueExpression = SymbolMatrixGetNode(expression, row, col);
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
			SymbolNode* valueExpression = SymbolMatrixGetNode(expression, row, col);
			SymbolNodePrintInternal(valueExpression);
			printf("\n");
		}
	}
}

void SymbolMatrixPrint(SymbolMatrix* expression) {
	SymbolMatrixPrintInternal(expression);
}
