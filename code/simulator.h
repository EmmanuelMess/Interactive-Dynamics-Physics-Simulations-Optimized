#ifndef CONSTRAINT_BASED_SIMULATOR_SIMULATOR_H
#define CONSTRAINT_BASED_SIMULATOR_SIMULATOR_H

#include <raylib.h>
#include <raymath.h>

#include "symdiff.h"
#include "matrixn.h"

//-----------------------------------------------------------------------------
// Particle
//-----------------------------------------------------------------------------

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
	unsigned int capacity;
	unsigned int size;
} ParticleArray;

ParticleArray* ParticleArrayCreate();

void ParticleArrayFree(ParticleArray* particles);

ParticleArray* ParticleArrayOf(unsigned int size, ...);

Particle* ParticleCreate(ParticleArray* array, Vector2 x, bool isStatic);

//-----------------------------------------------------------------------------
// Constraint
//-----------------------------------------------------------------------------

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
	unsigned int capacity;
	unsigned int size;
} ConstraintArray;

ConstraintArray* ConstraintArrayCreate();

void ConstraintArrayFree(ConstraintArray* particles);

Constraint* ConstraintCreate(ConstraintArray* array, ParticleArray* particlesArray, SymbolNode* t, SymbolMatrix* x,
                             SymbolMatrix* v, SymbolMatrix* a, SymbolNode* f, SymbolNode* df_dt, SymbolMatrix* df_dx,
                             SymbolMatrix* df_dxdt);

//-----------------------------------------------------------------------------
// Simulator
//-----------------------------------------------------------------------------

typedef struct Simulator {
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

Simulator SimulatorCreate(ParticleArray* particles, ConstraintArray* constraints, bool printData);

void SimulatorUpdate(Simulator* simulator, float timestep);

#endif //CONSTRAINT_BASED_SIMULATOR_SIMULATOR_H
