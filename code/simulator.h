#ifndef CONSTRAINT_BASED_SIMULATOR_SIMULATOR_H
#define CONSTRAINT_BASED_SIMULATOR_SIMULATOR_H

#include <raylib.h>
#include <raymath.h>

#include "symdiff.h"
#include "matrixn.h"

typedef SymbolNode*(*ConstraintFunction)(SymbolMatrixArray* array, SymbolNode* t, SymbolMatrix* x, Vector2 v, Vector2 a, ...);

typedef struct Particle {
	unsigned int index; //TODO implement
	Vector2 x;
	Vector2 v;
	Vector2 a;
	Vector2 aApplied;
	Vector2 aConstraint;
	bool isStatic;
} Particle;

typedef struct ParticleArray {
	Particle** start;
	unsigned int size;
	unsigned int last;
} ParticleArray;

typedef struct Constraint {
	unsigned int index; //TODO implement
	ParticleArray* particles;
	SymbolNode* t;
	SymbolMatrix* x;
	SymbolMatrix* v;
	SymbolMatrix* a;
	SymbolNode* constraintFunction;
	SymbolNode* constraintFunction_dt;
	SymbolMatrix* constraintFunction_dx;
	SymbolMatrix* constraintFunction_dxdt;
} Constraint;

typedef struct ConstraintArray {
	Constraint** start;
	unsigned int size;
	unsigned int last;
} ConstraintArray;


typedef struct Simulator {
	SymbolNodeArray* symbolNodeArray;
	SymbolMatrixArray* symbolMatrixArray;
	MatrixNArray* matrixNArray;
	float ks;
	float kd;
	ParticleArray* particles;
	ConstraintArray* constraints;
	bool printData;
	float time;
	float error;
} Simulator;

typedef struct SimulatorMatrices {
	MatrixN* f;
	MatrixN* g;
	MatrixN* J;
	float norm;
} SimulatorMatrices;

ParticleArray* ParticleArrayCreate();

void ParticleArrayFree(ParticleArray* particles);

Particle* ParticleCreate(ParticleArray* array);

ConstraintArray* ConstraintArrayCreate();

void ConstraintArrayFree(ConstraintArray* particles);

Constraint* ConstraintCreate(ConstraintArray* array);

SymbolNode* constraintCircle(SymbolMatrixArray* array, SymbolNode* t, SymbolMatrix* x, SymbolMatrix* v, SymbolMatrix* a,
                             ...);

Simulator SimulatorCreate(SymbolNodeArray* symbolNodeArray, SymbolMatrixArray* symbolMatrixArray,
                          MatrixNArray* matrixNArray, ParticleArray* particles, ConstraintArray* constraints,
                          bool printData);

void SimulatorUpdate(Simulator* simulator, float timestep);

#endif //CONSTRAINT_BASED_SIMULATOR_SIMULATOR_H
