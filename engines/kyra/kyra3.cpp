/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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

#include "kyra/kyra.h"
#include "kyra/kyra3.h"
#include "kyra/screen.h"
#include "kyra/wsamovie.h"
#include "kyra/sound.h"
#include "kyra/text.h"
#include "kyra/vqa.h"

#include "common/system.h"
#include "common/config-manager.h"

// TODO: Temporary, to get the mouse cursor mock-up working
#include "graphics/cursorman.h"

namespace Kyra {
KyraEngine_v3::KyraEngine_v3(OSystem *system) : KyraEngine(system) {
	_soundDigital = 0;
	_musicSoundChannel = -1;
	_menuAudioFile = "TITLE1.AUD";
	_curMusicTrack = -1;
	_unkPage1 = _unkPage2 = 0;
	_interfaceCPS1 = _interfaceCPS2 = 0;
	memset(_gameShapes, 0, sizeof(_gameShapes));
	_shapePoolBuffer = 0;
	_unkBuffer17 = 0;
	_itemBuffer1 = _itemBuffer2 = 0;
	_mouseSHPBuf = 0;
	_tableBuffer1 = _tableBuffer2 = 0;
}

KyraEngine_v3::~KyraEngine_v3() {
	delete _soundDigital;
	
	delete [] _unkPage1;
	delete [] _unkPage2;
	delete [] _interfaceCPS1;
	delete [] _interfaceCPS2;

	delete [] _unkBuffer17;

	delete [] _itemBuffer1;
	delete [] _itemBuffer2;

	delete [] _shapePoolBuffer;

	delete [] _mouseSHPBuf;
}

int KyraEngine_v3::setupGameFlags() {
	_game = GI_KYRA3;
	_lang = 0;
	Common::Language lang = Common::parseLanguage(ConfMan.get("language"));

	switch (lang) {
	case Common::EN_ANY:
	case Common::EN_USA:
	case Common::EN_GRB:
		_lang = 0;
		break;

	case Common::FR_FRA:
		_lang = 1;
		break;

	case Common::DE_DEU:
		_lang = 2;
		break;

	default:
		warning("unsupported language, switching back to English");
		_lang = 0;
		break;
	}

	return 0;
}

Movie *KyraEngine_v3::createWSAMovie() {
	return new WSAMovieV2(this);
}

int KyraEngine_v3::init() {
	KyraEngine::init();
	
	_soundDigital = new SoundDigital(this, _mixer);
	assert(_soundDigital);
	if (!_soundDigital->init())
		error("_soundDigital->init() failed");
	
	_screen->loadFont(Screen::FID_6_FNT, "6.FNT");
	_screen->loadFont(Screen::FID_8_FNT, "8FAT.FNT");
	_screen->loadFont(Screen::FID_BOOKFONT_FNT, "BOOKFONT.FNT");
	_screen->setAnimBlockPtr(3500);
	_screen->setScreenDim(0);

	_shapePoolBuffer = new uint8[300000];
	assert(_shapePoolBuffer);
	memset(_shapePoolBuffer, 0, 300000);

	initTableBuffer(_shapePoolBuffer, 300000);

	_unkBuffer17 = new uint8[1850];
	assert(_unkBuffer17);

	_itemBuffer1 = new uint8[72];
	_itemBuffer2 = new uint8[144];
	assert(_itemBuffer1 && _itemBuffer2);

	_mouseSHPBuf = _res->fileData("MOUSE.SHP", 0);
	assert(_mouseSHPBuf);

	for (int i = 0; i <= 6; ++i) {
		_gameShapes[i] = _screen->getPtrToShape(_mouseSHPBuf, i);
	}

	initItems();
	// XXX

	_screen->setMouseCursor(0, 0, *_gameShapes);

	return 0;
}

int KyraEngine_v3::go() {
	uint8 *pal = _screen->getPalette(1);
	assert(pal);
	
	Movie *logo = createWSAMovie();
	assert(logo);
	logo->open("REVENGE.WSA", 1, pal);
	assert(logo->opened());
	
	bool running = true;
	while (running && !_quitFlag) {
		_screen->_curPage = 0;
		_screen->clearPage(0);

		pal[0] = pal[1] = pal[2] = 0;
		
		_screen->setScreenPalette(pal);
		
		// XXX
		playMenuAudioFile();
		
		logo->setX(0); logo->setY(0);
		logo->setDrawPage(0);

		for (int i = 0; i < 64 && !_quitFlag; ++i) {
			uint32 nextRun = _system->getMillis() + 3 * _tickLength;
			logo->displayFrame(i);
			_screen->updateScreen();
			delayUntil(nextRun);
		}

		for (int i = 64; i > 29 && !_quitFlag; --i) {
			uint32 nextRun = _system->getMillis() + 3 * _tickLength;
			logo->displayFrame(i);
			_screen->updateScreen();
			delayUntil(nextRun);
		}
		
		switch (handleMainMenu(logo)) {
		case 0:
			delete logo;
			logo = 0;
			preinit();
			realInit();
			// XXX
			running = false;
			break;
		
		case 1:
			playVQA("K3INTRO");
			break;
		
		case 2:
			//delete logo;
			//logo = 0;
			//show load dialog
			//running = false;
			break;
		
		case 3:
			running = false;
			break;
		
		default:
			break;
		}
	}
	delete logo;

	return 0;
}

void KyraEngine_v3::playVQA(const char *name) {
	debugC(9, kDebugLevelMain, "KyraEngine::playVQA('%s')", name);
	VQAMovie vqa(this, _system);

	char filename[20];
	int size = 0;		// TODO: Movie size is 0, 1 or 2.

	snprintf(filename, sizeof(filename), "%s%d.VQA", name, size);

	if (vqa.open(filename)) {
		uint8 pal[768];
		memcpy(pal, _screen->getPalette(0), sizeof(pal));
		if (_screen->_curPage == 0)
			_screen->copyRegion(0, 0, 0, 0, 320, 200, 0, 3);

		_screen->hideMouse();
		_soundDigital->beginFadeOut(_musicSoundChannel);
		_musicSoundChannel = -1;
		_screen->fadeToBlack();
		vqa.setDrawPage(0);
		vqa.play();
		vqa.close();
		_screen->showMouse();

		if (_screen->_curPage == 0)
			_screen->copyRegion(0, 0, 0, 0, 320, 200, 3, 0);
		_screen->setScreenPalette(pal);
	}
}

#pragma mark -

void KyraEngine_v3::playMenuAudioFile() {
	debugC(9, kDebugLevelMain, "KyraEngine::playMenuAudioFile()");
	if (_soundDigital->isPlaying(_musicSoundChannel))
		return;

	Common::File *handle = new Common::File();
	uint32 temp = 0;
	_res->fileHandle(_menuAudioFile, &temp, *handle);
	if (handle->isOpen()) {
		_musicSoundChannel = _soundDigital->playSound(handle, true);
	} else {
		delete handle;
	}
}

void KyraEngine_v3::playMusicTrack(int track, int force) {
	debugC(9, kDebugLevelMain, "KyraEngine::playMusicTrack(%d, %d)", track, force);
	
	// XXX byte_2C87C compare
	
	if (_musicSoundChannel != -1 && !_soundDigital->isPlaying(_musicSoundChannel)) {
		force = 1;
	} else if (_musicSoundChannel == -1) {
		force = 1;
	}
	
	if (track == _curMusicTrack && !force)
		return;
	
	stopMusicTrack();
	
	if (_musicSoundChannel == -1) {
		assert(track < _soundListSize && track >= 0);

		Common::File *handle = new Common::File();
		uint32 temp = 0;
		_res->fileHandle(_soundList[track], &temp, *handle);
		if (handle->isOpen()) {
			_musicSoundChannel = _soundDigital->playSound(handle);
		} else {
			delete handle;
		}
	}
	
	_musicSoundChannel = track;
}

void KyraEngine_v3::stopMusicTrack() {
	if (_musicSoundChannel != -1 && _soundDigital->isPlaying(_musicSoundChannel)) {
		_soundDigital->stopSound(_musicSoundChannel);
	}
	
	_curMusicTrack = -1;
	_musicSoundChannel = -1;
}

int KyraEngine_v3::musicUpdate(int forceRestart) {
	debugC(9, kDebugLevelMain, "KyraEngine::unkUpdate(%d)", forceRestart);
	
	static uint32 timer = 0;
	static uint16 lock = 0;

	if (ABS<int>(_system->getMillis() - timer) > (int)(0x0F * _tickLength)) {
		timer = _system->getMillis();
	}
	
	if (_system->getMillis() < timer && !forceRestart) {
		return 1;
	}

	if (!lock) {
		lock = 1;
		if (_musicSoundChannel >= 0) {
			// XXX sub_1C262 (sound specific. it seems to close some sound resource files in special cases)
			if (!_soundDigital->isPlaying(_musicSoundChannel)) {
				if (_curMusicTrack != -1)
					playMusicTrack(_curMusicTrack, 1);
			}
		}
		lock = 0;
		timer = _system->getMillis() + 0x0F * _tickLength;
	}
	
	return 1;
}

#pragma mark -

int KyraEngine_v3::handleMainMenu(Movie *logo) {
	debugC(9, kDebugLevelMain, "KyraEngine::handleMainMenu(%p)", (const void*)logo);
	int command = -1;
	
	uint8 colorMap[16];
	memset(colorMap, 0, sizeof(colorMap));
	_screen->setTextColorMap(colorMap);
	
	const char * const *strings = &_mainMenuStrings[_lang << 2];
	Screen::FontId oldFont = _screen->setFont(Screen::FID_8_FNT);
	int charWidthBackUp = _screen->_charWidth;
	
	_screen->_charWidth = -2;
	_screen->setScreenDim(3);
	int backUpX = _screen->_curDim->sx;
	int backUpY = _screen->_curDim->sy;
	int backUpWidth = _screen->_curDim->w;
	int backUpHeight = _screen->_curDim->h;
	_screen->copyRegion(backUpX, backUpY, backUpX, backUpY, backUpWidth, backUpHeight, 0, 3);

	int x = _screen->_curDim->sx << 3;
	int y = _screen->_curDim->sy;
	int width = _screen->_curDim->w << 3;
	int height =  _screen->_curDim->h;

	drawMainBox(x, y, width, height, 1);
	drawMainBox(x + 1, y + 1, width - 2, height - 2, 0);
	
	int curFrame = 29, frameAdd = 1;
	uint32 nextRun = 0;

	int selected = 0;
	
	drawMainMenu(strings, selected);

	_system->warpMouse(300, 180);
	_screen->showMouse();

	int fh = _screen->getFontHeight();
	int textPos = ((_screen->_curDim->w >> 1) + _screen->_curDim->sx) << 3;

	Common::Rect menuRect(x + 16, y + 4, x + width - 16, y + 4 + fh * 4);
	
	while (command == -1 && !_quitFlag) {
		// yes 2 * _tickLength here not 3 * like in the first draw
		nextRun = _system->getMillis() + 2 * _tickLength;
		logo->displayFrame(curFrame);
		_screen->updateScreen();
		
		curFrame += frameAdd;
		if (curFrame < 29) {
			curFrame = 29;
			frameAdd = 1;
		} else if (curFrame > 63) {
			curFrame = 64;
			frameAdd = -1;
		}
		
		// XXX
		
		while (_system->getMillis() < nextRun) {
			// XXX
			_screen->updateScreen();
			if ((int32)nextRun - (int32)_system->getMillis() >= 10)
				delay(10);
		}

		if (menuRect.contains(mouseX(), mouseY())) {
			int item = (mouseY() - menuRect.top) / fh;

			if (item != selected) {
				gui_printString(strings[selected], textPos, menuRect.top + selected * fh, 0x80, 0, 5);
				gui_printString(strings[item], textPos, menuRect.top + item * fh, 0xFF, 0, 5);

				selected = item;
			}

			if (_mousePressFlag) {
				// TODO: Flash the text
				command = item;

				// TODO: For now, only playing the intro and quitting is supported
				if (command != 1 && command != 3)
					command = -1;
			}
		}
	}
	
	if (_quitFlag)
		command = -1;
	
	_screen->copyRegion(backUpX, backUpY, backUpX, backUpY, backUpWidth, backUpHeight, 3, 0);
	_screen->_charWidth = charWidthBackUp;
	_screen->setFont(oldFont);
	
	if (command == 3) {
		_soundDigital->beginFadeOut(_musicSoundChannel);
		_screen->fadeToBlack();
		_soundDigital->stopSound(_musicSoundChannel);
		_musicSoundChannel = -1;
	}
	
	return command;
}

void KyraEngine_v3::drawMainMenu(const char * const *strings, int select) {
	debugC(9, kDebugLevelMain, "KyraEngine::drawMainMenu(%p)", (const void*)strings);
	static const uint16 menuTable[] = { 0x01, 0x04, 0x0C, 0x04, 0x00, 0x80, 0xFF, 0x00, 0x01, 0x02, 0x03 };
	
	int top = _screen->_curDim->sy;
	top += menuTable[1];
	
	for (int i = 0; i < menuTable[3]; ++i) {
		int curY = top + i * _screen->getFontHeight();
		int color = (i == select) ? menuTable[6] : menuTable[5];
		gui_printString(strings[i], ((_screen->_curDim->w >> 1) + _screen->_curDim->sx) << 3, curY, color, 0, 5);
	}
}

void KyraEngine_v3::drawMainBox(int x, int y, int w, int h, int fill) {
	debugC(9, kDebugLevelMain, "KyraEngine::drawMainBox(%d, %d, %d, %d, %d)", x, y, w, h, fill);
	static const uint8 colorTable[] = { 0x16, 0x19, 0x1A, 0x16 };
	--w; --h;

	if (fill) {
		_screen->fillRect(x, y, x+w, y+h, colorTable[0]);
	}
	
	_screen->drawClippedLine(x, y+h, x+w, y+h, colorTable[1]);
	_screen->drawClippedLine(x+w, y, x+w, y+h, colorTable[1]);
	_screen->drawClippedLine(x, y, x+w, y, colorTable[2]);
	_screen->drawClippedLine(x, y, x, y+h, colorTable[2]);
	
	_screen->setPagePixel(_screen->_curPage, x, y+h, colorTable[3]);
	_screen->setPagePixel(_screen->_curPage, x+w, y, colorTable[3]);
}

void KyraEngine_v3::gui_printString(const char *format, int x, int y, int col1, int col2, int flags, ...) {
	debugC(9, kDebugLevelMain, "KyraEngine::gui_printString('%s', %d, %d, %d, %d, %d, ...)", format, x, y, col1, col2, flags);
	if (!format)
		return;
	
	char string[512];
	va_list vaList;
	va_start(vaList, flags);
	vsprintf(string, format, vaList);
	va_end(vaList);
	
	if (flags & 1) {
		x -= _screen->getTextWidth(string) >> 1;
	}
	
	if (flags & 2) {
		x -= _screen->getTextWidth(string);
	}
	
	if (flags & 4) {
		_screen->printText(string, x - 1, y, 240, col2);
		_screen->printText(string, x, y + 1, 240, col2);
	}
	
	if (flags & 8) {
		_screen->printText(string, x - 1, y, 227, col2);
		_screen->printText(string, x, y + 1, 227, col2);
	}
	
	_screen->printText(string, x, y, col1, col2);
}

#pragma mark -

void KyraEngine_v3::preinit() {
	debugC(9, kDebugLevelMain, "KyraEngine::preinit()");

	musicUpdate(0);

	// XXX snd_allocateSoundBuffer?
	memset(_flagsTable, 0, sizeof(_flagsTable));

	// XXX
	setGameFlag(0x216);
	
	_unkPage1 = new uint8[64000];
	assert(_unkPage1);
	
	musicUpdate(0);
	musicUpdate(0);
	
	_interfaceCPS1 = new uint8[17920];
	_interfaceCPS2 = new uint8[3840];
	assert(_interfaceCPS1 && _interfaceCPS2);
	
	_screen->setFont(Screen::FID_6_FNT);
}

void KyraEngine_v3::realInit() {
	debugC(9, kDebugLevelMain, "KyraEngine::realInit()");
}

#pragma mark -

int KyraEngine_v3::initTableBuffer(uint8 *buf, int size) {
	debugC(9, kDebugLevelMain, "KyraEngine::initTableBuffer(%p, %d)", (void*)buf, size);

	if (!buf || size < 6320)
		return 0;

	if (_tableBuffer2 != _tableBuffer1 && _tableBuffer2 && _tableBuffer1) {
		// no idea if this *should* be called
		memmove(_tableBuffer2, _tableBuffer1, 6320);
	}

	_tableBuffer1 = buf;
	size -= 6320;

	*((uint16*)(_tableBuffer1)) = 0;
	*((uint16*)(_tableBuffer1 + 2)) = 1;
	*((uint16*)(_tableBuffer1 + 4)) = 1;
	*((uint32*)(_tableBuffer1 + 6)) = size >> 4;
	*((uint16*)(_tableBuffer1 + 10)) = 1;
	*((uint32*)(_tableBuffer1 + 16)) = 6320;
	*((uint32*)(_tableBuffer1 + 22)) = size >> 4;

	_tableBuffer2 = buf;

	return 1;
}

void KyraEngine_v3::updateTableBuffer(uint8 *buf) {
	debugC(9, kDebugLevelMain, "KyraEngine::updateTableBuffer(%p)", (void*)buf);

	if (_tableBuffer2 == buf)
		return;

	if (_tableBuffer1 != _tableBuffer2)
		memmove(_tableBuffer2, _tableBuffer1, 6320);

	_tableBuffer2 = _tableBuffer1 = buf;
}

int KyraEngine_v3::addShapeToTable(uint8 *buf, int id, int shapeNum) {
	debugC(9, kDebugLevelMain, "KyraEngine::addShapeToTable(%p, %d, %d)", (void*)buf, id, shapeNum);

	if (!buf)
		return 0;

	uint8 *shapePtr = _screen->getPtrToShape(buf, shapeNum);
	if (!shapePtr)
		return 0;

	int shapeSize = _screen->getShapeSize(shapePtr);

	if (getTableSize(_shapePoolBuffer) < shapeSize) {
		// XXX
		error("[1] unimplemented table handling");
	}

	uint8 *ptr = allocTableSpace(_shapePoolBuffer, shapeSize, id);

	if (!ptr) {
		// XXX
		error("[2] unimplemented table handling");
	}

	if (!ptr) {
		warning("adding shape %d to _shapePoolBuffer not possible, not enough space left\n", id);
		return shapeSize;
	}

	memcpy(ptr, shapePtr, shapeSize);
	return shapeSize;
}

int KyraEngine_v3::getTableSize(uint8 *buf) {
	debugC(9, kDebugLevelMain, "KyraEngine::getTableSize(%p)", (void*)buf);
	updateTableBuffer(buf);

	if (*((uint16*)(_tableBuffer1 + 4)) >= 450)
		return 0;

	return (*((uint32*)(_tableBuffer1 + 6)) << 4);
}

uint8 *KyraEngine_v3::allocTableSpace(uint8 *buf, int size, int id) {
	debugC(9, kDebugLevelMain, "KyraEngine::allocTableSpace(%p, %d, %d)", (void*)buf, size, id);

	if (!buf || !size)
		return 0;

	updateTableBuffer(buf);

	int entries = *(uint16*)(_tableBuffer1 + 4);

	if (entries >= 450)
		return 0;

	size += 0xF;
	size &= 0xFFFFFFF0;

	uint size2 = size >> 4;

	if (*(uint32*)(_tableBuffer1 + 6) < size2)
		return 0;

	int unk1 = *(uint16*)(_tableBuffer1);
	int usedEntry = unk1;
	int ok = 0;

	for (; usedEntry < entries; ++usedEntry) {
		if (size2 <= *(uint32*)(_tableBuffer1 + usedEntry * 14 + 22)) {
			ok = 1;
			break;
		}
	}

	if (!ok)
		return 0;

	ok = 0;
	int unk3 = unk1 - 1;
	while (ok <= unk3) {
		int temp = (ok + unk3) >> 1;

		if (*(uint32*)(_tableBuffer1 + temp * 14 + 12) >= (uint)id) {
			if (*(uint32*)(_tableBuffer1 + temp * 14 + 12) <= (uint)id) {
				return 0;
			} else {
				unk3 = temp - 1;
				continue;
			}
		}

		ok = temp + 1;
	}

	uint8 *buf2 = _tableBuffer1 + usedEntry * 14;

	uint unkValue1 = *(uint32*)(buf2 + 16);
	uint unkValue2 = *(uint32*)(buf2 + 22);

	if (size2 < unkValue2) {
		*(uint32*)(buf2 + 22) = unkValue2 - size2;
		*(uint32*)(buf2 + 16) = unkValue1 + size;
		memcpy(_tableBuffer1 + entries * 14 + 12, _tableBuffer1 + unk1 * 14 + 12, 14);
	} else {
		if (usedEntry > unk1) {
			memcpy(buf2 + 12, _tableBuffer1 + unk1 * 14 + 12, 14);
		}
		int temp = *(uint16*)(_tableBuffer1 + 2) - 1;
		*(uint16*)(_tableBuffer1 + 2) = temp;
		temp = *(uint16*)(_tableBuffer1 + 4) - 1;
		*(uint16*)(_tableBuffer1 + 4) = temp;
	}

	for (int i = unk1; i > ok; --i) {
		memcpy(_tableBuffer1 + i * 14 + 12, _tableBuffer1 + (i-1) * 14 + 12, 14);
	}

	buf2 = _tableBuffer1 + ok * 14;

	*(uint32*)(buf2 + 12) = id;
	*(uint32*)(buf2 + 16) = unkValue1;
	*(uint32*)(buf2 + 20) = (_system->getMillis() / 60) >> 4;
	*(uint32*)(buf2 + 22) = size2;

	int temp = *(uint16*)(_tableBuffer1) + 1;
	*(uint16*)(_tableBuffer1) = temp;
	temp = *(uint16*)(_tableBuffer1 + 4) + 1;
	*(uint16*)(_tableBuffer1 + 4) = temp;

	if (temp > *(uint16*)(_tableBuffer1 + 10)) {
		*(uint16*)(_tableBuffer1 + 10) = temp;
		if (temp > _unkTableValue)
			_unkTableValue = temp;
	}

	temp = *(uint32*)(_tableBuffer1 + 6) - size2;
	*(uint32*)(_tableBuffer1 + 6) = temp;

	return _tableBuffer2 + unkValue1;
}

namespace {
int tableIdCompare(const void *l, const void *r) {
	int lV = *(uint32*)(l);
	int rV = *(uint32*)(r);

	return CLIP(lV - rV, -1, 1);
}
}

uint8 *KyraEngine_v3::findIdInTable(uint8 *buf, int id) {
	debugC(9, kDebugLevelMain, "KyraEngine::findIdInTable(%p, %d)", (void*)buf, id);

	updateTableBuffer(buf);

	uint32 idVal = id;
	uint8 *ptr = (uint8*)bsearch(&idVal, _tableBuffer1 + 12, *(uint16*)(_tableBuffer1), 14, &tableIdCompare);

	if (!ptr) {
		return 0;
	}

	return _tableBuffer2 + *(uint32*)(ptr + 4);
}

uint8 *KyraEngine_v3::findShapeInTable(int id) {
	debugC(9, kDebugLevelMain, "KyraEngine::findShapeInTable(%d)", id);

	return findIdInTable(_shapePoolBuffer, id);
}

#pragma mark - items

void KyraEngine_v3::initItems() {
	debugC(9, kDebugLevelMain, "KyraEngine::initItems()");

	_screen->loadBitmap("ITEMS.CSH", 3, 3, 0);

	for (int i = 248; i <= 319; ++i) {
		addShapeToTable(_screen->getPagePtr(3), i, i-248);
	}

	_screen->loadBitmap("ITEMS2.CSH", 3, 3, 0);

	for (int i = 320; i <= 397; ++i) {
		addShapeToTable(_screen->getPagePtr(3), i, i-320);
	}

	uint32 size = 0;
	uint8 *itemsDat = _res->fileData("_ITEMS.DAT", &size);

	assert(size >= 72+144);

	memcpy(_itemBuffer1, itemsDat   ,  72);
	memcpy(_itemBuffer2, itemsDat+72, 144);

	delete [] itemsDat;

	_screen->_curPage = 0;
}

} // end of namespace Kyra
