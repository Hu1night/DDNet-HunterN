/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include "laserindicator.h"
#include <game/generated/protocol.h>
#include <game/generated/server_data.h>
#include <game/server/gamecontext.h>

#include <engine/shared/config.h>
#include <game/server/teams.h>

#include "character.h"

CLaserIndicator::CLaserIndicator(
	CGameWorld *pGameWorld,
	int Owner,
	vec2 From,
	vec2 To,
	int Fineness) :
	CEntity(pGameWorld, CGameWorld::ENTTYPE_LASER)
{
	m_Owner = Owner;
	m_From = From;
	m_Pos = To;
	m_Fineness = Fineness;

	m_ID = Server()->SnapNewID();

	GameWorld()->InsertEntity(this);
}

CLaserIndicator::~CLaserIndicator()
{
	Server()->SnapFreeID(m_ID);
}

void CLaserIndicator::Reset()
{
	m_MarkedForDestroy = true;
}

void CLaserIndicator::Tick()
{
	CCharacter *pOwnerChar = GameServer()->GetPlayerChar(m_Owner);
	CPlayer *pOwnerPlayer = GameServer()->m_apPlayers[m_Owner];
	if((!pOwnerChar || !pOwnerChar->IsAlive()) && m_Owner >= 0)
	{
		m_MarkedForDestroy = true;
		return;
	}

	if(!pOwnerPlayer || GameServer()->GetDDRaceTeam(m_Owner) != GameWorld()->Team())
	{
		m_MarkedForDestroy = true; // owner has gone to another reality.
		return;
	}
}

bool CLaserIndicator::NetworkClipped(int SnappingClient)
{
	return NetworkLineClipped(SnappingClient, m_From, m_Pos);
}

void CLaserIndicator::Snap(int SnappingClient, int OtherMode)
{
	CNetObj_Laser *pObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_ID, sizeof(CNetObj_Laser)));
	if(!pObj)
		return;

	pObj->m_X = (int)m_Pos.x;
	pObj->m_Y = (int)m_Pos.y;
	pObj->m_FromX = (int)m_From.x;
	pObj->m_FromY = (int)m_From.y;
	pObj->m_StartTick = Server()->Tick() - m_Fineness;
}

void CLaserIndicator::Destroy()
{
	CEntity::Destroy();
}