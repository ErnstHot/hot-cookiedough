
// cookiedough -- *very* basic gamepad support (uses the first device detected)

#pragma once

void Gamepad_Create();
void Gamepad_Destroy();

// call this on demand to poll the left and right analog sticks
// returns false (and zeroes) if there's nothing available, otherwise between -1 and 1 with dead zone taken care of
bool Gamepad_Update(float delta, float &leftX, float &leftY, float &rightX, float &rightY);
