/*
 *   LMeter: Multilayer superconductive inductance extractor
 *   
 *   Copyright (C) 1992-1999 Paul Bunyk
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 *    That is the file to controle compiler-depended features
 *      of LM program.
 */

/*
 *    next defines can be undefined while debugging
 */

#define	NDUMP			/* don't dump internal tables     */
#define	NTEXT			/* don't write text of matrix     */
#undef	NSYMM			/* don't check matrix symmetry    */

#if	defined(__BORLANDC__)
#define	_HUGE	huge
#elif	defined(__NDPC__) || defined(__GNUC__) || defined(__STDC__)
#define	_HUGE
#define	NRMEM
#endif
