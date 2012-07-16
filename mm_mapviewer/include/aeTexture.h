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

#ifndef _ae_Texture_FILE
#define _ae_Texture_FILE

typedef enum
{
	TT_Texture,
	TT_Sprite,
	TT_Interface,
	TT_Font
}uTexType;

class aeTexture
{
	friend class aeTextureManager;
private:
	std::string             name;
	std::string             basename;
	int                     width,height,bpp;
	uTexType		type;
	UINT                    GLID;
	aeTextureManager*       texman;
	aeTexture( const std::string& tname, const std::string& basename, uTexType  textype, aeTextureManager* texman);
	bool LoadPNG();
	bool LoadFromLod();
	bool LoadSpriteFromLod( );
public:
	~aeTexture();
	int                     Width()  { return width;  }
	int                     Height() { return height; }
	int                     Bpp()    { return bpp;    }
	const std::string&      Name()   { return name;   }

	// Включить текстуру
	void    Enable()
	{
		glBindTexture( GL_TEXTURE_2D, GLID ); 
	}
	// Выключить текстуру
	void    Disable()
	{
		glBindTexture( GL_TEXTURE_2D, 0 ); 
	}
};
typedef boost::shared_ptr<aeTexture> p_aeTexture;
class aeTextureManager
{
private:
	typedef std::map<std::string, boost::weak_ptr<aeTexture> > texmap_t;
	typedef std::map<std::string, boost::weak_ptr<aeTexture> >::iterator texmap_iter_t;
	texmap_t items;
	friend class aeTexture;
	void            RemoveTexture( const std::string& name );
public:
	aeTextureManager();
	~aeTextureManager();
	p_aeTexture     GetTexture(const std::string& name, uTexType  textype);
};
extern aeTextureManager TexManager;

#endif //_ae_Texture_FILE