#include "simulator.h"

#include <stdbool.h>

#include "symdiff.h"

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
	SymbolNode* h = SymbolNodeBinary(array->nodeArray, ADD, SymbolMatrixNode(g, 0, 0), SymbolMatrixNode(g, 1, 0));      // sum((x(t) - center) ** 2 / 2 - (radius ** 2) / 2)

	return h;
}

//----------------------------------------------------------------------------------
// Matrix3
//----------------------------------------------------------------------------------

Matrix3 Matrix3Create(unsigned int rows, unsigned int cols, unsigned int elements) {
	return (Matrix3) {
		.rows = rows,
		.cols = cols,
		.elements = elements,
		.values = calloc(rows * cols * elements, sizeof(float))
	};
}

void Matrix3Free(Matrix3 * matrix) {
	free(matrix->values);
}

float* Matrix3Get(Matrix3 * matrix, unsigned int row, unsigned int col, unsigned int element) {
	return &matrix->values[row * matrix->rows * matrix->cols + col * matrix->cols + element];
}

//----------------------------------------------------------------------------------
// Constraint
//----------------------------------------------------------------------------------

float ConstraintEvaluateSymbolNode(Constraint* constraint, SymbolNodeArray *array, SymbolNode* expression) {
	SymbolNode* result = expression;
	for (unsigned int i = 0; i < constraint->particles.length; ++i) {
		// Set particle position
		result = SymbolNodeEvaluate(result, array, SymbolMatrixNode(&constraint->x[i], 0, 0), constraint->particles.particles[i].x.x);
		result = SymbolNodeEvaluate(result, array, SymbolMatrixNode(&constraint->x[i], 1, 0), constraint->particles.particles[i].x.y);

		// Set particle velocity
		result = SymbolNodeEvaluate(result, array, SymbolMatrixNode(&constraint->v[i], 0, 0), constraint->particles.particles[i].v.x);
		result = SymbolNodeEvaluate(result, array, SymbolMatrixNode(&constraint->v[i], 1, 0), constraint->particles.particles[i].v.y);

		// Set particle acceleration
		result = SymbolNodeEvaluate(result, array, SymbolMatrixNode(&constraint->a[i], 0, 0), constraint->particles.particles[i].a.x);
		result = SymbolNodeEvaluate(result, array, SymbolMatrixNode(&constraint->a[i], 1, 0), constraint->particles.particles[i].a.y);
	}

	if(result->operation != CONSTANT) {
		TraceLog(LOG_ERROR, "Variables are left without value");
		SymbolNodePrint(result);
		exit(EXIT_FAILURE);
	}

	return result->data.value;
}

Matrix3 ConstraintEvaluateSymbolMatrix(Constraint* constraint, SymbolNodeArray *array, SymbolMatrix* expression) {
	Matrix3 matrix = Matrix3Create(expression->rows, expression->cols, 1);
	for (unsigned int i = 0; i < expression->cols * expression->rows; ++i) {
		matrix.values[i] = ConstraintEvaluateSymbolNode(constraint, array, expression->values[i]);
	}
	return matrix;
}

//----------------------------------------------------------------------------------
// Simulator
//----------------------------------------------------------------------------------

SimulatorMatrices GetMatrices(SymbolNodeArray *array, ParticleArray particles, ConstraintArray constraints) {
	const unsigned int d = 2;
	const unsigned int n = particles.length;
	const unsigned int m = constraints.length;
	const float ks = 0.1f;
	const float kd = 1.0f;

	Matrix3 dq = Matrix3Create(n, d, 1);
	Matrix3 Q = Matrix3Create(n, d, 1);
	Matrix3 C = Matrix3Create(m, 1, 1);
	Matrix3 dC = Matrix3Create(m, 1, 1);
	Matrix3 W = Matrix3Create(n * d, n * d, 1);
	Matrix3 J = Matrix3Create(m, n, d);
	Matrix3 dJ = Matrix3Create(m, n, d);

	for (unsigned int i = 0; i < n*d; ++i) {
		*Matrix3Get(&W, i, i, 0) = 1;
	}

	for (unsigned int i = 0; i < particles.length; ++i) {
		*Matrix3Get(&dq, i, 0, 0) = particles.particles[i].v.x;
		*Matrix3Get(&dq, i, 1, 0) = particles.particles[i].v.y;
		*Matrix3Get(&Q, i, 0, 0) = particles.particles[i].a.x;
		*Matrix3Get(&Q, i, 1, 0) = particles.particles[i].a.y;
	}

	for (unsigned int i = 0; i < constraints.length; ++i) {
		Constraint* constraint = &constraints.constraints[i];

		float f = ConstraintEvaluateSymbolNode(constraint, array, constraint->constraintFunction);
		float df_dt = ConstraintEvaluateSymbolNode(constraint, array, constraint->constraintFunction_dt);
		Matrix3 df_dx = ConstraintEvaluateSymbolMatrix(constraint, array, constraint->constraintFunction_dx);
		Matrix3 df_dxdt = ConstraintEvaluateSymbolMatrix(constraint, array, constraint->constraintFunction_dxdt);

		
	}


}

Simulator SimulatorCreate(ParticleArray particles, ConstraintArray constraints, bool printData) {
	return (Simulator) {
		.particles = particles,
		.constraints = constraints,
		.printData = printData,
		.time = 0,
		.error = 0,
	};
}

void SimulatorUpdate(Simulator* simulator, float timestep) {
	for (unsigned int i = 0; i < simulator->particles.length; ++i) {
		Particle* particle = &simulator->particles.particles[i];

		if(particle->isStatic) {
			continue;
		}

		particle->aApplied = Vector2Zero(); // TODO apply force
		particle->a = particle->aApplied;
	}

	SimulationMatrices matrices = GetMatrices(simulator->particles, simulator->constraints);

	Matrix lambda = Optimize(&matrices);

	Matrix aConstraint = matrices.J * lambda;

	for (unsigned int i = 0; i < simulator->particles.length; ++i) {
		Particle* particle = &simulator->particles.particles[i];

		if(particle->isStatic) {
			continue;
		}

		particle->aConstraint = aConstraint;
		particle->a = Vector2Add(particle->aApplied, particle->aConstraint);

		ParticleUpdate(particle, timestep); // Update position and velocity, taking new acceleration into account
	}
}
