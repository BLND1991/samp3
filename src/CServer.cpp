/* =========================================

		FCNPC - Fully Controllable NPC
			----------------------

	- File: Server.cpp
	- Author(s): OrMisicL

  =========================================*/

#include "Main.h"

extern logprintf_t logprintf;
extern CNetGame *pNetGame;

CServer::CServer(eSAMPVersion version)
{
	m_iTicks = 0;
	m_iTickRate = 5;

	m_Version = version;
	// Reset instances
	m_pPlayerDataManager = NULL;
	m_pNodeManager = NULL;
	m_pDamageThread = NULL;
	m_pMapAndreas = NULL;
	// Initialize the update rate
	m_dwUpdateRate = DEFAULT_UPDATE_RATE;
}

CServer::~CServer()
{
	// Delete instance
	SAFE_DELETE(m_pPlayerDataManager);
	SAFE_DELETE(m_pNodeManager);
	SAFE_DELETE(m_pDamageThread);
	SAFE_DELETE(m_pMapAndreas);
}

BYTE CServer::Initialize()
{
	// Initialize necessary samp functions
	CFunctions::PreInitialize();
	// Initialize addresses
	CAddress::Initialize(CServer::GetVersion());
	// Initialize SAMP Functions
	CFunctions::Initialize();
	// Install hooks
	CHooks::InstallHooks();
	// Install patches
	CPatches::InstallPatches();
	// Create the player manager instance
	m_pPlayerDataManager = new CPlayerManager();
	if (!m_pPlayerDataManager) {
		return 2;
	}

	// Create the node manager instance
	m_pNodeManager = new CNodeManager();
	if (!m_pNodeManager) {
		return 3;
	}

	/*	// Create threads
		m_pDamageThread = new CThread(CThreadFunctions::DamageThread);
		if (!m_pDamageThread || !m_pDamageThread->Start())
			return 4;*/

	// Check the maxnpc from the config
	if (CFunctions::GetMaxNPC() == 0) {
		logprintf("Warning: the maxnpc limit is 0 (you will not be able to create NPCs unless you change it)");
	}
	// Check the maxnpc and maxplayers in the config
	else if (CFunctions::GetMaxPlayers() < CFunctions::GetMaxNPC()) {
		logprintf("Warning: the maxplayers limit is less than maxnpc (possible crash)");
	}

	return 0;
}

void CServer::Process()
{
	if (m_iTickRate == -1) {
		return;
	}

	if (++m_iTicks >= m_iTickRate) {
		m_iTicks = 0;
		// Process the player manager
		pServer->GetPlayerManager()->Process();
	}
}

bool CServer::DoesNameExist(char *szName)
{
	// Loop through all the players
	for (int i = 0; i < MAX_PLAYERS; i++) {
		// Ignore non connected players
		if (!pNetGame->pPlayerPool->bIsPlayerConnectedEx[i]) {
			continue;
		}

		// Compare names
		if (!strcmp(szName, pNetGame->pPlayerPool->szName[i])) {
			return true;
		}
	}
	return false;
}

void CServer::SetMapAndreas(CMapAndreas *pMapAndreas)
{
	m_pMapAndreas = pMapAndreas;
}

CMapAndreas *CServer::GetMapAndreas()
{
	return m_pMapAndreas;
}

bool CServer::IsMapAndreasInited()
{
	return m_pMapAndreas != NULL && m_pMapAndreas->IsInited();
}

bool CServer::IsVehicleSeatOccupied(int iPlayerId, WORD wVehicleId, BYTE byteSeatId)
{
	WORD wPlayerId = GetVehicleSeatPlayerId(wVehicleId, byteSeatId);

	if (wPlayerId != (WORD)iPlayerId && wPlayerId != INVALID_PLAYER_ID) {
		return true;
	}

	return false;
}

WORD CServer::GetVehicleSeatPlayerId(WORD wVehicleId, BYTE byteSeatId)
{
	if (wVehicleId < 1 || wVehicleId > MAX_VEHICLES) {
		return INVALID_PLAYER_ID;
	}

	CPlayerPool *pPlayerPool = pNetGame->pPlayerPool;
	CPlayer *pPlayer;

	// Loop through all the players
	for (int i = 0; i < MAX_PLAYERS; i++) {
		// Ignore non connected players and the same player
		if (!pPlayerPool->bIsPlayerConnectedEx[i]) {
			continue;
		}

		// Get the player interface
		pPlayer = pPlayerPool->pPlayer[i];

		// Check vehicle and seat
		if (pPlayer->wVehicleId == wVehicleId && pPlayer->byteSeatId == byteSeatId) {
			return pPlayer->wPlayerId;
		}
	}

	return INVALID_PLAYER_ID;
}
