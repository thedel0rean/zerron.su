#include "../Hooks.hpp"
#include "../Features/Visuals/Chams.hpp"

void __fastcall C_Hooks::hkDrawModelExecute(LPVOID pEcx, uint32_t, LPVOID pResults, DrawModelInfo_t* pInfo, matrix3x4_t* pBoneToWorld, float_t* flpFlexWeights, float_t* flpFlexDelayedWeights, Vector& vrModelOrigin, int32_t iFlags)
{
	if (g_Globals.m_Model.m_bDrawModel && pInfo->m_pStudioHdr)
	{
		if (strstr(pInfo->m_pStudioHdr->szName, _S("weapons\\w_pist_elite.mdl")))
		{
			g_Globals.m_Interfaces.m_StudioRender->ForcedMaterialOverride(NULL);
			return g_Globals.m_Hooks.m_Originals.m_DrawModelExecute(g_Globals.m_Interfaces.m_StudioRender, pResults, pInfo, pBoneToWorld, flpFlexWeights, flpFlexDelayedWeights, vrModelOrigin, iFlags);
		}
	}

	if (strstr(pInfo->m_pStudioHdr->szName, _S("shadow")) != nullptr)
		return;

	if (g_Globals.m_Interfaces.m_StudioRender->IsForcedMaterialOverride())
		return g_Globals.m_Hooks.m_Originals.m_DrawModelExecute(g_Globals.m_Interfaces.m_StudioRender, pResults, pInfo, pBoneToWorld, flpFlexWeights, flpFlexDelayedWeights, vrModelOrigin, iFlags);

	if (!pInfo->m_pClientEntity)
		return g_Globals.m_Hooks.m_Originals.m_DrawModelExecute(g_Globals.m_Interfaces.m_StudioRender, pResults, pInfo, pBoneToWorld, flpFlexWeights, flpFlexDelayedWeights, vrModelOrigin, iFlags);

	return g_Chams->OnModelRender(pResults, pInfo, pBoneToWorld, flpFlexWeights, flpFlexDelayedWeights, vrModelOrigin, iFlags);
}