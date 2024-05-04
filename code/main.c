#include <raylib.h>
#include <stdio.h>

#include "simulator.h"
#include "constraint_type.h"
#include "math.h"

#define NDEBUG 1

int main(void) {
	// Initialization
	//--------------------------------------------------------------------------------------
	SetTraceLogLevel(LOG_ALL);

	const int screenWidth = 800;
	const int screenHeight = 450;

	InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

	SymbolMatrixArray* symbolMatrixArray = SymbolMatrixArrayCreate();
	MatrixNArray* matrixNArray = MatrixNArrayCreate();
	ParticleArray* allParticlesArray = ParticleArrayCreate();
	ConstraintArray* allConstraintsArray = ConstraintArrayCreate();

	Particle* particle1 = ParticleCreate(allParticlesArray);
	*particle1 = (Particle) {
		.x = (Vector2) { 320.0f, 320.0f },
		.v = Vector2Zero(),
		.a = Vector2Zero(),
		.aApplied = Vector2Zero(),
		.aConstraint = Vector2Zero(),
		.isStatic = false,
	};

	Particle* particle2 = ParticleCreate(allParticlesArray);
	*particle2 = (Particle) {
		.x = (Vector2) { 40.0f, 120.0f },
		.v = Vector2Zero(),
		.a = Vector2Zero(),
		.aApplied = Vector2Zero(),
		.aConstraint = Vector2Zero(),
		.isStatic = false,
	};

	//const Vector2 center1 = (Vector2) { .x = 200.0f, .y = 200.0f };
	//const Vector2 radius1 = (Vector2) { .x = 50.0f, .y = 50.0f };
	//CircleConstraintCreate(allConstraintsArray, symbolMatrixArray, ParticleArrayOf(1, particle1), center1, radius1);

	//const Vector2 center2 = (Vector2) { .x = 250.0f, .y = 200.0f };
	//const Vector2 radius2 = (Vector2) { .x = 50.0f, .y = 50.0f };
	//CircleConstraintCreate(allConstraintsArray, symbolMatrixArray, ParticleArrayOf(1, particle1), center2, radius2);

	const float distance = 10.0f;
	DistanceConstraintCreate(allConstraintsArray, symbolMatrixArray, ParticleArrayOf(2, particle1, particle2), distance);

	Simulator simulator = SimulatorCreate(allParticlesArray, allConstraintsArray, true);

	const int FONT_SIZE = 11;

	SetTargetFPS(0);
	//--------------------------------------------------------------------------------------

	while (!WindowShouldClose()) { // Detect window close button or ESC key
		// Update
		//----------------------------------------------------------------------------------

		const double updateTimeStartMs = GetTime() * 1000;

		SimulatorUpdate(&simulator, 0.0001f);

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

		//DrawEllipseLines(iroundf(center1.x), iroundf(center1.y), radius1.x, radius1.y, LIGHTGRAY);
		//DrawEllipseLines(iroundf(center2.x), iroundf(center2.y), radius2.x, radius2.y, LIGHTGRAY);
		DrawLine(iroundf(particle1->x.x), iroundf(particle1->x.y), iroundf(particle2->x.x), iroundf(particle2->x.y), LIGHTGRAY);

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
	MatrixNArrayFree(matrixNArray);
	ParticleArrayFree(allParticlesArray);
	ConstraintArrayFree(allConstraintsArray);

	CloseWindow();
	//--------------------------------------------------------------------------------------

	return 0;
}
