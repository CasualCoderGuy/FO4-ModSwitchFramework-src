#pragma once
#include "rva/RVA.h"

class DataHandler;

namespace Globals
{
    void Init();
    extern RVA<DataHandler*> dataHandler;
}