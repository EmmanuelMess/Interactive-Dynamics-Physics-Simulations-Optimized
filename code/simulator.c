#include "simulator.h"

#include <stdbool.h>
#include <stdio.h>

#include "symdiff.h"
#include "custom_assert.h"

//----------------------------------------------------------------------------------
// Particle
//----------------------------------------------------------------------------------

ParticleArray* ParticleArrayCreate() {
	ParticleArray* array = malloc(sizeof(ParticleArray));
	*array = (ParticleArray) {
		.start = NULL,
		.capacity = 0,
		.size = 0,
	};
	return array;
}

void ParticleArrayFree(ParticleArray* array) {
	for (size_t i = 0; i < array->size; ++i) {
		//TODO ParticleFree(array->start[i]);
		free(array->start[i]);
	}
	free(array->start);
	free(array);
}

ParticleArray* ParticleArrayOf(unsigned int size, ...) {
	ParticleArray* array = malloc(sizeof(ParticleArray));
	*array = (ParticleArray) {
		.start = calloc(size, sizeof(Particle*)),
		.capacity = size,
		.size = size,
	};

	va_list args;
	va_start(args, size);
	for (unsigned int i = 0; i < size; ++i) {
		Particle* p = va_arg(args, Particle*);
		array->start[i] = p;
	}
	va_end(args);
	return array;
}

Particle* ParticleArrayAdd(ParticleArray* array) {
	if(array->size == array->capacity) {
		array->capacity++;
		array->start = reallocarray(array->start, array->capacity, sizeof(Particle*));

		assert(array->start != NULL, "No memory!");
	}

	Particle* particle = malloc(sizeof(Particle));
	array->start[array->size] = particle;
	array->size++;
	return particle;
}

Particle* ParticleCreate(ParticleArray* array) {
	static unsigned int index = 0;

	Particle* particle = ParticleArrayAdd(array);
	particle->index = index;
	index++;
	return particle;
}

void ParticleUpdate(Particle* particle, float timestep) {
	Vector2 t1 = particle->x;                                              // x
	Vector2 t2 = Vector2Scale(particle->v, timestep);                      // v * t
	Vector2 t3 = Vector2Scale(particle->a, 0.5f*timestep*timestep);        // 1/2 * a * t^2
	particle->x = Vector2Add(t1, Vector2Add(t2, t3));                      // x + v * t + 1/2 * a * t^2

	Vector2 t4 = Vector2Scale(particle->a, timestep);                      // a * t
	particle->v = Vector2Add(particle->v, t4);                             // v + a * t
}

//----------------------------------------------------------------------------------
// Constraint
//----------------------------------------------------------------------------------

ConstraintArray* ConstraintArrayCreate() {
	ConstraintArray* array = malloc(sizeof(ConstraintArray));
	*array = (ConstraintArray) {
		.start = NULL,
		.capacity = 0,
		.size = 0,
	};
	return array;
}

void ConstraintArrayFree(ConstraintArray* array) {
	for (size_t i = 0; i < array->size; ++i) {
		free(array->start[i]);
	}
	free(array->start);
	free(array);
}

Constraint* ConstraintArrayAdd(ConstraintArray* array) {
	if(array->size == array->capacity) {
		array->capacity++;
		array->start = reallocarray(array->start, array->capacity, sizeof(Constraint*));

		assert(array->start != NULL, "No memory!");
	}

	Constraint* particle = malloc(sizeof(Constraint));
	array->start[array->size] = particle;
	array->size++;
	return particle;
}

Constraint* ConstraintCreate(ConstraintArray* array) {
	static unsigned int index = 0;

	Constraint* particle = ConstraintArrayAdd(array);
	particle->index = index;
	index++;
	return particle;
}

//----------------------------------------------------------------------------------
// Constraint
//----------------------------------------------------------------------------------

// Provide position, velocity and acceleration for all particles in expression from values in Constraint
// Then evaluate the expression
float ConstraintEvaluateSymbolNode(Constraint* constraint, SymbolNodeArray *array, SymbolNode* expression) {
	SymbolNode* result = expression;
	result = SymbolNodeEvaluate(result, array, constraint->t, 0.0f);

	for (unsigned int i = 0; i < constraint->particles->size; ++i) {
		Particle* particle = constraint->particles->start[i];

		// Set particle position
		result = SymbolNodeEvaluate(result, array, SymbolMatrixGet(constraint->x, i, 0), particle->x.x);
		result = SymbolNodeEvaluate(result, array, SymbolMatrixGet(constraint->x, i, 1), particle->x.y);

		// Set particle velocity
		result = SymbolNodeEvaluate(result, array, SymbolMatrixGet(constraint->v, i, 0), particle->v.x);
		result = SymbolNodeEvaluate(result, array, SymbolMatrixGet(constraint->v, i, 1), particle->v.y);

		// Set particle acceleration
		result = SymbolNodeEvaluate(result, array, SymbolMatrixGet(constraint->a, i, 0), particle->a.x);
		result = SymbolNodeEvaluate(result, array, SymbolMatrixGet(constraint->a, i, 1), particle->a.y);
	}

	assert(result->operation == CONSTANT, "Variables are left without value!");
	return result->data.value;
}

// Provide position, velocity and acceleration for all particles in expression from values in Constraint
// Then evaluate the expression
MatrixN* ConstraintEvaluateSymbolMatrix(Constraint* constraint, SymbolNodeArray *array, MatrixNArray* matrixNArray,
										SymbolMatrix* expression) {
	MatrixN* matrix = MatrixNCreate(matrixNArray, expression->rows, expression->cols);
	for (unsigned int i = 0; i < expression->cols * expression->rows; ++i) {
		matrix->values[i] = ConstraintEvaluateSymbolNode(constraint, array, expression->values[i]);
	}
	return matrix;
}

//----------------------------------------------------------------------------------
// Simulator
//----------------------------------------------------------------------------------

SimulatorMatrices GetMatrices(SymbolMatrixArray *array, MatrixNArray* matrixNArray, float ks, float kd,
							  ParticleArray* particles, ConstraintArray* constraints) {
	const unsigned int d = 2;
	const unsigned int n = particles->capacity;
	const unsigned int m = constraints->capacity;

	MatrixN* dq = MatrixNCreate(matrixNArray, n, d);
	MatrixN* Q = MatrixNCreate(matrixNArray, n, d);
	MatrixN* C = MatrixNCreate(matrixNArray, m, 1);
	MatrixN* dC = MatrixNCreate(matrixNArray, m, 1);
	MatrixN* W = MatrixNCreate(matrixNArray, n * d, n * d);
	MatrixN* J = MatrixNCreate(matrixNArray, m, n * d);
	MatrixN* dJ = MatrixNCreate(matrixNArray, m, n * d);

	for (unsigned int i = 0; i < n*d; ++i) {
		*MatrixNGet(W, i, i) = 1;
	}

	for (unsigned int i = 0; i < particles->size; ++i) {
		// TODO check if this should be differenciated x(t) terms
		*MatrixNGet(dq, i, 0) = particles->start[i]->v.x;
		*MatrixNGet(dq, i, 1) = particles->start[i]->v.y;
		*MatrixNGet(Q, i, 0) = particles->start[i]->a.x;
		*MatrixNGet(Q, i, 1) = particles->start[i]->a.y;
	}

	MatrixNReshape(dq, n*d, 1);
	MatrixNReshape(Q, n*d, 1);

	for (unsigned int i = 0; i < constraints->capacity; ++i) {
		Constraint* constraint = constraints->start[i];

		float c = ConstraintEvaluateSymbolNode(constraint, array->nodeArray, constraint->constraintFunction);
		float dc_dt = ConstraintEvaluateSymbolNode(constraint, array->nodeArray, constraint->constraintFunction_dt);
		MatrixN* dc_dx = ConstraintEvaluateSymbolMatrix(constraint, array->nodeArray,  matrixNArray, constraint->constraintFunction_dx);
		MatrixN* dc_dxdt = ConstraintEvaluateSymbolMatrix(constraint, array->nodeArray, matrixNArray, constraint->constraintFunction_dxdt);

		*MatrixNGet(C, constraint->index, 0) += c;
		*MatrixNGet(dC, constraint->index, 0) += dc_dt;
		for (unsigned int j = 0; j < constraint->particles->size; ++j) {
			Particle* particle = particles->start[j];

			for (unsigned int k = 0; k < d; ++k) {
				// The constraint/particle index is for the simulation, each constraint has its own (smaller) indices
				// and has to be reindexed into the full matrix
				*MatrixNGet(J, constraint->index, particle->index + n*k) += *MatrixNGet(dc_dx, j, k);
				*MatrixNGet(dJ, constraint->index, particle->index + n*k) += *MatrixNGet(dc_dxdt, j, k);
			}
		}
	}

	// Compute f(X) = dJdq + J W Q + ks C + kd dC; g(X) = J W J'
	MatrixN* t1 = MatrixNMultiply(matrixNArray, dJ, dq); // dJ dq
	MatrixN* t2 = MatrixNMultiply(matrixNArray, J, W); // J W
	MatrixN* t3 = MatrixNMultiply(matrixNArray, t2, Q); // J W Q
	MatrixN* t4 = MatrixNMultiplyValue(matrixNArray, C, ks); // ks C
	MatrixN* t5 = MatrixNMultiplyValue(matrixNArray, dC, kd); // kd dC
	MatrixN* t6 = MatrixNAdd(matrixNArray, t1, t3); // dJ dq + J W Q
	MatrixN* t7 = MatrixNAdd(matrixNArray, t6, t4); // dJ dq + J W Q + ks C
	MatrixN* f = MatrixNAdd(matrixNArray, t7, t5); // dJ dq + J W Q + ks C + kd dC

	MatrixN* t8 = MatrixNMultiply(matrixNArray, J, W); // J W
	MatrixN* t9 = MatrixNTranspose(matrixNArray, J); // J'
	MatrixN* g = MatrixNMultiply(matrixNArray, t8, t9); // J W J'

	return (SimulatorMatrices) {
		.f = f,
		.g = g,
		.J = J,
		.norm = ks * MatrixNNorm(C) + kd * MatrixNNorm(dC)
	};
}

Simulator SimulatorCreate(ParticleArray* particles, ConstraintArray* constraints, bool printData) {
	const float ks = 0.1f;
	return (Simulator) {
		.ks = ks,
		.kd = ks * 0.1f,
		.particles = particles,
		.constraints = constraints,
		.printData = printData,
		.time = 0,
		.error = 0,
	};
}

void SimulatorUpdate(Simulator* simulator, float timestep) {
	SymbolMatrixArray* symbolMatrixArray = SymbolMatrixArrayCreate();
	MatrixNArray* matrixNArray = MatrixNArrayCreate();

	for (unsigned int i = 0; i < simulator->particles->size; ++i) {
		Particle* particle = simulator->particles->start[i];

		if(particle->isStatic) {
			continue;
		}

		particle->aApplied = Vector2Zero(); // TODO apply force
		particle->a = particle->aApplied;
	}

	SimulatorMatrices matrices = GetMatrices(symbolMatrixArray, matrixNArray, simulator->ks,
											 simulator->kd, simulator->particles, simulator->constraints);

	assert(matrices.f->rows == simulator->constraints->size && matrices.f->cols == 1, "Wrong size for simulator matrices!");
	assert(matrices.g->rows == simulator->constraints->size && matrices.g->cols == simulator->constraints->size, "Wrong size for simulator matrices!");
	assert(matrices.J->rows == simulator->constraints->size && matrices.J->cols == simulator->particles->size * 2, "Wrong size for simulator matrices!");

	// Solve for x in g(X) * λ = -f(X)
	MatrixN* t10 = MatrixNPseudoinverse(matrixNArray, matrices.g);
	MatrixN* t11 = MatrixNNegate(matrixNArray, matrices.f);
	MatrixN* lambda = MatrixNMultiply(matrixNArray, t10, t11);

	assert(lambda->rows == simulator->constraints->size && lambda->cols == 1, "Wrong size for simulator matrices!");

	// Solve for accelerations in J' * λ = â
	MatrixN* transposeJ = MatrixNTranspose(matrixNArray, matrices.J);
	MatrixN* aConstraint = MatrixNMultiply(matrixNArray, transposeJ, lambda);
	MatrixNReshape(aConstraint, simulator->particles->size, 2);

	for (unsigned int i = 0; i < simulator->particles->size; ++i) {
		Particle* particle = simulator->particles->start[i];

		if(particle->isStatic) {
			continue;
		}

		particle->aConstraint.x = *MatrixNGet(aConstraint, i, 0);
		particle->aConstraint.y = *MatrixNGet(aConstraint, i, 1);
		particle->a = Vector2Add(particle->aApplied, particle->aConstraint);

		ParticleUpdate(particle, timestep);
	}

	simulator->error = matrices.norm;
	simulator->time += timestep;

	if(simulator->printData) {
		TraceLog(LOG_DEBUG, "---------");
		TraceLog(LOG_DEBUG, "t %f", simulator->time);
		TraceLog(LOG_DEBUG, "ks %f", simulator->ks);
		TraceLog(LOG_DEBUG, "kd %f", simulator->kd);
		TraceLog(LOG_DEBUG, "J");
		MatrixNPrint(matrices.J);
		TraceLog(LOG_DEBUG, "f = dJ dq + J W Q + ks C + kd dC");
		MatrixNPrint(matrices.f);
		TraceLog(LOG_DEBUG, "g = J W J.T");
		MatrixNPrint(matrices.g);
		TraceLog(LOG_DEBUG, "λ");
		MatrixNPrint(lambda);
		TraceLog(LOG_DEBUG, "g λ' + f");
		MatrixN* r = MatrixNAdd(matrixNArray, MatrixNMultiply(matrixNArray, matrices.g, lambda), matrices.f);
		MatrixNPrint(r);
	}

	SymbolMatrixArrayFree(symbolMatrixArray);
	MatrixNArrayFree(matrixNArray);
}
