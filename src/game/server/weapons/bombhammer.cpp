#include "bombhammer.h"
#include <game/generated/server_data.h>
#include <game/server/entities/projectile.h>
#include <game/server/weapons.h>

CBombHammer::CBombHammer(CCharacter *pOwnerChar) :
	CWeapon(pOwnerChar)
{
	m_MaxAmmo = g_pData->m_Weapons.m_aId[WEAPON_HAMMER].m_Maxammo;
	m_AmmoRegenTime = g_pData->m_Weapons.m_aId[WEAPON_GUN].m_Ammoregentime;
	m_FireDelay = g_pData->m_Weapons.m_aId[WEAPON_HAMMER].m_Firedelay;
	m_AmmoRegenDelay = 380;

	m_Roundleft = 20; // 还有多少圈引线
	m_nextRoundtick = 1; // 下一圈引线要的tick
	m_IsActive = false;
	//m_PerRoundms = 1000;
}

void CBombHammer::Tick()
{
	CWeapon::Tick();

	if(!m_IsActive)
		return;
	
	if(!m_nextRoundtick) // 是否下一圈引线
	{
		if(!m_Roundleft) // 如果引线没了
		{
			GameWorld()->CreateExplosionParticle(Pos());
			GameWorld()->CreateSound(Pos(), SOUND_GRENADE_EXPLODE);
			Character()->Die(Character()->GetPlayer()->GetCID(), WEAPON_WORLD);
		}
		else // 还有引线
		{
			Character()->SetHealth(m_Roundleft); // Good Coding
			Character()->SetArmor(m_Roundleft - 10);
			GameWorld()->CreateSound(Pos(), SOUND_HOOK_NOATTACH);
			GameWorld()->CreateDamageInd(Pos(), 0, m_Roundleft);

			--m_Roundleft;
			m_nextRoundtick = Server()->TickSpeed() * m_PerRoundms / 1000; // 下一次计时
		}
	}
	else
		--m_nextRoundtick;
}

void CBombHammer::Fire(vec2 Direction)
{
	int ClientID = Character()->GetPlayer()->GetCID();
	GameWorld()->CreateSound(Pos(), SOUND_HAMMER_FIRE);

	GameServer()->Antibot()->OnHammerFire(ClientID);

	if(Character()->IsSolo() || Character()->m_Hit & CCharacter::DISABLE_HIT_HAMMER)
		return;

	vec2 HammerHitPos = Pos() + Direction * GetProximityRadius() * 0.75f;

	CCharacter *pChr = (CCharacter *)GameWorld()->ClosestEntity(HammerHitPos, GetProximityRadius() * 1.5f, CGameWorld::ENTTYPE_CHARACTER, Character());

	//if ((pChr == this) || GameServer()->Collision()->IntersectLine(ProjStartPos, pChr->m_Pos, NULL, NULL))
	if(!pChr || (pChr->IsAlive() && pChr->IsSolo()))
		return;

	// set his velocity to fast upward (for now)
	if(length(pChr->m_Pos - HammerHitPos) > 0.0f)
		GameWorld()->CreateHammerHit(pChr->m_Pos - normalize(pChr->m_Pos - HammerHitPos) * GetProximityRadius() * 0.5f);
	else
		GameWorld()->CreateHammerHit(HammerHitPos);

	vec2 Dir;
	if(length(pChr->m_Pos - Pos()) > 0.0f)
		Dir = normalize(pChr->m_Pos - Pos());
	else
		Dir = vec2(0.f, -1.f);

	float Strength = Character()->CurrentTuning()->m_HammerStrength;

	vec2 Temp = pChr->Core()->m_Vel + normalize(Dir + vec2(0.f, -1.1f)) * 10.0f;
	Temp = ClampVel(pChr->m_MoveRestrictions, Temp);
	Temp -= pChr->Core()->m_Vel;

	pChr->TakeDamage((vec2(0.f, -1.0f) + Temp) * Strength, g_pData->m_Weapons.m_Hammer.m_pBase->m_Damage, ClientID, WEAPON_HAMMER, GetWeaponID(), false);

	GameServer()->Antibot()->OnHammerHit(ClientID);

	float FireDelay;
	int TuneZone = Character()->m_TuneZone;
	if(!TuneZone)
		FireDelay = GameServer()->Tuning()->m_HammerHitFireDelay;
	else
		FireDelay = GameServer()->TuningList()[TuneZone].m_HammerHitFireDelay;
	m_ReloadTimer = FireDelay * Server()->TickSpeed() / 1000;

	CWeapon *pWeapon = pChr->GetPowerupWeapon();
	if(!pWeapon || pWeapon->GetWeaponID() != WEAPON_ID_BOMBHAMMER ||
		!(((CBombHammer *)Character()->GetPowerupWeapon())->m_IsActive) || ((CBombHammer *)pWeapon)->m_IsActive)
			return;
		
	pChr->SetHealth(m_Roundleft);
	pChr->SetArmor(m_Roundleft - 10);
	pChr->SetPowerUpWeapon(WEAPON_ID_BOMBHAMMER, 0); // HACK: 利用弹药再生避免因玩家tick顺序在1tick内传回来（CID 1 -> CID 2 -> CID 1）
	CBombHammer *pBomb = (CBombHammer *)pChr->GetPowerupWeapon();
	pBomb->m_Roundleft = m_Roundleft;
	pBomb->m_nextRoundtick = m_nextRoundtick;
	pBomb->m_IsActive = true;

	m_IsActive = false; // 关掉炸弹
	Character()->SetHealth(10);
	Character()->SetArmor(0);
}