//#define GAME_DLL
#define DEBUG
#define WIN32_LEAN_AND_MEAN
#define CLIENT_DLL

#pragma comment (lib, "mathlib.lib")
#pragma comment (lib, "psapi.lib" )
#pragma comment (lib, "User32.lib")
#pragma comment (linker, "/NODEFAULTLIB:LIBCMT")
#pragma warning (disable : 4099 4075 4996)


#include <string>
#include <sstream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <windows.h>
#include <Color.h>
#include <tlhelp32.h>
#include <psapi.h>

#include <cbase.h>
#include <eiface.h>
#include <convar.h>
#include <cdll_int.h>
#include <tier1/tier1.h>
#include <tier2/tier2.h>
#include <tier3/tier3.h>
#include <vstdlib\cvar.h>
#include <tier0/dbg.h>
#include <conio.h>
#include <winlite.h>
#include <usercmd.h>
#include <inetchannel.h>
#include <inetchannelinfo.h>
#include <bitbuf.h>
#include <cliententitylist.h>
#include <cdll_util.h>
#include <in_main.h>
#include <fx_line.h>
#include <input.h>
#include <igameresources.h>
#include <VGuiMatSurface\IMatSystemSurface.h>
#include <c_baseanimating.h>
#include <iclientrenderable.h>
#include "ivmodelrender.h"
#include "vfnhook.h"
#include <igameevents.h>
#include <iprediction.h>
#include <filesystem.h>
#include <inetmsghandler.h>
#include <iclient.h>

#define NET_MESSAGE_BITS 6

#define IN_ATTACK		(1<<0)
#define IN_JUMP			(1<<1)
#define IN_DUCK			(1<<2)
#define IN_FORWARD		(1<<3)
#define IN_BACK			(1<<4)
#define IN_USE			(1<<5)
#define IN_CANCEL		(1<<6)
#define IN_LEFT			(1<<7)
#define IN_RIGHT		(1<<8)
#define IN_MOVELEFT		(1<<9)
#define IN_MOVERIGHT		(1<<10)
#define IN_ATTACK2		(1<<11)
#define IN_RUN			(1<<12)
#define IN_RELOAD		(1<<13)
#define IN_ALT1			(1<<14)
#define IN_SCORE		(1<<15)

using namespace std;

inline void**& getvtable( void* inst, size_t offset = 0 )
{
	return *reinterpret_cast<void***>( (size_t)inst + offset );
}
inline const void** getvtable( const void* inst, size_t offset = 0 )
{
	return *reinterpret_cast<const void***>( (size_t)inst + offset );
}
template< typename Fn >
inline Fn getvfunc( const void* inst, size_t index, size_t offset = 0 )
{
	return reinterpret_cast<Fn>( getvtable( inst, offset )[ index ] );
}

#include "synthia_structs.h"
#include "synthia_utils.cpp"

