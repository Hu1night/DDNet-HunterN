/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_GAMEMODES_MULTIBOMB_H
#define GAME_SERVER_GAMEMODES_MULTIBOMB_H
#include <game/server/gamecontroller.h>

class CGameControllerMultiBomb : public IGameController
{
public:
	CGameControllerMultiBomb();

	// event
	void OnCharacterSpawn(class CCharacter *pChr) override;
	void OnWorldReset() override;
	bool OnEntity(int Index, vec2 Pos, int Layer, int Flags, int Number) override;
	int OnCharacterTakeDamage(class CCharacter *pChr, vec2 &Force, int &Dmg, int From, int WeaponType, int WeaponID, bool IsExplosion) override { return DAMAGE_NO_DAMAGE | DAMAGE_NO_INDICATOR; }
	void DoWincheckMatch() override {};
	int OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon) override;
private:
	int m_BoomerNum;
	int m_BoomerCID[];
};

#endif // GAME_SERVER_GAMEMODES_DM_H
