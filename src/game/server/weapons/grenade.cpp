#include "grenade.h"
#include <game/generated/server_data.h>
#include <game/server/entities/projectile.h>

CGrenade::CGrenade(CCharacter *pOwnerChar) :
	CWeapon(pOwnerChar)
{
	m_MaxAmmo = g_pData->m_Weapons.m_aId[WEAPON_GRENADE].m_Maxammo;
	m_AmmoRegenTime = g_pData->m_Weapons.m_aId[WEAPON_GRENADE].m_Ammoregentime;
	m_FireDelay = g_pData->m_Weapons.m_aId[WEAPON_GRENADE].m_Firedelay;
	m_FullAuto = true;
}
/* Hunter Start */
bool CGrenade::FragCollide(CProjectile *pProj, vec2 Pos, CCharacter *pHit, bool EndOfLife)
{
	if(pHit)
	{
		if(pHit->GetPlayer()->GetCID() == pProj->GetOwner())
			return false;

		pHit->TakeDamage(vec2(0, 0), g_pData->m_Weapons.m_Shotgun.m_pBase->m_Damage, pProj->GetOwner(), WEAPON_GRENADE, pProj->GetWeaponID(), false);
	}

	return true;
}
/* Hunter End */

bool CGrenade::GrenadeCollide(CProjectile *pProj, vec2 Pos, CCharacter *pHit, bool EndOfLife)
{
	if(pHit && pHit->GetPlayer()->GetCID() == pProj->GetOwner())
		return false;

	pProj->GameWorld()->CreateSound(Pos, SOUND_GRENADE_EXPLODE);
	pProj->GameWorld()->CreateExplosion(Pos, pProj->GetOwner(), WEAPON_GRENADE, pProj->GetWeaponID(), g_pData->m_Weapons.m_aId[WEAPON_GRENADE].m_Damage, pProj->GetOwner() < 0);

	/* Hunter Start */
	if(pProj->GameServer()->m_apPlayers[pProj->GetOwner()]->m_UseHunterWeapon)
	{
		float a = (rand()%314)/5.0;
		vec2 ParticleDir = vec2(cosf(a), sinf(a)) * 80;

		pProj->GameWorld()->CreateExplosionParticle(Pos + ParticleDir); // vec2(d.x, d.y) // Create Particle
		pProj->GameWorld()->CreateExplosionParticle(Pos + vec2(ParticleDir.y, -ParticleDir.x));
		pProj->GameWorld()->CreateExplosionParticle(Pos + vec2(-ParticleDir.x, -ParticleDir.y));
		pProj->GameWorld()->CreateExplosionParticle(Pos + vec2(-ParticleDir.y, ParticleDir.x));

		vec2 DirChr; // 指向玩家
		vec2 FragPos; // 破片生成的位置
		CCharacter* ClosestCharacter = 0; // 最近的其他的玩家的角色的指针

		if(!pHit && pProj->Controller()->m_HuntFragTrack) // 打得好 给了他们狠狠一击!
		{
			FragPos = pProj->GetPos((pProj->Server()->Tick() - pProj->GetStartTick() - 1) / (float)pProj->Server()->TickSpeed()); // 上一刻位置 让破片不至于埋土里

			ClosestCharacter = (CCharacter *)pProj->GameWorld()->ClosestEntity(FragPos, 448.f, CGameWorld::ENTTYPE_CHARACTER, pProj->GameServer()->m_apPlayers[pProj->GetOwner()]->GetCharacter());
			if(ClosestCharacter) // 如果14格内没有其他玩家
				DirChr = normalize(ClosestCharacter->m_Pos - FragPos); // 指向玩家
		}
		else
			FragPos = Pos;

		CMsgPacker Msg(NETMSGTYPE_SV_EXTRAPROJECTILE);
		Msg.AddInt(pProj->Controller()->m_HuntFragNum);

		for(int i = 0; i < pProj->Controller()->m_HuntFragNum; i++) // Create Fragments
		{
			float a = (rand()%314)/5.f;
			vec2 d = vec2(cosf(a), sinf(a)); // 随机指向
			if(ClosestCharacter)
				d = normalize((d * 0.5f) + DirChr); // 飞向玩家

			CProjectile *pProjFrag = new CProjectile(
				pProj->GameWorld(),
				WEAPON_SHOTGUN, //Type
				pProj->GetWeaponID(), //WeaponID
				pProj->GetOwner(), //Owner
				FragPos + d, //Pos
				d * 0.5f, //Dir
				6.0f, // Radius
				0.2f * pProj->Server()->TickSpeed(), //Span
				FragCollide);
			
			// pack the Projectile and send it to the client Directly
			CNetObj_Projectile p;
			pProjFrag->FillInfo(&p);

			for(unsigned i = 0; i < sizeof(CNetObj_Projectile) / sizeof(int); i++)
				Msg.AddInt(((int *)&p)[i]);
		}
	}
	/* Hunter End */

	return true;
}

void CGrenade::Fire(vec2 Direction)
{
	int ClientID = Character()->GetPlayer()->GetCID();
	int Lifetime = Character()->CurrentTuning()->m_GrenadeLifetime * Server()->TickSpeed();

	vec2 ProjStartPos = Pos() + Direction * GetProximityRadius() * 0.75f;

	CProjectile *pProj = new CProjectile(
		GameWorld(),
		WEAPON_GRENADE, //Type
		GetWeaponID(), //WeaponID
		ClientID, //Owner
		ProjStartPos, //Pos
		Direction, //Dir
		6.0f, // Radius
		Lifetime, //Span
		GrenadeCollide);

	// pack the Projectile and send it to the client Directly
	CNetObj_Projectile p;
	pProj->FillInfo(&p);

	CMsgPacker Msg(NETMSGTYPE_SV_EXTRAPROJECTILE);
	Msg.AddInt(1);
	for(unsigned i = 0; i < sizeof(CNetObj_Projectile) / sizeof(int); i++)
		Msg.AddInt(((int *)&p)[i]);

	Server()->SendMsg(&Msg, MSGFLAG_VITAL, ClientID);
	GameWorld()->CreateSound(Character()->m_Pos, SOUND_GRENADE_FIRE);
}