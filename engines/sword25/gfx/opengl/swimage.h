/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

/*
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#ifndef SWORD25_SWIMAGE_H
#define SWORD25_SWIMAGE_H

// -----------------------------------------------------------------------------
// INCLUDES
// -----------------------------------------------------------------------------

#include "sword25/kernel/common.h"
#include "sword25/gfx/image/image.h"
#include "sword25/gfx/graphicengine.h"


namespace Sword25 {

// -----------------------------------------------------------------------------
// CLASS DEFINITION
// -----------------------------------------------------------------------------

class SWImage : public Image {
public:
	SWImage(const Common::String &Filename, bool &Result);
	virtual ~SWImage();

	virtual int GetWidth() const {
		return m_Width;
	}
	virtual int GetHeight() const {
		return m_Height;
	}
	virtual GraphicEngine::COLOR_FORMATS GetColorFormat() const {
		return GraphicEngine::CF_ARGB32;
	}

	virtual bool Blit(int PosX = 0, int PosY = 0,
	                  int Flipping = Image::FLIP_NONE,
	                  Common::Rect *pPartRect = NULL,
	                  uint Color = BS_ARGB(255, 255, 255, 255),
	                  int Width = -1, int Height = -1);
	virtual bool Fill(const Common::Rect *FillRectPtr, uint Color);
	virtual bool SetContent(const byte *Pixeldata, uint size, uint Offset, uint Stride);
	virtual uint GetPixel(int X, int Y);

	virtual bool IsBlitSource() const               {
		return false;
	}
	virtual bool IsBlitTarget() const               {
		return false;
	}
	virtual bool IsScalingAllowed() const           {
		return false;
	}
	virtual bool IsFillingAllowed() const           {
		return false;
	}
	virtual bool IsAlphaAllowed() const             {
		return false;
	}
	virtual bool IsColorModulationAllowed() const   {
		return false;
	}
	virtual bool IsSetContentAllowed() const        {
		return false;
	}
private:
	uint *_ImageDataPtr;

	int m_Width;
	int m_Height;
};

} // End of namespace Sword25

#endif
