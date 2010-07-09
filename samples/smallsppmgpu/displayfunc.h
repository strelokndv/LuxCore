/***************************************************************************
 *   Copyright (C) 1998-2010 by authors (see AUTHORS.txt )                 *
 *                                                                         *
 *   This file is part of LuxRays.                                         *
 *                                                                         *
 *   LuxRays is free software; you can redistribute it and/or modify       *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   LuxRays is distributed in the hope that it will be useful,            *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *                                                                         *
 *   LuxRays website: http://www.luxrender.net                             *
 ***************************************************************************/

#ifndef _DISPLAYFUNC_H
#define	_DISPLAYFUNC_H

#include <cmath>

#if defined (WIN32)
#include <windows.h>
#endif

// Jens's patch for MacOS
#if defined(__APPLE__)
#include <GLut/glut.h>
#else
#include <GL/glut.h>
#endif

extern void InitGlut(int argc, char *argv[], const unsigned int width, const unsigned int height);
extern void RunGlut(const unsigned int width, const unsigned int height);

#endif	/* _DISPLAYFUNC_H */

