#pragma warning(disable : 4996)
#include <exception>
#include <algorithm>
#include <functional>
#include "Configuration.h"
#include "DataCenterEngine.h"
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
			return szPath;//__BasePath( szPath );
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


void DllPathTable::AddPath( std::string sDllPath )
{
	CriticalLock			lock( m_oLock );

	DataIOEngine::GetEngineObj().WriteInfo( "DllPathTable::AddPath() : dll path: %s", sDllPath.c_str() );
	std::vector<std::string>::push_back( sDllPath );
}

unsigned int DllPathTable::GetCount()
{
	CriticalLock			lock( m_oLock );

	return std::vector<std::string>::size();
}

std::string DllPathTable::GetPathByPos( unsigned int nPos )
{
	CriticalLock			lock( m_oLock );
	unsigned int			nSize = std::vector<std::string>::size();

	if( nPos >= nSize )
	{
		DataIOEngine::GetEngineObj().WriteWarning( "DllPathTable::GetPathByPos() : invalid dll path table index ( %u >= %u )", nPos, nSize );
		return "";
	}

	return this->operator []( nPos );
}


Configuration::Configuration()
 : m_bLoaded( false )
{
}

int Configuration::Load()
{
	inifile::IniFile	oIniFile;
	int					nErrCode = 0;
	char				pszDllPath[218] = { 0 };
	std::string			sIniPath = GetModulePath(g_oModule);
	std::string			sFolder = sIniPath;
	sFolder = __BasePath( (char*)sFolder.c_str() );
    sIniPath = sIniPath.substr( 0, sIniPath.find(".") ) + ".ini";

	if( true == m_bLoaded )
	{
		return 0;
	}

	///< ---------- load .ini -------------------------
	if( 0 != (nErrCode=oIniFile.load( sIniPath )) )
	{
		::printf( "Configuration::Load() : failed 2 load configuration file, %s\n", sIniPath.c_str() );
		return -1;
	}

	///< ------------------------ [service configuration] --------------------------------------
	///< Memory Database Plugin
	m_sMemPluginPath = sFolder + oIniFile.getStringValue( std::string("MemDB"), std::string("Path"), nErrCode );
	if( 0 != nErrCode )	{
		::printf( "Configuration::Load() : invalid memory plugin path\n" );
		return -2;
	}

	m_sRecoveryFolder = sFolder + oIniFile.getStringValue( std::string("MemDB"), std::string("DumpFolder"), nErrCode );
	if( false == m_sRecoveryFolder.empty() )	{
		::printf( "Configuration::Load() : dump folder = %s\n", m_sRecoveryFolder.c_str() );
	}

	///< Quotation Plugin
	int					nPluginPathCount = oIniFile.getIntValue( std::string("Plugin"), std::string("Count"), nErrCode );
	if( 0 != nErrCode )	{
		::printf( "Configuration::Load() : missing node (Plugin::Count)\n" );
		return -3;
	}

	for( int n = 0; n < nPluginPathCount; n++ )
	{
		::sprintf( pszDllPath, "Path_%d", n );
		std::string		sQuotationPluginPath = sFolder + oIniFile.getStringValue( std::string("Plugin"), std::string(pszDllPath), nErrCode );
		if( 0 != nErrCode )	{
			::printf( "Configuration::Load() : missing node (Plugin::Path_%d)\n", n );
			return -4;
		}

		m_oDCPathTable.AddPath( sQuotationPluginPath );
	}

	m_bLoaded = true;

	return 0;
}

Configuration& Configuration::GetConfigObj()
{
	static Configuration		obj;

	return obj;
}

const std::string& Configuration::GetMemPluginPath() const
{
	return m_sMemPluginPath;
}

DllPathTable& Configuration::GetDCPathTable()
{
	return m_oDCPathTable;
}

const std::string& Configuration::GetRecoveryFolderPath() const
{
	return m_sRecoveryFolder;
}







