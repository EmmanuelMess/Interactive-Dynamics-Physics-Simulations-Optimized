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
	unsigned int index;
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

typedef enum ConstraintType {
	CIRCLE,
	DISTANCE,
} ConstraintType;

typedef struct Constraint {
	ConstraintType type;
	unsigned int index;

	ParticleArray* particles;

	SymbolNode* t;
	SymbolMatrix* x;
	SymbolMatrix* v;
	SymbolMatrix* a;

	SymbolNode* constraintFunction;
	SymbolNode* constraintFunction_dt;
	SymbolMatrix* constraintFunction_dx;
	SymbolMatrix* constraintFunction_dxdt;

	union {
		struct {
			Vector2 center;
			Vector2 radius;
		} circle;
		struct {
			float distance;
		} distance;
	} metadata;
} Constraint;

typedef struct ConstraintArray {
	Constraint** start;
	unsigned int capacity;
	unsigned int size;
} ConstraintArray;

ConstraintArray* ConstraintArrayCreate();

void ConstraintArrayFree(ConstraintArray* particles);

Constraint* ConstraintCreate(ConstraintArray* array, ParticleArray* particlesArray, ConstraintType type, SymbolNode* t,
                             SymbolMatrix* x, SymbolMatrix* v, SymbolMatrix* a, SymbolNode* f, SymbolNode* df_dt,
                             SymbolMatrix* df_dx, SymbolMatrix* df_dxdt);

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
