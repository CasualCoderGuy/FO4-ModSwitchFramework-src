#pragma once
#include "MSF_Shared.h"

struct StaticFunctionTag;
class VirtualMachine;


namespace MSF_Papyrus
{
	void RegisterFuncs(VirtualMachine* vm);
	bool RegisterPapyrus(VirtualMachine * vm);
}