/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_GAMEMODES_HUNTERC_H
#define GAME_SERVER_GAMEMODES_HUNTERC_H
#include <game/server/gamemodes/huntern.h>

class CGameControllerHunterC : public CGameControllerHunterN
{
public:
	CGameControllerHunterC();

	// event
	//virtual void OnCharacterSpawn(class CCharacter *pChr) override;

	void OnGameStart(bool IsRound) override;

	CFlag *m_pFlag;
};

#endif // GAME_SERVER_GAMEMODES_HUNTERC_H
