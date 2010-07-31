// -----------------------------------------------------------------------------
// This file is part of Broken Sword 2.5
// Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsd�rfer
//
// Broken Sword 2.5 is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Broken Sword 2.5 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Broken Sword 2.5; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/common.h"
#include "sword25/kernel/kernel.h"
#include "sword25/script/script.h"
#include "sword25/script/luabindhelper.h"

#include "sword25/package/packagemanager.h"

// -----------------------------------------------------------------------------

static BS_PackageManager * GetPM()
{
	BS_Kernel * pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	BS_PackageManager * pPM = static_cast<BS_PackageManager *>(pKernel->GetService("package"));
	BS_ASSERT(pPM);
	return pPM;
}

// -----------------------------------------------------------------------------

static int LoadPackage(lua_State * L)
{
	BS_PackageManager * pPM = GetPM();

	lua_pushbooleancpp(L, pPM->LoadPackage(luaL_checkstring(L, 1), luaL_checkstring(L, 2)));

	return 1;
}

// -----------------------------------------------------------------------------

static int LoadDirectoryAsPackage(lua_State * L)
{
	BS_PackageManager * pPM = GetPM();

	lua_pushbooleancpp(L, pPM->LoadDirectoryAsPackage(luaL_checkstring(L, 1), luaL_checkstring(L, 2)));

	return 1;
}

// -----------------------------------------------------------------------------

static int GetCurrentDirectory(lua_State * L)
{
	BS_PackageManager * pPM = GetPM();

	lua_pushstring(L, pPM->GetCurrentDirectory().c_str());

	return 1;
}

// -----------------------------------------------------------------------------

static int ChangeDirectory(lua_State * L)
{
	BS_PackageManager * pPM = GetPM();

	lua_pushbooleancpp(L, pPM->ChangeDirectory(luaL_checkstring(L, 1)));

	return 1;
}

// -----------------------------------------------------------------------------

static int GetAbsolutePath(lua_State * L)
{
	BS_PackageManager * pPM = GetPM();

	lua_pushstring(L, pPM->GetAbsolutePath(luaL_checkstring(L, 1)).c_str());

	return 1;
}

// -----------------------------------------------------------------------------

static int GetFileSize(lua_State * L)
{
	BS_PackageManager * pPM = GetPM();

	lua_pushnumber(L, pPM->GetFileSize(luaL_checkstring(L, 1)));

	return 1;
}

// -----------------------------------------------------------------------------

static int GetFileType(lua_State * L)
{
	BS_PackageManager * pPM = GetPM();

	lua_pushnumber(L, pPM->GetFileType(luaL_checkstring(L, 1)));

	return 1;
}

// -----------------------------------------------------------------------------

static void SplitSearchPath(const std::string & Path, std::string & Directory, std::string & Filter)
{
	std::string::size_type LastSlash = Path.rfind("/");
	if (LastSlash == std::string::npos)
	{
		Directory = "";
		Filter = Path;
	}
	else
	{
		Directory = Path.substr(0, LastSlash);
		Filter = Path.substr(LastSlash + 1);
	}
}

// -----------------------------------------------------------------------------

static void DoSearch(lua_State * L, const std::string & Path, unsigned int Type)
{
	BS_PackageManager * pPM = GetPM();

	// Der Packagemanager-Service muss den Suchstring und den Pfad getrennt �bergeben bekommen.
	// Um die Benutzbarkeit zu verbessern sollen Skriptprogrammierer dieses als ein Pfad �bergeben k�nnen.
	// Daher muss der �bergebene Pfad am letzten Slash aufgesplittet werden.
	std::string Directory;
	std::string Filter;
	SplitSearchPath(Path, Directory, Filter);

	// Ergebnistable auf dem Lua-Stack erstellen
	lua_newtable(L);

	// Suche durchf�hren und die Namen aller gefundenen Dateien in die Ergebnistabelle einf�gen.
	// Als Indizes werden fortlaufende Nummern verwandt.
	unsigned int ResultNr = 1;
	BS_PackageManager::FileSearch * pFS = pPM->CreateSearch(Filter, Directory, Type);
	if (pFS)
	{
		do
		{
			lua_pushnumber(L, ResultNr);
			lua_pushstring(L, pFS->GetCurFileName().c_str());
			lua_settable(L, -3);
			ResultNr++;
		} while(pFS->NextFile());
	}

	delete(pFS);
}

// -----------------------------------------------------------------------------

static int FindFiles(lua_State * L)
{
	DoSearch(L, luaL_checkstring(L, 1), BS_PackageManager::FT_FILE);
	return 1;
}

// -----------------------------------------------------------------------------

static int FindDirectories(lua_State * L)
{
	DoSearch(L, luaL_checkstring(L, 1), BS_PackageManager::FT_DIRECTORY);
	return 1;
}

// -----------------------------------------------------------------------------

static int GetFileAsString(lua_State * L)
{
	BS_PackageManager * pPM = GetPM();

	unsigned int FileSize;
	void * FileData = pPM->GetFile(luaL_checkstring(L, 1), &FileSize);
	if (FileData)
	{
		lua_pushlstring(L, static_cast<char *>(FileData), FileSize);
		delete FileData;

		return 1;
	}
	else
		return 0;
}

// -----------------------------------------------------------------------------

static int FileExists(lua_State * L)
{
	lua_pushbooleancpp(L, GetPM()->FileExists(luaL_checkstring(L, 1)));
	return 1;
}

// -----------------------------------------------------------------------------

static const char * PACKAGE_LIBRARY_NAME = "Package";

static const luaL_reg PACKAGE_FUNCTIONS[] =
{
	"LoadPackage", LoadPackage,
	"LoadDirectoryAsPackage", LoadDirectoryAsPackage,
	"GetCurrentDirectory", GetCurrentDirectory,
	"ChangeDirectory", ChangeDirectory,
	"GetAbsolutePath", GetAbsolutePath,
	"GetFileSize", GetFileSize,
	"GetFileType", GetFileType,
	"FindFiles", FindFiles,
	"FindDirectories", FindDirectories,
	"GetFileAsString", GetFileAsString,
	"FileExists", FileExists,
	0, 0,
};

// -----------------------------------------------------------------------------

bool BS_PackageManager::_RegisterScriptBindings()
{
	BS_Kernel * pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	BS_ScriptEngine * pScript = static_cast<BS_ScriptEngine *>(pKernel->GetService("script"));
	BS_ASSERT(pScript);
	lua_State * L = static_cast<lua_State *>(pScript->GetScriptObject());
	BS_ASSERT(L);

	if (!BS_LuaBindhelper::AddFunctionsToLib(L, PACKAGE_LIBRARY_NAME, PACKAGE_FUNCTIONS)) return false;

	return true;
}