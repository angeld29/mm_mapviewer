/*

Copyright (C) 2009 Angel (angel.d.death@gmail.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the included (GNU.txt) GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#include "stdafx.h"

inline const   font_sym_typ & aeFont::GetChar( int charcode )
{
	std::map < int, font_sym_typ >::iterator result = tbl.find( charcode );

	if ( result == tbl.end(  ) )
		return tbl[defaultchar];
	else
		return ( *result ).second;
}

aeFont::aeFont( const std::string& name )
{
	tex = TexManager.GetTexture( name, TT_Font );

	//font_header_typ header;
	font_sym_typ sym;
	std::string fname( "data/fonts/"+name+".dat");

	FILE   *f = fopen( fname.c_str(), "rb" );

	if ( !f )
	{
		throw error( (boost::format("aeFont:Cannot Load Font %s") % name).str() );
	}
	fread( &header, sizeof( header ), 1, f );
	for ( int i = 0; i < header.numchars; i++ )
	{
		fread( &sym, sizeof( sym ), 1, f );
		tbl[sym.charcode] = sym;
	}
	fclose( f );
}


void aeFont::DrawString( float x, float y, const std::string& str , int fixedwidth )
{

	tex->Enable( );
	glPushMatrix(  );
	glTranslated( x, y, 0 );

	for ( std::string::const_iterator i = str.begin(); i!=str.end(); ++i )
	{
		const font_sym_typ & sym = GetChar( (BYTE)*i );

		glBegin( GL_QUADS );	// Рисуем букву четырехугольником
			glTexCoord2f( sym.x1, sym.y1 );
			glVertex2f( 0 + sym.hDx, 0 - sym.hDy );
			glTexCoord2f( sym.x1, sym.y2 );
			glVertex2f( 0 + sym.hDx, sym.sy - sym.hDy );
			glTexCoord2f( sym.x2, sym.y2 );
			glVertex2f( sym.sx + sym.hDx, sym.sy - sym.hDy );
			glTexCoord2f( sym.x2, sym.y1 );
			glVertex2f( sym.sx + sym.hDx, 0 - sym.hDy );
		glEnd(  );	// Закончили рисовать
		// Перемещаемся на другую позицию*/
		if( fixedwidth )
		glTranslated( fixedwidth, 0, 0 );
		else
		glTranslated( sym.hAdvance, 0, 0 );
	}
	glPopMatrix(  );
}
