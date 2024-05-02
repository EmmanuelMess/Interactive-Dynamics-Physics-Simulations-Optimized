#undef SUPPORT_MODULE_RSHAPES
#undef SUPPORT_MODULE_RTEXTURES
#undef SUPPORT_MODULE_RMODELS
#undef SUPPORT_MODULE_RAUDIO

#include <raylib.h>
#include <stdio.h>

#include "simulator.h"
#include "math.h"

int main(void) {
	// Initialization
	//--------------------------------------------------------------------------------------
	SetTraceLogLevel(LOG_ALL);

	const int screenWidth = 800;
	const int screenHeight = 450;

	InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

	SymbolNodeArray* symbolNodeArray = SymbolNodeArrayCreate();
	SymbolMatrixArray* symbolMatrixArray = SymbolMatrixArrayCreate(symbolNodeArray);
	MatrixNArray* matrixNArray = MatrixNArrayCreate();
	ParticleArray* particleArray = ParticleArrayCreate();
	ConstraintArray* constraintArray = ConstraintArrayCreate();

	SymbolNode* t = SymbolNodeVariable(symbolNodeArray);
	SymbolMatrix* x = SymbolMatrixCreate(symbolMatrixArray, 1, 2);
	SymbolMatrixSet(x, 0, 0, SymbolNodeVariable(symbolNodeArray));
	SymbolMatrixSet(x, 0, 1, SymbolNodeVariable(symbolNodeArray));
	SymbolMatrix* v = SymbolMatrixCreate(symbolMatrixArray, 1, 2);
	SymbolMatrixSet(v, 0, 0, SymbolNodeVariable(symbolNodeArray));
	SymbolMatrixSet(v, 0, 1, SymbolNodeVariable(symbolNodeArray));
	SymbolMatrix* a = SymbolMatrixCreate(symbolMatrixArray, 1, 2);
	SymbolMatrixSet(a, 0, 0, SymbolNodeVariable(symbolNodeArray));
	SymbolMatrixSet(a, 0, 1, SymbolNodeVariable(symbolNodeArray));

	const Vector2 center = (Vector2) { .x = 200.0f, .y = 200.0f };
	const Vector2 radius = (Vector2) { .x = 100.0f, .y = 100.0f };

	SymbolNode* f = constraintCircle(symbolMatrixArray, t, x, v, a, center, radius);
	SymbolNode* df_dt = SymbolNodeDifferentiate(f, symbolNodeArray, t);
	SymbolMatrix* df_dx = SymbolNodeDifferentiateSymbolMatrix(f, symbolMatrixArray, x);
	SymbolMatrix* df_dxdt = SymbolMatrixDifferentiateSymbolNode(df_dx, symbolMatrixArray, t);

	*ParticleCreate(particleArray) = (Particle) {
		.x = (Vector2) { 30.0f, 120.0f },
		.v = Vector2Zero(),
		.a = Vector2Zero(),
		.aApplied = Vector2Zero(),
		.aConstraint = Vector2Zero(),
		.isStatic = false,
	};

	*ConstraintCreate(constraintArray) = (Constraint) {
		.particles = particleArray,
		.t = t,
		.x = x,
		.v = v,
		.a = a,
		.constraintFunction = f,
		.constraintFunction_dt = df_dt,
		.constraintFunction_dx = df_dx,
		.constraintFunction_dxdt = df_dxdt,
	};

	Simulator simulator = SimulatorCreate(symbolNodeArray, symbolMatrixArray, matrixNArray, particleArray,
										  constraintArray, true);

	const int FONT_SIZE = 11;

	SetTargetFPS(60);
	//--------------------------------------------------------------------------------------

	while (!WindowShouldClose()) { // Detect window close button or ESC key
		// Update
		//----------------------------------------------------------------------------------

		SimulatorUpdate(&simulator, 0.00005f);

		//----------------------------------------------------------------------------------

		// Draw
		//----------------------------------------------------------------------------------
		BeginDrawing();

		ClearBackground(RAYWHITE);

		DrawText(TextFormat("t %fs", simulator.time), 5, 5+0*15, FONT_SIZE, BLACK);
		DrawText(TextFormat("error %f", simulator.error), 5, 5+1*15, FONT_SIZE, BLACK);

		DrawEllipseLines(iroundf(center.x), iroundf(center.y), radius.x, radius.y, LIGHTGRAY);

		for(unsigned int i = 0; i < particleArray->last; i++) {
			Particle *particle = particleArray->start[i];
			const char * text = TextFormat("p %u\n  x [%-.6F %.6F]\n  v [%-.6F %.6F]\n  a [%-.6F %.6F]",
								i, particle->x.x, particle->x.y, particle->v.x, particle->v.y, particle->a.x, particle->a.y);
			DrawText(text, 5, 5+2*15, FONT_SIZE, BLACK);

			DrawCircle(iroundf(particle->x.x), iroundf(particle->x.y), 4, particle->isStatic? RED:BLUE);
			DrawLine(iroundf(particle->x.x), iroundf(particle->x.y),
			         iroundf(particle->x.x + particle->aApplied.x),
			         iroundf(particle->x.y + particle->aApplied.y),
			         GREEN);
			DrawLine(iroundf(particle->x.x), iroundf(particle->x.y),
			         iroundf(particle->x.x + particle->aConstraint.x),
					 iroundf(particle->x.y + particle->aConstraint.y),
					 RED);
		}

		EndDrawing();
		//----------------------------------------------------------------------------------
	}

	// De-Initialization
	//--------------------------------------------------------------------------------------
	SymbolNodeArrayFree(symbolNodeArray);
	SymbolMatrixArrayFree(symbolMatrixArray);
	MatrixNArrayFree(matrixNArray);
	ParticleArrayFree(particleArray);
	ConstraintArrayFree(constraintArray);

	CloseWindow();
	//--------------------------------------------------------------------------------------

	return 0;
}
