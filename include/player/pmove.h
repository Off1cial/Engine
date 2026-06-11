#ifndef PMOVE_H
#define PMOVE_H

void PlayerLook(int player, float xrel, float yrel);
void PlayerMove(int player, float wishspeed, float accel, float friction, float gravity, float jump_velocity, float dt);




#endif
