#include <raylib.h>

#include "simulator.h"

int main(void) {
	// Initialization
	//--------------------------------------------------------------------------------------
	SetTraceLogLevel(LOG_ALL);

	const int screenWidth = 800;
	const int screenHeight = 450;

	InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");


	SymbolNodeArray array = SymbolNodeArrayCreate();
	SymbolMatrixArray matrixArray = SymbolMatrixArrayCreate(&array);

	SymbolNode* t = SymbolNodeVariable(&array);
	SymbolMatrix* x = SymbolMatrixCreate(&matrixArray, 2, 1);
	SymbolMatrixSetNode(x, 0, 0, SymbolNodeVariable(&array));
	SymbolMatrixSetNode(x, 1, 0, SymbolNodeVariable(&array));
	SymbolMatrix* v = SymbolMatrixCreate(&matrixArray, 2, 1);
	SymbolMatrixSetNode(v, 0, 0, SymbolNodeVariable(&array));
	SymbolMatrixSetNode(v, 1, 0, SymbolNodeVariable(&array));
	SymbolMatrix* a = SymbolMatrixCreate(&matrixArray, 2, 1);
	SymbolMatrixSetNode(a, 0, 0, SymbolNodeVariable(&array));
	SymbolMatrixSetNode(a, 1, 0, SymbolNodeVariable(&array));

	SymbolNode* f = constraintCircle(
		&matrixArray, t, x, v, a,
		(Vector2) { .x = 1.0f, .y = 1.0f },
		(Vector2) { .x = 2.0f, .y = 2.0f }
		);
	SymbolNode* df_dt = SymbolNodeDifferentiate(f, &array, t);
	SymbolMatrix* df_dx = SymbolNodeDifferentiateSymbolMatrix(f, &matrixArray, x);
	SymbolMatrix* df_dxdt = SymbolMatrixDifferentiateSymbolNode(df_dx, &matrixArray, t);

	SymbolNodePrint(f);
	SymbolNodePrint(df_dt);
	SymbolMatrixPrint(df_dx);
	SymbolMatrixPrint(df_dxdt);

	Constraint constraint = (Constraint) {
		.particles = {},
		.x = x,
		.v = v,
		.a = a,
		.constraintFunction = f,
		.constraintFunction_dt = df_dt,
		.constraintFunction_dx = df_dx,
		.constraintFunction_dxdt = df_dxdt,
	};


	SetTargetFPS(60);
	//--------------------------------------------------------------------------------------

	while (!WindowShouldClose()) { // Detect window close button or ESC key
		// Update
		//----------------------------------------------------------------------------------

		//----------------------------------------------------------------------------------

		// Draw
		//----------------------------------------------------------------------------------
		BeginDrawing();

		ClearBackground(RAYWHITE);

		DrawText("Congrats! You created your first window!", 190, 200, 20, LIGHTGRAY);

		EndDrawing();
		//----------------------------------------------------------------------------------
	}

	// De-Initialization
	//--------------------------------------------------------------------------------------
	SymbolNodeArrayFree(&array);
	SymbolMatrixArrayFree(&matrixArray);

	CloseWindow();
	//--------------------------------------------------------------------------------------

	return 0;
}
