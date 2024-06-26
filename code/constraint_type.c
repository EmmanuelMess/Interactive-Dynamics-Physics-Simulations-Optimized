#include "constraint_type.h"
#include "custom_assert.h"
#include "math.h"

SymbolMatrix* TaylorPositionApproximation(SymbolMatrixArray* array, SymbolNode* t, SymbolNode* xx, SymbolNode* xy,
                                          SymbolNode* vx, SymbolNode* vy, SymbolNode* ax, SymbolNode* ay) {
	SymbolMatrix* t1 = SymbolMatrixCreate(array, 1, 2);                                                                 // x
	SymbolMatrixSet(t1, 0, 0, xx);
	SymbolMatrixSet(t1, 0, 1, xy);

	SymbolMatrix* t2 = SymbolMatrixCreate(array, 1, 2);                                                                 // v
	SymbolMatrixSet(t2, 0, 0, vx);
	SymbolMatrixSet(t2, 0, 1, vy);

	SymbolMatrix* t3 = SymbolMatrixCreate(array, 1, 2);                                                                 // a
	SymbolMatrixSet(t3, 0, 0, ax);
	SymbolMatrixSet(t3, 0, 1, ay);

	SymbolMatrix* t4 = SymbolMatrixMultiplyValue(array, t2, t);                                                         // v * t
	SymbolNode* t5 = SymbolNodeBinary(array->nodeArray, MULTIPLY, t, t);                                                // t^2
	SymbolMatrix* t6 = SymbolMatrixMultiplyValue(array, t3, t5);                                                        // a * t^2
	SymbolMatrix* t7 = SymbolMatrixMultiplyValue(array, t6, SymbolNodeConstant(array->nodeArray, 0.5f));                // 1/2 * a * t^2
	SymbolMatrix* t8 = SymbolMatrixAdd(array, t1, t4);                                                                  // x + v * t
	SymbolMatrix* t9 = SymbolMatrixAdd(array, t8, t7);                                                                  // x + v * t + 1/2 * a * t^2

	return t9;
}

SymbolNode* CircleConstraintFunction(SymbolMatrixArray* array, SymbolNode* t, SymbolMatrix* x, SymbolMatrix* v, SymbolMatrix* a,
                                     Vector2 center, Vector2 radius) {
	SymbolMatrix* positionParticle1 = TaylorPositionApproximation(array, t,
	                                                              SymbolMatrixGet(x, 0, 0), SymbolMatrixGet(x, 0, 1),
	                                                              SymbolMatrixGet(v, 0, 0), SymbolMatrixGet(v, 0, 1),
	                                                              SymbolMatrixGet(a, 0, 0), SymbolMatrixGet(a, 0, 1));

	//const float distance = sum((x(t) - center) ** 2 / 2 - (radius ** 2) / 2);

	SymbolMatrix* t1 = SymbolMatrixCreate(array, 1, 2);                                                                 // -1 * center
	SymbolMatrixSet(t1, 0, 0, SymbolNodeConstant(array->nodeArray, -center.x));
	SymbolMatrixSet(t1, 0, 1, SymbolNodeConstant(array->nodeArray, -center.y));

	SymbolMatrix* t2 = SymbolMatrixAdd(array, positionParticle1, t1);                                                   // x(t) - center
	SymbolMatrix* t3 = SymbolMatrixMultiplyElementWise(array, t2, t2);                                                  // (x(t) - center) ** 2
	SymbolMatrix* t4 = SymbolMatrixMultiplyValue(array, t3, SymbolNodeConstant(array->nodeArray, 0.5f));                // (x(t) - center) ** 2 / 2
	SymbolMatrix* t5 = SymbolMatrixCreate(array, 1, 2);                                                                 // (radius ** 2) / 2)
	SymbolMatrixSet(t5, 0, 0, SymbolNodeConstant(array->nodeArray, (radius.x * radius.x) / 2.0f));
	SymbolMatrixSet(t5, 0, 1, SymbolNodeConstant(array->nodeArray, (radius.y * radius.y) / 2.0f));
	SymbolMatrix* t6 = SymbolMatrixSubtract(array, t4, t5);                                                             // (x(t) - center) ** 2 / 2 - (radius ** 2) / 2
	SymbolNode* t7 = SymbolNodeBinary(array->nodeArray, ADD, SymbolMatrixGet(t6, 0, 0), SymbolMatrixGet(t6, 0, 1));     // sum((x(t) - center) ** 2 / 2 - (radius ** 2) / 2)

	return t7;
}

Constraint* CircleConstraintCreate(ConstraintArray* constraintsArray, SymbolMatrixArray* symbolMatrixArray,
								   ParticleArray* particlesArray, Vector2 center, Vector2 radius) {
	assert(particlesArray->size == 1, "Circle constraint has incorrect number of particles!");

	SymbolNode* t = SymbolNodeVariable(symbolMatrixArray->nodeArray);
	SymbolMatrix* x = SymbolMatrixCreate(symbolMatrixArray, 1, 2);
	SymbolMatrixSet(x, 0, 0, SymbolNodeVariable(symbolMatrixArray->nodeArray));
	SymbolMatrixSet(x, 0, 1, SymbolNodeVariable(symbolMatrixArray->nodeArray));
	SymbolMatrix* v = SymbolMatrixCreate(symbolMatrixArray, 1, 2);
	SymbolMatrixSet(v, 0, 0, SymbolNodeVariable(symbolMatrixArray->nodeArray));
	SymbolMatrixSet(v, 0, 1, SymbolNodeVariable(symbolMatrixArray->nodeArray));
	SymbolMatrix* a = SymbolMatrixCreate(symbolMatrixArray, 1, 2);
	SymbolMatrixSet(a, 0, 0, SymbolNodeVariable(symbolMatrixArray->nodeArray));
	SymbolMatrixSet(a, 0, 1, SymbolNodeVariable(symbolMatrixArray->nodeArray));

	SymbolNode* f = CircleConstraintFunction(symbolMatrixArray, t, x, v, a, center, radius);
	SymbolNode* df_dt = SymbolNodeDifferentiate(f, symbolMatrixArray->nodeArray, t);
	SymbolMatrix* df_dx = SymbolNodeDifferentiateSymbolMatrix(f, symbolMatrixArray, x);
	SymbolMatrix* df_dxdt = SymbolMatrixDifferentiateSymbolNode(df_dx, symbolMatrixArray, t);

	Constraint* constraint = ConstraintCreate(constraintsArray, particlesArray, CIRCLE, t, x, v, a, f, df_dt, df_dx,
											  df_dxdt);

	constraint->metadata.circle.center = center;
	constraint->metadata.circle.radius = radius;

	return constraint;
}

SymbolNode* DistanceConstraintFunction(SymbolMatrixArray* array, SymbolNode* t, SymbolMatrix* x, SymbolMatrix* v,
                                       SymbolMatrix* a, float distance) {
	SymbolMatrix* positionParticle1 = TaylorPositionApproximation(array, t,
	                                                              SymbolMatrixGet(x, 0, 0), SymbolMatrixGet(x, 0, 1),
	                                                              SymbolMatrixGet(v, 0, 0), SymbolMatrixGet(v, 0, 1),
	                                                              SymbolMatrixGet(a, 0, 0), SymbolMatrixGet(a, 0, 1));
	SymbolMatrix* positionParticle2 = TaylorPositionApproximation(array, t,
	                                                              SymbolMatrixGet(x, 1, 0), SymbolMatrixGet(x, 1, 1),
	                                                              SymbolMatrixGet(v, 1, 0), SymbolMatrixGet(v, 1, 1),
	                                                              SymbolMatrixGet(a, 1, 0), SymbolMatrixGet(a, 1, 1));

	//const float distance = sum((x_1(t) - x_2(t)) ** 2 / 2 - (distance ** 2) / 2);

	SymbolMatrix* t1 = SymbolMatrixCreate(array, 1, 2);                                                                 // x_1(t)
	SymbolMatrixSet(t1, 0, 0, SymbolMatrixGet(positionParticle1, 0, 0));
	SymbolMatrixSet(t1, 0, 1, SymbolMatrixGet(positionParticle1, 0, 1));

	SymbolMatrix* t2 = SymbolMatrixCreate(array, 1, 2);                                                                 // x_2(t)
	SymbolMatrixSet(t2, 0, 0, SymbolMatrixGet(positionParticle2, 0, 0));
	SymbolMatrixSet(t2, 0, 1, SymbolMatrixGet(positionParticle2, 0, 1));

	SymbolMatrix* t3 = SymbolMatrixSubtract(array, t1, t2);                                                             // x_1(t) - x_2(t)
	SymbolMatrix* t4 = SymbolMatrixMultiplyElementWise(array, t3, t3);                                                  // (x_1(t) - x_2(t)) ** 2
	SymbolMatrix* t5 = SymbolMatrixMultiplyValue(array, t4, SymbolNodeConstant(array->nodeArray, 0.5f));                // (x_1(t) - x_2(t)) ** 2 / 2
	SymbolMatrix* t6 = SymbolMatrixCreate(array, 1, 2);                                                                 // (distance ** 2) / 2)
	SymbolMatrixSet(t6, 0, 0, SymbolNodeConstant(array->nodeArray, (distance * distance) / 2.0f));
	SymbolMatrixSet(t6, 0, 1, SymbolNodeConstant(array->nodeArray, (distance * distance) / 2.0f));
	SymbolMatrix* t7 = SymbolMatrixSubtract(array, t5, t6);                                                             // (x_1(t) - x_2(t)) ** 2 / 2 - (distance ** 2) / 2)
	SymbolNode* t8 = SymbolNodeBinary(array->nodeArray, ADD, SymbolMatrixGet(t7, 0, 0), SymbolMatrixGet(t7, 0, 1));     // sum((x_1(t) - x_2(t)) ** 2 / 2 - (distance ** 2) / 2))

	return t8;
}

Constraint* DistanceConstraintCreate(ConstraintArray* constraintsArray, SymbolMatrixArray* symbolMatrixArray,
                                     ParticleArray* particlesArray, float distance) {
	assert(particlesArray->size == 2, "Circle constraint has incorrect number of particles!");

	SymbolNode* t = SymbolNodeVariable(symbolMatrixArray->nodeArray);
	SymbolMatrix* x = SymbolMatrixCreate(symbolMatrixArray, 2, 2);
	SymbolMatrixSet(x, 0, 0, SymbolNodeVariable(symbolMatrixArray->nodeArray));
	SymbolMatrixSet(x, 0, 1, SymbolNodeVariable(symbolMatrixArray->nodeArray));
	SymbolMatrixSet(x, 1, 0, SymbolNodeVariable(symbolMatrixArray->nodeArray));
	SymbolMatrixSet(x, 1, 1, SymbolNodeVariable(symbolMatrixArray->nodeArray));
	SymbolMatrix* v = SymbolMatrixCreate(symbolMatrixArray, 2, 2);
	SymbolMatrixSet(v, 0, 0, SymbolNodeVariable(symbolMatrixArray->nodeArray));
	SymbolMatrixSet(v, 0, 1, SymbolNodeVariable(symbolMatrixArray->nodeArray));
	SymbolMatrixSet(v, 1, 0, SymbolNodeVariable(symbolMatrixArray->nodeArray));
	SymbolMatrixSet(v, 1, 1, SymbolNodeVariable(symbolMatrixArray->nodeArray));
	SymbolMatrix* a = SymbolMatrixCreate(symbolMatrixArray, 2, 2);
	SymbolMatrixSet(a, 0, 0, SymbolNodeVariable(symbolMatrixArray->nodeArray));
	SymbolMatrixSet(a, 0, 1, SymbolNodeVariable(symbolMatrixArray->nodeArray));
	SymbolMatrixSet(a, 1, 0, SymbolNodeVariable(symbolMatrixArray->nodeArray));
	SymbolMatrixSet(a, 1, 1, SymbolNodeVariable(symbolMatrixArray->nodeArray));

	SymbolNode* f = DistanceConstraintFunction(symbolMatrixArray, t, x, v, a, distance);
	SymbolNode* df_dt = SymbolNodeDifferentiate(f, symbolMatrixArray->nodeArray, t);
	SymbolMatrix* df_dx = SymbolNodeDifferentiateSymbolMatrix(f, symbolMatrixArray, x);
	SymbolMatrix* df_dxdt = SymbolMatrixDifferentiateSymbolNode(df_dx, symbolMatrixArray, t);

	Constraint* constraint = ConstraintCreate(constraintsArray, particlesArray, DISTANCE, t, x, v, a, f, df_dt, df_dx,
											  df_dxdt);
	constraint->metadata.distance.distance = distance;
	return constraint;
}

void ConstraintDraw(Constraint* constraint) {
	switch (constraint->type) {
		case CIRCLE:
			const Vector2 center = constraint->metadata.circle.center;
			const Vector2 radius = constraint->metadata.circle.radius;

			DrawEllipseLines(iroundf(center.x), iroundf(center.y), radius.x, radius.y, LIGHTGRAY);
			break;
		case DISTANCE:
			assert(constraint->particles->size == 2, "Circle constraint has incorrect number of particles!");

			Particle* particle1 = constraint->particles->start[0];
			Particle* particle2 = constraint->particles->start[1];
			DrawLine(iroundf(particle1->x.x), iroundf(particle1->x.y), iroundf(particle2->x.x), iroundf(particle2->x.y),
			         LIGHTGRAY);
			break;
		default:
			assert(false, "Draw not implemented for constraint!");
	}


}
