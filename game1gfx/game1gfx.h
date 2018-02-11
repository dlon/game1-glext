// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the GAME1GFX_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// GAME1GFX_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef GAME1GFX_EXPORTS
#define GAME1GFX_API __declspec(dllexport)
#else
#define GAME1GFX_API __declspec(dllimport)
#endif

// This class is exported from the game1gfx.dll
class GAME1GFX_API Cgame1gfx {
public:
	Cgame1gfx(void);
	// TODO: add your methods here.
};

extern GAME1GFX_API int ngame1gfx;

GAME1GFX_API int fngame1gfx(void);
