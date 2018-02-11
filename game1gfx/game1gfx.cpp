// game1gfx.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "game1gfx.h"


// This is an example of an exported variable
GAME1GFX_API int ngame1gfx=0;

// This is an example of an exported function.
GAME1GFX_API int fngame1gfx(void)
{
    return 42;
}

// This is the constructor of a class that has been exported.
// see game1gfx.h for the class definition
Cgame1gfx::Cgame1gfx()
{
    return;
}
