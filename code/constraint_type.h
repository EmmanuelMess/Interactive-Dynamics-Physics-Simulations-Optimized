#ifndef SIMULATOR_CONSTRAINT_TYPE_H
#define SIMULATOR_CONSTRAINT_TYPE_H

#include "simulator.h"

//-----------------------------------------------------------------------------
// Constraint functions
//-----------------------------------------------------------------------------

Constraint* CircleConstraintCreate(ConstraintArray* constraintsArray, SymbolMatrixArray* symbolMatrixArray,
                                   ParticleArray* particlesArray, Vector2 center, Vector2 radius);

Constraint* DistanceConstraintCreate(ConstraintArray* constraintsArray, SymbolMatrixArray* symbolMatrixArray,
                                   ParticleArray* particlesArray, float distance);

void ConstraintDraw(Constraint* constraint);

#endif //SIMULATOR_CONSTRAINT_TYPE_H
