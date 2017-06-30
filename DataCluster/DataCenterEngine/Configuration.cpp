#pragma warning(disable : 4996)
#include <exception>
#include <algorithm>
#include <functional>
#include "DataCenterEngine.h"
#include "Configuration.h"
#include "../Infrastructure/IniFile.h"
#include "../Infrastructure/DateTime.h"


HMODULE			g_oModule = NULL;


char*	__BasePath(char *in)
{
	if( !in )
		return NULL;

	int	len = strlen(in);
	for( int i = len-1; i >= 0; i-- )
	{
		if( in[i] == '\\' || in[i] == '/' )
		{
			in[i + 1] = 0;
			break;
		}
	}

	return in;
}

std::string GetModulePath( void* hModule )
{
	char					szPath[MAX_PATH] = { 0 };
#ifndef LINUXCODE
		int	iRet = ::GetModuleFileName( (HMODULE)hModule, szPath, MAX_PATH );
		if( iRet <= 0 )	{
			return "";
		} else {
			return __BasePath( szPath );
		}
#else
		if( !hModule ) {
			int iRet =  readlink( "/proc/self/exe", szPath, MAX_PATH );
			if( iRet <= 0 ) {
				return "";
			} else {
				return __BasePath( szPath );
			}
		} else {
			class MDll	*pModule = (class MDll *)hModule;
			strncpy( szPath, pModule->GetDllSelfPath(), sizeof(szPath) );
			if( strlen(szPath) == 0 ) {
				return "";
			} else {
				return __BasePath(szPath);
			}
		}
#endif
}


Configuration::Configuration()
{
}

int Configuration::Load()
{
	inifile::IniFile	oIniFile;
	int					nErrCode = 0;
	std::string			sIniPath = GetModulePath(NULL) + "DataNode.ini";

	///< ---------- load .ini -------------------------
	if( 0 != (nErrCode=oIniFile.load( sIniPath )) )
	{
		::printf( "Configuration::Load() : failed 2 load configuration file, %s\n", sIniPath.c_str() );
		return -1;
	}

	///< [service configuration]
	m_sMemPluginPath = oIniFile.getStringValue( std::string("Plugin"), std::string("memdb"), nErrCode );
	if( 0 != nErrCode )	{
		::printf( "Configuration::Load() : invalid memory plugin path\n" );
		return -2;
	}

	m_sDataCollectorPluginPath = oIniFile.getStringValue( std::string("Plugin"), std::string("datacollector"), nErrCode );
	if( 0 != nErrCode )	{
		::printf( "Configuration::Load() : invalid data collector plugin path\n" );
		return -3;
	}

	///< [service framework configuration]
	m_sRecoveryFolder = oIniFile.getStringValue( std::string("ServerIO"), std::string("dumpfolder"), nErrCode );
	if( false == m_sRecoveryFolder.empty() )	{
		::printf( "Configuration::Load() : dump folder = %s\n", m_sRecoveryFolder.c_str() );
	}

	return 0;
}

Configuration& Configuration::GetConfigObj()
{
	static Configuration		obj;

	return obj;
}

const std::string& Configuration::GetRecoveryFolderPath() const
{
	return m_sRecoveryFolder;
}

const std::string& Configuration::GetMemPluginPath() const
{
	return m_sMemPluginPath;
}

const std::string& Configuration::GetDataCollectorPluginPath() const
{
	return m_sDataCollectorPluginPath;
}






