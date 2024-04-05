#ifndef GAME_SERVER_WEAPONS_ASMRHAMMER_H
#define GAME_SERVER_WEAPONS_ASMRHAMMER_H

#include <game/server/weapons/hammer.h>

class CASMRHammer : public CHammer
{
public:
	CASMRHammer(CCharacter *pOwnerChar);
	void Tick() override;
};

#endif // GAME_SERVER_WEAPONS_ASMRHAMMER_H
