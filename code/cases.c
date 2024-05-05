#include "cases.h"
#include "constraint_type.h"

Simulator case1(SymbolMatrixArray* symbolMatrixArray, ParticleArray* allParticlesArray,
				ConstraintArray* allConstraintsArray) {
	Particle* particle1 = ParticleCreate(allParticlesArray, (Vector2) { .x = 320.0f, .y = 320.0f }, false);

	const Vector2 center1 = (Vector2) { .x = 200.0f, .y = 200.0f };
	const Vector2 radius1 = (Vector2) { .x = 50.0f, .y = 50.0f };
	CircleConstraintCreate(allConstraintsArray, symbolMatrixArray, ParticleArrayOf(1, particle1), center1, radius1);

	Simulator simulator = SimulatorCreate(allParticlesArray, allConstraintsArray, true);

	return simulator;
}

Simulator case2(SymbolMatrixArray* symbolMatrixArray, ParticleArray* allParticlesArray,
				ConstraintArray* allConstraintsArray) {
	Particle* particle1 = ParticleCreate(allParticlesArray, (Vector2) { .x = 320.0f, .y = 320.0f }, false);

	const Vector2 center1 = (Vector2) { .x = 200.0f, .y = 200.0f };
	const Vector2 radius1 = (Vector2) { .x = 50.0f, .y = 50.0f };
	CircleConstraintCreate(allConstraintsArray, symbolMatrixArray, ParticleArrayOf(1, particle1), center1, radius1);

	const Vector2 center2 = (Vector2) { .x = 250.0f, .y = 200.0f };
	const Vector2 radius2 = (Vector2) { .x = 50.0f, .y = 50.0f };
	CircleConstraintCreate(allConstraintsArray, symbolMatrixArray, ParticleArrayOf(1, particle1), center2, radius2);

	Simulator simulator = SimulatorCreate(allParticlesArray, allConstraintsArray, true);

	return simulator;
}


Simulator case3(SymbolMatrixArray* symbolMatrixArray, ParticleArray* allParticlesArray,
				ConstraintArray* allConstraintsArray) {
	Particle* particle1 = ParticleCreate(allParticlesArray, (Vector2) { .x = 320.0f, .y = 320.0f }, false);
	Particle* particle2 = ParticleCreate(allParticlesArray, (Vector2) { .x = 40.0f, .y = 120.0f }, false);

	const Vector2 center1 = (Vector2) { .x = 200.0f, .y = 200.0f };
	const Vector2 radius1 = (Vector2) { .x = 50.0f, .y = 50.0f };
	CircleConstraintCreate(allConstraintsArray, symbolMatrixArray, ParticleArrayOf(1, particle1), center1, radius1);

	const float distance = 10.0f;
	DistanceConstraintCreate(allConstraintsArray, symbolMatrixArray, ParticleArrayOf(2, particle1, particle2), distance);

	Simulator simulator = SimulatorCreate(allParticlesArray, allConstraintsArray, true);

	return simulator;
}
