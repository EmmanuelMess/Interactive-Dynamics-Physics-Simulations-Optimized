#ifndef CONSTRAINT_BASED_SIMULATOR_SIMULATOR_H
#define CONSTRAINT_BASED_SIMULATOR_SIMULATOR_H

#include <raylib.h>
#include <raymath.h>

#include "symdiff.h"

// Three dimensional matrix
typedef struct Matrix3 {
	unsigned int rows;
	unsigned int cols;
	unsigned int elements;
	float * values;
} Matrix3;

Matrix3 Matrix3Create(unsigned int rows, unsigned int cols, unsigned int elements);

void Matrix3Free(Matrix3 * matrix);

typedef SymbolNode*(*ConstraintFunction)(SymbolMatrixArray* array, SymbolNode* t, SymbolMatrix* x, Vector2 v, Vector2 a, ...);

typedef struct Particle {
	Vector2 x;
	Vector2 v;
	Vector2 a;
	Vector2 aApplied;
	Vector2 aConstraint;
	bool isStatic;
} Particle;

typedef struct ParticleArray {
	Particle* particles;
	unsigned int length;
} ParticleArray;

typedef struct Constraint {
	ParticleArray particles;
	SymbolMatrix* x;
	SymbolMatrix* v;
	SymbolMatrix* a;
	SymbolNode* constraintFunction;
	SymbolNode* constraintFunction_dt;
	SymbolMatrix* constraintFunction_dx;
	SymbolMatrix* constraintFunction_dxdt;
} Constraint;

typedef struct ConstraintArray {
	Constraint* constraints;
	unsigned int length;
} ConstraintArray;


typedef struct Simulator {
	ParticleArray particles;
	ConstraintArray constraints;
	bool printData;
	float time;
	float error;
} Simulator;

SymbolNode* constraintCircle(SymbolMatrixArray* array, SymbolNode* t, SymbolMatrix* x, SymbolMatrix* v, SymbolMatrix* a,
                             ...);
void SimulatorUpdate(Simulator* simulator, float timestep);

#endif //CONSTRAINT_BASED_SIMULATOR_SIMULATOR_H
