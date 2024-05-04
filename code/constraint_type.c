#include "constraint_type.h"

SymbolMatrix* computePositionApproximation(SymbolMatrixArray* array, SymbolNode* t, SymbolNode* xx, SymbolNode* xy,
                                           SymbolNode* vx, SymbolNode* vy,  SymbolNode* ax, SymbolNode* ay) {
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

SymbolNode* constraintCircle(SymbolMatrixArray* array, SymbolNode* t, SymbolMatrix* x, SymbolMatrix* v, SymbolMatrix* a,
                             Vector2 center, Vector2 radius) {
	SymbolMatrix* positionParticle1 = computePositionApproximation(array, t,
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
	SymbolMatrix* t5 = SymbolMatrixCreate(array, 1, 2);                                                                 // -1 * (radius ** 2) / 2)
	SymbolMatrixSet(t5, 0, 0, SymbolNodeConstant(array->nodeArray, -(radius.x * radius.x) / 2.0f));
	SymbolMatrixSet(t5, 0, 1, SymbolNodeConstant(array->nodeArray, -(radius.y * radius.y) / 2.0f));
	SymbolMatrix* t6 = SymbolMatrixAdd(array, t4, t5);                                                                  // (x(t) - center) ** 2 / 2 + (-1 * (radius ** 2) / 2)
	SymbolNode* t7 = SymbolNodeBinary(array->nodeArray, ADD, SymbolMatrixGet(t6, 0, 0), SymbolMatrixGet(t6, 0, 1));     // sum((x(t) - center) ** 2 / 2 + (-1 * (radius ** 2) / 2))

	return t7;
}

Constraint* CircleConstraintCreate(ConstraintArray* constraintsArray, SymbolMatrixArray* symbolMatrixArray,
								   ParticleArray* particlesArray, Vector2 center, Vector2 radius) {
	if(particlesArray->size != 1) {
		TraceLog(LOG_FATAL, "Circle constraint has incorrect number of particles!");
	}

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

	SymbolNode* f = constraintCircle(symbolMatrixArray, t, x, v, a, center, radius);
	SymbolNode* df_dt = SymbolNodeDifferentiate(f, symbolMatrixArray->nodeArray, t);
	SymbolMatrix* df_dx = SymbolNodeDifferentiateSymbolMatrix(f, symbolMatrixArray, x);
	SymbolMatrix* df_dxdt = SymbolMatrixDifferentiateSymbolNode(df_dx, symbolMatrixArray, t);

	Constraint* constraint = ConstraintCreate(constraintsArray);
	*constraint = (Constraint) {
		.particles = particlesArray,
		.t = t,
		.x = x,
		.v = v,
		.a = a,
		.constraintFunction = f,
		.constraintFunction_dt = df_dt,
		.constraintFunction_dx = df_dx,
		.constraintFunction_dxdt = df_dxdt,
	};

	return constraint;
}

SymbolNode* ConstraintDistance(SymbolMatrixArray* array, SymbolNode* t, SymbolMatrix* x, SymbolMatrix* v,
							   SymbolMatrix* a, float distance) {
	SymbolMatrix* positionParticle1 = computePositionApproximation(array, t,
	                                                               SymbolMatrixGet(x, 0, 0), SymbolMatrixGet(x, 0, 1),
	                                                               SymbolMatrixGet(v, 0, 0), SymbolMatrixGet(v, 0, 1),
	                                                               SymbolMatrixGet(a, 0, 0), SymbolMatrixGet(a, 0, 1));
	SymbolMatrix* positionParticle2 = computePositionApproximation(array, t,
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

	SymbolMatrix* t3 = SymbolMatrixSubtract(array, t1, t2);                                                            // x_1(t) - x_2(t)
	SymbolMatrix* t4 = SymbolMatrixMultiplyElementWise(array, t3, t3);                                                  // (x_1(t) - x_2(t)) ** 2
	SymbolMatrix* t5 = SymbolMatrixMultiplyValue(array, t4, SymbolNodeConstant(array->nodeArray, 0.5f));                // (x_1(t) - x_2(t)) ** 2 / 2
	SymbolMatrix* t6 = SymbolMatrixCreate(array, 1, 2);                                                                 // -1 * (distance ** 2) / 2)
	SymbolMatrixSet(t6, 0, 0, SymbolNodeConstant(array->nodeArray, -(distance * distance) / 2.0f));
	SymbolMatrixSet(t6, 0, 1, SymbolNodeConstant(array->nodeArray, -(distance * distance) / 2.0f));
	SymbolMatrix* t7 = SymbolMatrixAdd(array, t5, t6);                                                                  // (x(t) - center) ** 2 / 2 + (-1 * (radius ** 2) / 2)
	SymbolNode* t8 = SymbolNodeBinary(array->nodeArray, ADD, SymbolMatrixGet(t7, 0, 0), SymbolMatrixGet(t7, 0, 1));     // sum((x(t) - center) ** 2 / 2 + (-1 * (radius ** 2) / 2))

	return t8;
}

Constraint* DistanceConstraintCreate(ConstraintArray* constraintsArray, SymbolMatrixArray* symbolMatrixArray,
                                     ParticleArray* particlesArray, float distance) {
	if(particlesArray->size != 2) {
		TraceLog(LOG_FATAL, "Distance constraint has incorrect number of particles!");
	}

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

	SymbolNode* f = ConstraintDistance(symbolMatrixArray, t, x, v, a, distance);
	SymbolNode* df_dt = SymbolNodeDifferentiate(f, symbolMatrixArray->nodeArray, t);
	SymbolMatrix* df_dx = SymbolNodeDifferentiateSymbolMatrix(f, symbolMatrixArray, x);
	SymbolMatrix* df_dxdt = SymbolMatrixDifferentiateSymbolNode(df_dx, symbolMatrixArray, t);

	Constraint* constraint = ConstraintCreate(constraintsArray);
	*constraint = (Constraint) {
		.particles = particlesArray,
		.t = t,
		.x = x,
		.v = v,
		.a = a,
		.constraintFunction = f,
		.constraintFunction_dt = df_dt,
		.constraintFunction_dx = df_dx,
		.constraintFunction_dxdt = df_dxdt,
	};

	return constraint;
}
