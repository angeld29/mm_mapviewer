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