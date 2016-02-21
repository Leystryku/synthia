void WriteLogFile(const char* szString)
{
 
  FILE* pFile = fopen("logFile.txt", "a");
  fprintf(pFile, "%s\n",szString);
  fclose(pFile);
 
}

std::string formatString( PCHAR fmt, ... )
{
	char buf[1024];
	memset( buf, 0, 1024 );

	va_list va_alist;
	va_start( va_alist, fmt );
	_vsnprintf( buf + strlen( buf ), sizeof( buf ) - strlen( buf ), fmt, va_alist );
	va_end( va_alist );

	return buf;
}

std::string getDirectory( HMODULE module )
{
	char path[255], drive[255], dir[255];
	
	GetModuleFileNameA( module, path, 260 );
	_splitpath( path, drive, dir, 0, 0 );

	return formatString( "%s%s", drive, dir );
}

std::wstring stringToWide( const std::string& text )
{
	std::wstring wide( text.length(), L' ' );
	std::copy( text.begin(), text.end(), wide.begin() );

	return wide;
}

const char *sourceChars = "abcdefghijklmnopqrstuvwxyz123456789";

int scLen = strlen( sourceChars );

char *randomString( int maxLen )
{
	int len = rand() % maxLen + 1;
	char *retVal = (char *) malloc( len + 1 );
	int i;

	for (i=0; i<len; ++i) 
		retVal[i] = sourceChars[ rand() % scLen ];

	retVal[len] = '\0';
	return retVal;
}

bool bDataCompare(const BYTE* pData, const BYTE* bMask, const char* szMask)
{
	for(;*szMask;++szMask,++pData,++bMask)
		if(*szMask=='x' && *pData!=*bMask )
			return false;
	return (*szMask) == NULL;
}
DWORD dwFindPattern(DWORD dwAddress,DWORD dwLen,BYTE *bMask,char * szMask)
{
	for(DWORD i=0; i < dwLen; i++)
	{
		if( bDataCompare( (BYTE*)( dwAddress+i ),bMask,szMask) )
			return (DWORD)(dwAddress+i);
	}
	return 0;
}

void* Synthia::Utils::Interfaces::get( CreateInterfaceFn* fn, PCHAR name )
{

	void* ptr = 0;

	for( int i = 0; i < 100; ++i )
	{
		PCHAR fmt = ( i < 10 ) ? "%s00%d" : "%s0%d"; // i hope they dont go over 99

		ptr = ( *fn )( formatString( fmt, name, i ).c_str(), 0 );
		if( ptr )
		{
			//MessageBox( NULL, formatString( fmt, name, i ).c_str(), "Got interface", MB_OK );
			break;
		}
	}

	return ptr;
}

bool Synthia::Utils::Interfaces::bInitiated = false;

IBaseClientDLL				*Synthia::Utils::Interfaces::Client = NULL;
IVEngineClient				*Synthia::Utils::Interfaces::Engine_cl = NULL;
IVRenderView				*Synthia::Utils::Interfaces::Render = NULL;
IGameEvent					*Synthia::Utils::Interfaces::GameEventManager = NULL;
CInput						*Synthia::Utils::Interfaces::Input = NULL;
IVModelInfo					*Synthia::Utils::Interfaces::ModelInfo = NULL;
IVDebugOverlay				*Synthia::Utils::Interfaces::DebugOverlay = NULL;
IMaterialSystem				*Synthia::Utils::Interfaces::MatSys = NULL;
IVModelRender				*Synthia::Utils::Interfaces::Modelrender = NULL;
IClientEntityList			*Synthia::Utils::Interfaces::EntList = NULL;
IFileSystem					*Synthia::Utils::Interfaces::FileSystem = NULL;
ICvar						*Synthia::Utils::Interfaces::CVar = NULL;


CVMTHookManager *Synthia::Utils::Hooks::FileSystem;
CVMTHookManager *Synthia::Utils::Hooks::ModelRender;
CVMTHookManager *Synthia::Utils::Hooks::Client;


void Synthia::Utils::Interfaces::Init( )
{
	
	if ( Synthia::Utils::Interfaces::bInitiated )
		return;

	HMODULE hModuleClient = NULL;
	
	while ( !GetModuleHandle("client.dll") )
		Sleep(10);

	hModuleClient = GetModuleHandle("client.dll");

	

	CreateInterfaceFn ClientFactory	= (CreateInterfaceFn)GetProcAddress(hModuleClient, "CreateInterface");

	ConnectTier1Libraries( &Synthia::Utils::AppSysFactory, 1 );
	ConnectTier2Libraries( &Synthia::Utils::AppSysFactory, 1 );
	ConnectTier3Libraries( &Synthia::Utils::AppSysFactory, 1 );

	Synthia::Utils::Interfaces::DebugOverlay	= ( IVDebugOverlay* )Synthia::Utils::Interfaces::get( &Synthia::Utils::AppSysFactory, "VDebugOverlay" );
	Synthia::Utils::Interfaces::CVar			= ( ICvar* )Synthia::Utils::Interfaces::get( &Synthia::Utils::AppSysFactory, "VEngineCvar" );
	Synthia::Utils::Interfaces::Engine_cl		= ( IVEngineClient* )Synthia::Utils::Interfaces::get( &Synthia::Utils::AppSysFactory, "VEngineClient" );
	Synthia::Utils::Interfaces::ModelInfo		= ( IVModelInfoClient* )Synthia::Utils::Interfaces::get( &Synthia::Utils::AppSysFactory, "VModelInfoClient" );
	//Synthia::Utils::Interfaces::Client		= ( IBaseClientDLL* )Synthia::Utils::Interfaces::get( &Synthia::Utils::AppSysFactory, "VClient" );;
	Synthia::Utils::Interfaces::EntList			= ( IClientEntityList* )Synthia::Utils::Interfaces::get( &ClientFactory, "VClientEntityList" );
	Synthia::Utils::Interfaces::Modelrender		= ( IVModelRender*)Synthia::Utils::Interfaces::get( &Synthia::Utils::AppSysFactory, "VEngineModel" );
	Synthia::Utils::Interfaces::MatSys			= ( IMaterialSystem *)Synthia::Utils::Interfaces::get( &Synthia::Utils::AppSysFactory, "VMaterialSystem" );
	Synthia::Utils::Interfaces::FileSystem		= ( IFileSystem *)Synthia::Utils::Interfaces::get( &Synthia::Utils::AppSysFactory, "VFileSystem" );

	Synthia::Utils::Hooks::Client->UnHook();

	PDWORD pdwClientVirtual = *reinterpret_cast< PDWORD* >( Synthia::Utils::Interfaces::Client );
	PDWORD pdwAddress = ( PDWORD ) ( ( ( DWORD ) pdwClientVirtual[20] ) );
	DWORD dwInputPointer = dwFindPattern((DWORD)pdwAddress, 0x100, (PBYTE)"\x8B\x0D", "xx" ); //Find the pointer to CInput in CHLClient::CreateMove.
	
	if ( dwInputPointer != NULL )
	{
		dwInputPointer += 0x2;
		Synthia::Utils::Interfaces::Input = **( CInput*** )dwInputPointer; //A pointer within a pointer. Pointerception.
	}else{

		MessageBox(NULL, "g_pInput is NULL !", "Synthia", MB_OK);
    }

	Synthia::Utils::Hooks::Client->ReHook();

	srand( time( NULL ) );

	Synthia::Utils::Interfaces::bInitiated = true;

}

bool Synthia::Utils::IsGameOverlayOpen()
{
	return (GetCursorPos(NULL) == TRUE);
}



int r;

HANDLE hOut, hIn;
void Synthia::Utils::WriteInConsole( const char *fmt, ... )
{

	va_list marker;
	char msg[4096];


	va_start(marker, fmt);
	Q_vsnprintf( msg, sizeof( msg ), fmt, marker );
	va_end(marker);

	WriteConsole( hOut, msg, strlen( msg ), (LPDWORD)&r, 0); 


}

int Synthia::Utils::CrawlTableForOffset( RecvTable *pTable, const char *name )
{
	int addOffset = 0;

	for( int i = 0; i < pTable->GetNumProps(); i++ )
	{
		RecvProp *pProp = pTable->GetProp( i );

		if( pProp->GetDataTable() && pProp->GetDataTable()->GetNumProps() > 0 )
		{
			int iOffset = Utils::CrawlTableForOffset( pProp->GetDataTable(), name );

			if( iOffset )
			{
				addOffset += pProp->GetOffset() + iOffset;
			}
		}

		string propName( pProp->GetName() );
		if( propName.find( "[0]" ) != propName.npos )
			propName.replace( propName.find( "[0]" ), 3, "\0\0\0" );

		if( stricmp( name, propName.c_str() ) )
			continue;

		return pProp->GetOffset() + addOffset;
	}
	return addOffset;
}

int Synthia::Utils::GetNetvarOffset( const char *table, const char *name )
{
	ClientClass *pClass = Utils::Interfaces::Client->GetAllClasses();

	for( ; pClass; pClass = pClass->m_pNext )
	{
		RecvTable *pTable = pClass->m_pRecvTable;

		if( pTable->GetNumProps() <= 1 )
			continue;

		if( stricmp( table, pTable->GetName() ) )
			continue;

		int iResult = Utils::CrawlTableForOffset( pTable, name );
		
		return iResult;
	}
	return NULL;
}