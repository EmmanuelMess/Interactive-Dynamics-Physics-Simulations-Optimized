#include "simulator.h"

#include <stdbool.h>

#include "symdiff.h"

//----------------------------------------------------------------------------------
// Particle
//----------------------------------------------------------------------------------

ParticleArray* ParticleArrayCreate() {
	ParticleArray* array = malloc(sizeof(ParticleArray));
	*array = (ParticleArray) {
		.start = NULL,
		.size = 0,
		.last = 0,
	};
	return array;
}

void ParticleArrayFree(ParticleArray* array) {
	for (size_t i = 0; i < array->last; ++i) {
		//TODO ParticleFree(array->start[i]);
	}
	free(array->start);
	free(array);
}

Particle* ParticleArrayAdd(ParticleArray* array) {
	if(array->last == array->size) {
		array->size++;
		array->start = reallocarray(array->start, array->size, sizeof(Particle*));

		if (array->start == NULL) {
			TraceLog(LOG_ERROR, "No memory");
			exit(EXIT_FAILURE);
		}
	}

	Particle* particle = malloc(sizeof(Particle));
	array->start[array->last] = particle;
	array->last++;
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
		.size = 0,
		.last = 0,
	};
	return array;
}

void ConstraintArrayFree(ConstraintArray* array) {
	for (size_t i = 0; i < array->last; ++i) {
		//TODO ConstraintFree(array->start[i]);
	}
	free(array->start);
	free(array);
}

Constraint* ConstraintArrayAdd(ConstraintArray* array) {
	if(array->last == array->size) {
		array->size++;
		array->start = reallocarray(array->start, array->size, sizeof(Constraint*));

		if (array->start == NULL) {
			TraceLog(LOG_ERROR, "No memory");
			exit(EXIT_FAILURE);
		}
	}

	Constraint* particle = malloc(sizeof(Constraint));
	array->start[array->last] = particle;
	array->last++;
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
// Circle
//----------------------------------------------------------------------------------

SymbolNode* constraintCircle(SymbolMatrixArray* array, SymbolNode* t, SymbolMatrix* x, SymbolMatrix* v, SymbolMatrix* a,
							 ...) {
	va_list args;
	va_start(args, a);
	const Vector2 center = va_arg(args, Vector2);
	const Vector2 radius = va_arg(args, Vector2);
	va_end(args);

	SymbolNode* half = SymbolNodeConstant(array->nodeArray, 0.5f);

	SymbolMatrix* t1 = x;                                                                                               // x
	SymbolMatrix* t2 = SymbolMatrixMultiplyValue(array, v, t);                                                          // v * t
	SymbolNode* m1 = SymbolNodeBinary(array->nodeArray, MULTIPLY, t, t);                                                // t^2
	SymbolMatrix* m2 = SymbolMatrixMultiplyValue(array, a, m1);                                                         // a * t^2
	SymbolMatrix* t3 = SymbolMatrixMultiplyValue(array, m2, half);                                                      // 1/2 * a * t^2
	SymbolMatrix* t5 = SymbolMatrixAdd(array, t1, t2);                                                                  // x + v * t
	SymbolMatrix* t6 = SymbolMatrixAdd(array, t5, t3);                                                                  // x + v * t + 1/2 * a * t^2

	//const float distance = sum((x(t) - center) ** 2 / 2 - (radius ** 2) / 2);

	SymbolMatrix* b = SymbolMatrixCreate(array, 2, 1);                                                                  // -1 * center
	SymbolMatrixSetNode(b, 0, 0, SymbolNodeConstant(array->nodeArray, -center.x));
	SymbolMatrixSetNode(b, 1, 0, SymbolNodeConstant(array->nodeArray, -center.y));

	SymbolMatrix* c = SymbolMatrixAdd(array, t6, b);                                                                    // x(t) - center
	SymbolMatrix* d = SymbolMatrixMultiplyElementWise(array, c, c);                                                     // (x(t) - center) ** 2
	SymbolMatrix* e = SymbolMatrixMultiplyValue(array, d, half);                                                        // (x(t) - center) ** 2 / 2
	SymbolMatrix* f = SymbolMatrixCreate(array, 2, 1);                                                                  // - (radius ** 2) / 2)
	SymbolMatrixSetNode(f, 0, 0, SymbolNodeConstant(array->nodeArray, -(radius.x * radius.x) / 2.0f));
	SymbolMatrixSetNode(f, 1, 0, SymbolNodeConstant(array->nodeArray, -(radius.y * radius.y) / 2.0f));
	SymbolMatrix* g = SymbolMatrixMultiplyElementWise(array, e, f);                                                     // (x(t) - center) ** 2 / 2 - (radius ** 2) / 2
	SymbolNode* h = SymbolNodeBinary(array->nodeArray, ADD, SymbolMatrixGetNode(g, 0, 0), SymbolMatrixGetNode(g, 1, 0));      // sum((x(t) - center) ** 2 / 2 - (radius ** 2) / 2)

	return h;
}

//----------------------------------------------------------------------------------
// Constraint
//----------------------------------------------------------------------------------

// Provide position, velocity and acceleration for all particles in expression from values in Constraint
// Then evaluate the expression
float ConstraintEvaluateSymbolNode(Constraint* constraint, SymbolNodeArray *array, SymbolNode* expression) {
	SymbolNode* result = expression;
	result = SymbolNodeEvaluate(result, array, constraint->t, 0);

	for (unsigned int i = 0; i < constraint->particles->last; ++i) {
		Particle* particle = constraint->particles->start[i];

		// Set particle position
		result = SymbolNodeEvaluate(result, array, SymbolMatrixGetNode(constraint->x, 0, i), particle->x.x);
		result = SymbolNodeEvaluate(result, array, SymbolMatrixGetNode(constraint->x, 1, i), particle->x.y);

		// Set particle velocity
		result = SymbolNodeEvaluate(result, array, SymbolMatrixGetNode(constraint->v, 0, i), particle->v.x);
		result = SymbolNodeEvaluate(result, array, SymbolMatrixGetNode(constraint->v, 1, i), particle->v.y);

		// Set particle acceleration
		result = SymbolNodeEvaluate(result, array, SymbolMatrixGetNode(constraint->a, 0, i), particle->a.x);
		result = SymbolNodeEvaluate(result, array, SymbolMatrixGetNode(constraint->a, 1, i), particle->a.y);
	}

	if(result->operation != CONSTANT) {
		TraceLog(LOG_ERROR, "Variables are left without value");
		SymbolNodePrint(expression);
		SymbolNodePrint(result);
		exit(EXIT_FAILURE);
	}

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
	const unsigned int n = particles->size;
	const unsigned int m = constraints->size;

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

	for (unsigned int i = 0; i < particles->last; ++i) {
		// TODO check if this should be differenciated x(t) terms
		*MatrixNGet(dq, i, 0) = particles->start[i]->v.x;
		*MatrixNGet(dq, i, 1) = particles->start[i]->v.y;
		*MatrixNGet(Q, i, 0) = particles->start[i]->a.x;
		*MatrixNGet(Q, i, 1) = particles->start[i]->a.y;
	}

	MatrixNReshape(dq, n*d, 1);
	MatrixNReshape(Q, n*d, 1);

	for (unsigned int i = 0; i < constraints->size; ++i) {
		Constraint* constraint = constraints->start[i];

		float c = ConstraintEvaluateSymbolNode(constraint, array->nodeArray, constraint->constraintFunction);
		float dc_dt = ConstraintEvaluateSymbolNode(constraint, array->nodeArray, constraint->constraintFunction_dt);
		MatrixN* dc_dx = ConstraintEvaluateSymbolMatrix(constraint, array->nodeArray,  matrixNArray, constraint->constraintFunction_dx);
		MatrixN* dc_dxdt = ConstraintEvaluateSymbolMatrix(constraint, array->nodeArray, matrixNArray, constraint->constraintFunction_dxdt);

		*MatrixNGet(C, constraint->index, 0) += c;
		*MatrixNGet(dC, constraint->index, 0) += dc_dt;
		for (unsigned int j = 0; j < particles->last; ++j) {
			Particle* particle = particles->start[j];

			for (unsigned int k = 0; k < d; ++k) {
				*MatrixNGet(J, constraint->index, particle->index + d*k) += *MatrixNGet(dc_dx, j, k);
				*MatrixNGet(dJ, constraint->index, particle->index + d*k) += *MatrixNGet(dc_dxdt, j, k);
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
	return (Simulator) {
		.ks = 0.1f,
		.kd = 0.01f,
		.particles = particles,
		.constraints = constraints,
		.printData = printData,
		.time = 0,
		.error = 0,
	};
}

void SimulatorUpdate(Simulator* simulator, float timestep) {
	for (unsigned int i = 0; i < simulator->particles->last; ++i) {
		Particle* particle = simulator->particles->start[i];

		if(particle->isStatic) {
			continue;
		}

		particle->aApplied = Vector2Zero(); // TODO apply force
		particle->a = particle->aApplied;
	}

	SymbolNodeArray* array = SymbolNodeArrayCreate();
	SymbolMatrixArray* symbolMatrixArray = SymbolMatrixArrayCreate(array);
	MatrixNArray* matrixNArray = MatrixNArrayCreate();

	SimulatorMatrices matrices = GetMatrices(symbolMatrixArray, matrixNArray, simulator->ks, simulator->kd,
											 simulator->particles, simulator->constraints);

	// Solve for x in g(X) * λ = -f(X)
	MatrixN* t10 = MatrixNPseudoinverse(matrixNArray, matrices.g);
	MatrixN* t11 = MatrixNNegate(matrixNArray, matrices.f);
	MatrixN* lambda = MatrixNMultiply(matrixNArray, t10, t11);

	// Solve for accelerations in J' * λ = â
	MatrixN* transposeJ = MatrixNTranspose(matrixNArray, matrices.J);
	MatrixN* aConstraint = MatrixNMultiply(matrixNArray, transposeJ, lambda);

	for (unsigned int i = 0; i < simulator->particles->last; ++i) {
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
}
