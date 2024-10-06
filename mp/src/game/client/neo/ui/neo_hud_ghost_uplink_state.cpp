#include "cbase.h"
#include "neo_hud_ghost_uplink_state.h"

#include "iclientmode.h"
#include <vgui/ISurface.h>

#include "c_neo_player.h"
#include "weapon_ghost.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using vgui::surface;

DECLARE_NAMED_HUDELEMENT(CNEOHud_GhostUplinkState, neo_ghost_uplink_state);

NEO_HUD_ELEMENT_DECLARE_FREQ_CVAR(GhostUplinkState, 0.1)

CNEOHud_GhostUplinkState::CNEOHud_GhostUplinkState(const char *pElementName, vgui::Panel *parent)
	: CHudElement(pElementName), Panel(parent, pElementName)
{
	SetAutoDelete(true);

	SetScheme("ClientScheme.res");

	if (parent) {
		SetParent(parent);
	}
	else
	{
		SetParent(g_pClientMode->GetViewport());
	}

	for (int i = 0; i < NUM_UPLINK_TEXTURES; i++)
	{
		m_pUplinkTextures[i] = surface()->CreateNewTextureID();
		Assert(m_pUplinkTextures[i] > 0);
		char textureFilePath[24];
		V_sprintf_safe(textureFilePath, "%s%i", "vgui/hud/ctg/g_gscan_0", i);
		surface()->DrawSetTextureFile(m_pUplinkTextures[i], textureFilePath, 1, false);
	}

	surface()->DrawGetTextureSize(m_pUplinkTextures[0], m_iUplinkTextureWidth, m_iUplinkTextureHeight);

	SetVisible(true);
}

void CNEOHud_GhostUplinkState::ApplySchemeSettings(vgui::IScheme* pScheme)
{
	int screenWidth, screenHeight;
	surface()->GetScreenSize(screenWidth, screenHeight);
	int centerX = screenWidth / 2;
	int textureXPos = centerX - (m_iUplinkTextureWidth / 2);
	//NEOJANK >=1440p resolutions have the uplink element clip into compass (Bryson)
	static int COMPASS_HEIGHT_PLUS_MARGINS = 30;
	if (screenHeight > 1440)
		COMPASS_HEIGHT_PLUS_MARGINS = 45;
	int textureYPos = screenHeight - m_iUplinkTextureHeight - COMPASS_HEIGHT_PLUS_MARGINS;
	SetBounds(textureXPos, textureYPos, m_iUplinkTextureWidth, m_iUplinkTextureHeight);
	SetFgColor(COLOR_TRANSPARENT);
	SetBgColor(COLOR_TRANSPARENT);

	BaseClass::ApplySchemeSettings(pScheme);
}

void CNEOHud_GhostUplinkState::UpdateStateForNeoHudElementDraw()
{
	if (m_iCurrentTextureIndex < (NUM_TEXTURE_START_TIMES - 1))
	{
		static constexpr float textureStartTimes[NUM_TEXTURE_START_TIMES] = { 0, 0.2, 0.5, 0.6, 1.0, 1.1, 1.4, 1.8, 2.5, 2.7, 3.0 };
		if ((gpGlobals->curtime - m_flTimeGhostEquip) >= textureStartTimes[m_iCurrentTextureIndex + 1])
		{
			m_iCurrentTextureIndex++;
		}
	}
}

void CNEOHud_GhostUplinkState::DrawNeoHudElement()
{
	if (!ShouldDraw())
	{
		return;
	}

	auto localPlayer = C_NEO_Player::GetLocalNEOPlayer();
	const auto wep = static_cast<C_NEOBaseCombatWeapon*>(localPlayer->GetActiveWeapon());
	if (!wep || !wep->IsGhost())
	{
		m_flTimeGhostEquip = 0.f;
		m_iCurrentTextureIndex = 0;
		return;
	}

	if (m_flTimeGhostEquip == 0.f)
	{
		m_flTimeGhostEquip = gpGlobals->curtime;
	}

	surface()->DrawSetColor(COLOR_RED);
	static constexpr int textureOrder[NUM_TEXTURE_START_TIMES] = { 0, 1, 2, 3, 2, 0, 1, 2, 3, 0, 1 };
	surface()->DrawSetTexture(m_pUplinkTextures[textureOrder[m_iCurrentTextureIndex]]);
	surface()->DrawTexturedRect(0, 0, m_iUplinkTextureWidth, m_iUplinkTextureHeight);
}

void CNEOHud_GhostUplinkState::Paint()
{
	BaseClass::Paint();
	PaintNeoElement();
}