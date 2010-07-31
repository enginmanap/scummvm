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

#ifndef	SWORD25_SCREENSHOT_H
#define SWORD25_SCREENSHOT_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/common.h"
#include "sword25/kernel/memlog_off.h"
#include <string>
#include <vector>
#include "sword25/kernel/memlog_on.h"

// -----------------------------------------------------------------------------
// Class declaration
// -----------------------------------------------------------------------------

class BS_Screenshot
{
public:
	static bool SaveToFile(unsigned int Width, unsigned int Height, const std::vector<unsigned int> & Data, const std::string & Filename);
	static bool SaveThumbnailToFile(unsigned int Width, unsigned int Height, const std::vector<unsigned int> & Data, const std::string & Filename);
};

#endif