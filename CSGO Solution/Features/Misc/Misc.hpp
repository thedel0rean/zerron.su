#pragma once
#include "../../SDK/Globals.hpp"
#include "../../SDK/Includes.hpp"

class C_Misc
{
public:
	virtual void VelocityModiferFix();
private:

};
inline C_Misc* g_Misc = new C_Misc();