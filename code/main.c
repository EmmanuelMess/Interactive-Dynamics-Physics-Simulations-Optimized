#include <raylib.h>
#include <stdio.h>

#include "simulator.h"
#include "constraint_type.h"
#include "math.h"
#include "cases.h"

#define NDEBUG 1

int main(void) {
	// Initialization
	//--------------------------------------------------------------------------------------
	SetTraceLogLevel(LOG_ALL);

	const int screenWidth = 450;
	const int screenHeight = screenWidth;

	InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

	SymbolMatrixArray* symbolMatrixArray = SymbolMatrixArrayCreate();
	ParticleArray* allParticlesArray = ParticleArrayCreate();
	ConstraintArray* allConstraintsArray = ConstraintArrayCreate();

	Simulator simulator = case3(symbolMatrixArray, allParticlesArray, allConstraintsArray);

	const int FONT_SIZE = 11;

	SetTargetFPS(0);
	//--------------------------------------------------------------------------------------

	while (!WindowShouldClose()) { // Detect window close button or ESC key
		// Update
		//----------------------------------------------------------------------------------

		const double updateTimeStartMs = GetTime() * 1000;

		SimulatorUpdate(&simulator, 0.000001f);

		const double updateTimeEndMs = GetTime() * 1000;

		//----------------------------------------------------------------------------------

		// Draw
		//----------------------------------------------------------------------------------
		BeginDrawing();

		ClearBackground(RAYWHITE);

		DrawText(TextFormat("%2i FPS", GetFPS()), screenWidth - 50, 0, 10, BLACK);

		DrawText(TextFormat("t %fs", simulator.time), 5, 5+0*15, FONT_SIZE, BLACK);
		DrawText(TextFormat("error %f", simulator.error), 5, 5+1*15, FONT_SIZE, BLACK);
		DrawText(TextFormat("ΔT %.3fms", updateTimeEndMs - updateTimeStartMs), 5, 5+2*15, FONT_SIZE, BLACK);

		for (unsigned int i = 0; i < allConstraintsArray->size; ++i) {
			Constraint * constraint = allConstraintsArray->start[i];
			ConstraintDraw(constraint);
		}

		for(unsigned int i = 0; i < allParticlesArray->size; i++) {
			Particle *particle = allParticlesArray->start[i];
			const char * text = TextFormat("p %u\n  x [%-.6F %.6F]\n  v [%-.6F %.6F]\n  a [%-.6F %.6F]",
								i, particle->x.x, particle->x.y, particle->v.x, particle->v.y, particle->a.x, particle->a.y);
			DrawText(text, 5, 5+3*15+i*4*15, FONT_SIZE, BLACK);

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
	SymbolMatrixArrayFree(symbolMatrixArray);
	ParticleArrayFree(allParticlesArray);
	ConstraintArrayFree(allConstraintsArray);

	CloseWindow();
	//--------------------------------------------------------------------------------------

	return 0;
}
