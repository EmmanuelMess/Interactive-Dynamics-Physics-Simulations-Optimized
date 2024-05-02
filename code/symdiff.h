#ifndef SIMULATOR_SYMDIFF_H
#define SIMULATOR_SYMDIFF_H

#include <stdlib.h>

//-----------------------------------------------------------------------------
// SymbolNode
//-----------------------------------------------------------------------------

typedef enum Operation {
	CONSTANT = 0,
	VARIABLE,
	ADD,
	MULTIPLY,
} Operation;

struct SymbolNode;

typedef struct SymbolNode {
	Operation operation;
	union {
		float value;
		unsigned int variableId;
		struct {
			struct SymbolNode* left;
			struct SymbolNode* right;
		} children;
	} data;
} SymbolNode;

typedef struct SymbolNodeArray {
	SymbolNode **start;
	size_t size;
	size_t last;
} SymbolNodeArray;

SymbolNodeArray* SymbolNodeArrayCreate();

void SymbolNodeArrayFree(SymbolNodeArray* array);

void SymbolNodeArrayPrint(SymbolNodeArray* array);

SymbolNode* SymbolNodeConstant(SymbolNodeArray* array, float value);

SymbolNode *SymbolNodeVariable(SymbolNodeArray *array);

SymbolNode* SymbolNodeBinary(SymbolNodeArray* array, Operation operation, SymbolNode* left, SymbolNode* right);

SymbolNode* SymbolNodeDifferentiate(SymbolNode* expression, SymbolNodeArray* array, SymbolNode* variable);

SymbolNode* SymbolNodeEvaluate(SymbolNode* expression, SymbolNodeArray* array, SymbolNode *variable, float value);

void SymbolNodePrint(SymbolNode* expression);

//-----------------------------------------------------------------------------
// SymbolMatrix
//-----------------------------------------------------------------------------

typedef struct SymbolMatrix {
	SymbolNode** values;
	unsigned int cols;
	unsigned int rows;
} SymbolMatrix;

typedef struct SymbolMatrixArray {
	SymbolMatrix **start;
	SymbolNodeArray *nodeArray;
	size_t size;
	size_t last;
} SymbolMatrixArray;


SymbolMatrixArray* SymbolMatrixArrayCreate();

void SymbolMatrixArrayFree(SymbolMatrixArray* array);

void SymbolMatrixArrayPrint(SymbolMatrixArray* array);

SymbolMatrix *SymbolMatrixCreate(SymbolMatrixArray *array, unsigned int rows, unsigned int cols);

void SymbolMatrixFree(SymbolMatrix *matrix);

void SymbolMatrixSet(SymbolMatrix *matrix, unsigned int row, unsigned int col, SymbolNode *value);

SymbolNode *SymbolMatrixGet(SymbolMatrix *matrix, unsigned int row, unsigned int col);

SymbolMatrix* SymbolMatrixTranspose(SymbolMatrix* matrix);

SymbolMatrix* SymbolMatrixAdd(SymbolMatrixArray* array, SymbolMatrix* left, SymbolMatrix* right);

SymbolMatrix* SymbolMatrixMultiplyValue(SymbolMatrixArray* array, SymbolMatrix* left, SymbolNode* right);

SymbolMatrix* SymbolMatrixMultiplyElementWise(SymbolMatrixArray* array, SymbolMatrix* left, SymbolMatrix* right);

SymbolMatrix* SymbolMatrixMultiply(SymbolMatrixArray* array, SymbolMatrix* left, SymbolMatrix* right);

SymbolMatrix* SymbolNodeDifferentiateSymbolMatrix(SymbolNode* expression, SymbolMatrixArray* array, SymbolMatrix* variableMatrix);

SymbolMatrix* SymbolMatrixDifferentiateSymbolNode(SymbolMatrix* expression, SymbolMatrixArray* array, SymbolNode* variableMatrix);

void SymbolMatrixPrint(SymbolMatrix* expression);

#endif //SIMULATOR_SYMDIFF_H
