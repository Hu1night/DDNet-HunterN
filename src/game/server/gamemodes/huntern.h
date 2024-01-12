/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_GAMEMODES_HUNTERN_H
#define GAME_SERVER_GAMEMODES_HUNTERN_H
#include <game/server/gamecontroller.h>

class CGameControllerHunterN : public IGameController
{
private: // config
	int m_HunterRatio;
	int m_BroadcastHunterList;
	int m_BroadcastHunterDeath;
	int m_EffectHunterDeath;
	int m_Wincheckdeley;
	int m_GameoverTime;
	//int m_RoundMode;

public:
	CGameControllerHunterN();

	static void OnResetClass(CCharacter *pChr);
	void SendChatRoom(const char *pText, int Flags = 3);

	// event
	void OnCharacterSpawn(class CCharacter *pChr) override;
	void OnWorldReset() override;
	void OnPlayerJoin(class CPlayer *pPlayer) override;
	int OnCharacterTakeDamage(class CCharacter *pChr, vec2 &Force, int &Dmg, int From, int WeaponType, int WeaponID, bool IsExplosion) override;
	bool CanDeadPlayerFreeView(const class CPlayer *pSpectator) override { return true; }
	bool CanDeadPlayerFollow(const CPlayer *pSpectator, const CPlayer *pTarget) override { return true; }
	bool CanChangeTeam(CPlayer *pPlayer, int JoinTeam) const override;
	void DoWincheckRound() override;
	int OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon) override;

private: // Intelnal function and value
	static void OnClassSpawn(CCharacter *pChr);
	void EndRoundClass(int Flag);

	int nHunter; // 有多少个猎人
	int DoWinchenkClassTick; // 终局判断延迟的Tick
	char HunterList[256]; // 猎人列表
	int TeamClass[1];
	//int MatchFlag = -1;

	enum HUNTERN_WINFLAG
	{
		FLAG_WIN_NONE = 0,
		FLAG_WIN_NO_ONE = 1,
		FLAG_WIN_TEAMRED = 2,
		FLAG_WIN_TEAMBLUE = 4,
	};
};

#endif