/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_GAMEMODES_HUNTERN_H
#define GAME_SERVER_GAMEMODES_HUNTERN_H
#include <game/server/gamecontroller.h>
#include <game/server/classes.h>

class CGameControllerHunterN : public IGameController
{
private: // config
	int m_HunterRatio;
	//int m_BroadcastHunterList;
	int m_BroadcastHunterDeath;
	int m_EffectHunterDeath;
	int m_Wincheckdeley;
	int m_GameoverTime;
	//int m_RoundMode;

public: // Maprotation
	enum { MAX_MAPROTATIONS = 32, };
	// enum { MAX_MAPROTATIONS = 114514, };
	int m_aMaprotation[MAX_MAPROTATIONS] = {0}; // 存储MapIndex的数组

public:
	CGameControllerHunterN();

	static void OnResetClass(CCharacter *pChr);
	void CycleMap();

	// event
	void OnWorldReset() override;
	void OnCharacterSpawn(class CCharacter *pChr) override;
	void OnPlayerJoin(class CPlayer *pPlayer) override;
	int OnCharacterTakeDamage(class CCharacter *pChr, vec2 &Force, int &Dmg, int From, int WeaponType, int WeaponID, bool IsExplosion) override;
	//int OnPickup(CPickup *pPickup, CCharacter *pChar, SPickupSound *pSound) override;
	//bool CanDeadPlayerFreeView(const class CPlayer *pSpectator) override { return true; }
	//bool CanDeadPlayerFollow(const CPlayer *pSpectator, const CPlayer *pTarget) override { return true; }
	bool CanChangeTeam(CPlayer *pPlayer, int JoinTeam) const override;
	void DoWincheckRound() override;
	void DoWincheckMatch() override;
	int OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon) override;
	void OnPreEntitySnap(int SnappingClient, int OtherMode) override;

private: // Intelnal function and value
	int m_NumHunter; // 有多少个猎人
	int m_DoWinchenkClassTick; // 终局判断延迟的Tick
	char m_HunterList[256]; // 猎人列表
	//int TeamClass[1];
	//int MatchFlag = -1;

protected:
	enum { HUNTERN_GAMEFLAGS = IGF_SURVIVAL | IGF_ROUND_TIMER_ROUND | IGF_SUDDENDEATH | IGF_MARK_MATCH | IGF_MARK_AMONGUS, };
	enum HUNTERN_WINFLAG
	{
		FLAG_WIN_NONE = 0,
		FLAG_WIN_NO_ONE = 1,
		FLAG_WIN_TEAMRED = 2,
		FLAG_WIN_TEAMBLUE = 4,
	};

	const int m_aKillScore[NUM_CLASSES][2] = { // Kill & TeamKill Score
	{0, 0}, // CLASS_NONE
	{1, -1}, // CLASS_CIVIC
	{4, -2}, // CLASS_HUNTER
	{8, -114514}, // CLASS_JUGGERNAUT
	{4, -2},}; // CLASS_PUPPETEE

	const char m_aWeaponName[7][16] = {
	{"地刺"},
	{"锤子"},
	{"手枪"},
	{"霰弹"},
	{"榴弹"},
	{"激光"},
	{"忍者刀"},};
};

#endif