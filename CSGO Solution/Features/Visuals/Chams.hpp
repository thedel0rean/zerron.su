#pragma once
#include "../SDK/Includes.hpp"
#include "../Settings.hpp"

class C_Chams
{
public:
	virtual void CreateMaterials();
	virtual void DrawModel(C_BaseEntity* pEntity, C_ChamsSettings Settings, LPVOID pResults, DrawModelInfo_t* pInfo, matrix3x4_t* aMatrix, float* flpFlexWeights, float* flpFlexDelayedWeights, Vector& vrModelOrigin, int iFlags, bool bForceNull, bool bXQZ);
	virtual void OnModelRender(LPVOID pResults, DrawModelInfo_t* pInfo, matrix3x4_t* aMatrix, float* flpFlexWeights, float* flpFlexDelayedWeights, Vector& vrModelOrigin, int iFlags);
	virtual void ApplyMaterial(int32_t iMaterial, bool bIgnoreZ, Color aColor, bool bCustom = false, bool bShotChams = false);
	virtual bool GetInterpolatedMatrix(C_BasePlayer* pPlayer, matrix3x4_t* aMatrix);
private:
	C_Material* m_pFlatMaterial = nullptr;
	C_Material* m_pGlowMaterial = nullptr;
	C_Material* m_pPulseMaterial = nullptr;
	C_Material* m_pRegularMaterial = nullptr;
	C_Material* m_pGlassMaterial = nullptr;
	C_Material* m_pGhostMaterial = nullptr;
	C_Material* m_pHaloMaterial = nullptr;
	C_Material* m_pSkyMaterial = nullptr;
};

inline C_Chams* g_Chams = new C_Chams();