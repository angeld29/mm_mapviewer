#include "stdafx.h"
//#include <libgfl.h>
#include <zlib.h>
#include <png.h>

aeTextureManager TexManager;

static const std::string typenames[] = {
		"data/textures/common/",
		"data/textures/sprite/",
		"data/textures/interface/",
		"data/fonts/"
};
bool aeTexture::LoadPNG( )
{
	std::string fname( name + ".png");
	Log << aeLog::debug <<"aeTexture::LoadPNG: Loading texture "<< fname << aeLog::endl;
	png_structp	png; 
	png_infop	pnginfo; 
	int		bit_depth, color_type, interlace_type, compression_type, filter_type;
	
	FILE*fin=fopen( fname.c_str(), "rb");
	if(!fin)
		return false;
	
	png = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	if (!png) 
	{
		fclose(fin);
		return false;
	}
	
	pnginfo = png_create_info_struct( png );
	if (!pnginfo) {
		fclose(fin);
		png_destroy_read_struct(&png,&pnginfo,png_infopp_NULL);
		return false;
	}
	if (setjmp(png_jmpbuf(png))) {
		png_destroy_read_struct(&png, &pnginfo, png_infopp_NULL);
		fclose(fin);
		Log << "LoadPNG: " << fname << " : Invalid PNG format" << aeLog::endl;
		return false;
	}

	png_set_sig_bytes(png,0/*sizeof( sig )*/);

	png_init_io(png, fin);
	png_read_info(png, pnginfo);

	png_uint_32	w, h;
	png_get_IHDR(png, pnginfo, &w, &h, &bit_depth, 
		&color_type, &interlace_type, &compression_type, &filter_type);

	if (color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(png);

	if (color_type == PNG_COLOR_TYPE_GRAY ||
		color_type == PNG_COLOR_TYPE_GRAY_ALPHA){
			png_set_gray_to_rgb(png);
			if (bit_depth < 8)
				png_set_gray_1_2_4_to_8(png);
		}
	// Add alpha channel if present 
	if(png_get_valid( png, pnginfo, PNG_INFO_tRNS ))
			png_set_tRNS_to_alpha(png);

	if ( !(color_type & PNG_COLOR_MASK_ALPHA) )
		png_set_filler(png, 255, PNG_FILLER_AFTER);
	if (bit_depth < 8)
		png_set_expand (png);
	if (bit_depth == 16)
		png_set_strip_16(png);

	// update the info structure 
	png_read_update_info( png, pnginfo );

	int rowbytes = png_get_rowbytes( png, pnginfo );
	bpp = png_get_channels( png, pnginfo );
	width = w;
	height = h;
	GLuint format;
	switch( bpp )
	{
	case 4:
		format = GL_RGBA;
		break;
	case 3:
		format = GL_RGB;
		break;
	default:
		png_destroy_read_struct(&png, &pnginfo, png_infopp_NULL);
		fclose(fin);
		Log << "LoadPNG: " << fname << " : Invalid PNG bpp " << bpp << aeLog::endl;
		return false;
	}

	png_bytep  data = new png_byte[height * rowbytes];
	boost::scoped_array<png_byte> sdata(data); 
	png_bytep* rowptrs = new png_bytep[height];
	boost::scoped_array<png_bytep> srowptrs(rowptrs); 

	for(int y=0;y<height;y++) {
		rowptrs[y] = data + ( y * rowbytes );
	}
	png_read_image(png, rowptrs);
	
	png_read_end(png, pnginfo); // read last information chunks
	png_destroy_read_struct(&png, &pnginfo, 0);

	fclose (fin);

	glGenTextures( 1, ( GLuint * ) & GLID);
	glBindTexture( GL_TEXTURE_2D,  GLID );
	bool makemipmap = false;
	if( type == TT_Texture )
	{
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
		makemipmap  = true;
	}else
	{
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
		if( type == TT_Sprite)
			makemipmap =true;
	}
		
	if( makemipmap )
	{
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST ); //GL_LINEAR
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
		gluBuild2DMipmaps( GL_TEXTURE_2D, bpp, width , height, format, GL_UNSIGNED_BYTE, data );
	}else
	{
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST ); //GL_LINEAR
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexImage2D( GL_TEXTURE_2D, 0, bpp, width , height, 0,	format, GL_UNSIGNED_BYTE, data );
	}
	CheckGlError("aeTexture::LoadPNG");
	return true;
}

bool CheckPOW2( int n )
{
	int n1 = 1;
	while( n1 < n ) n1<<=1;

	return ( n1 == n );
}
static const std::string inloddir[] = {
	    "bitmaps/",
		"sprites08/",
		"icons/",
		"fonts/"
};
bool aeTexture::LoadSpriteFromLod( )
{
	std::string fname = inloddir[type]+basename;
	int size;
	BYTE*data = LodManager.LoadFile( fname , &size);
	if(!data)
		return false;
	boost::scoped_array<BYTE> sdata(data); 
	int psize = *(int*)(data+0x0c);
	unsigned long unpsize = *(int*)(data+0x1c);
	width  = *(WORD*)(data+0x10);
	height = *(WORD*)(data+0x12);

	if( psize+0x20+height*8 != size )
		return false;
	BYTE*unpdata = new BYTE[unpsize];
	boost::scoped_array<BYTE> sunpdata(unpdata);
	if ( uncompress( unpdata, &unpsize , data + 0x20+height*8, psize ) != Z_OK )
		return false;
	int palindex = *(int*)(data+0x14);
	boost::format palname("bitmaps/pal%03d");
	palname	% palindex;
	int sizepal;
	BYTE *pal = LodManager.LoadFile( palname.str() , &sizepal);
	boost::scoped_array<BYTE> spal(pal);
	if( sizepal != 0x330 )
		return false;
	
	

	bpp = 4;
	BYTE * imgdata = new BYTE[width*height*4];
	boost::scoped_array<BYTE> simgdata(imgdata);
	memset(imgdata,0,width*height*4);
	for ( int i = 0; i< height;i++)
	{
		int off = *(int*)(data+0x20+i*8+4);
		int x1 = *(WORD*)(data+0x20+i*8+0);
		int x2 = *(WORD*)(data+0x20+i*8+2);
		int imgoff = ( height - i - 1)*width*4+(x1)*4;
		if( x1 < 0 || x2 < 0 || x1 > width || x2 > width || x2 < x1 )
			continue;
		for ( int j = 0 ; j < x2-x1+1 ; j++ )
		{
			int cindex = unpdata[off+j];
			imgdata[imgoff+j*4 + 0 ] = pal[0x30+cindex*3+0];
			imgdata[imgoff+j*4 + 1 ] = pal[0x30+cindex*3+1];
			imgdata[imgoff+j*4 + 2 ] = pal[0x30+cindex*3+2];
			if( cindex )
				imgdata[imgoff+j*4 + 3 ] = 0xff;
			else
				imgdata[imgoff+j*4 + 3 ] = 0;
		}
	}
	glGenTextures( 1, ( GLuint * ) & GLID);
	glBindTexture( GL_TEXTURE_2D,  GLID );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST ); //GL_LINEAR
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	int w = 1;while( w < width)w<<=1;
	int h = 1;while( h < height)h<<=1;
	if( w!=width || h != height )
	{
		BYTE* newimgdata = new BYTE[w*h*4];
		if(gluScaleImage(GL_RGBA,width,height, GL_UNSIGNED_BYTE, imgdata,
			w,h,GL_UNSIGNED_BYTE,newimgdata)!=0)
		{
			delete[] newimgdata;
			return false;
		}
		width = w;
		height = h;
		gluBuild2DMipmaps( GL_TEXTURE_2D, bpp, width , height, GL_RGBA, GL_UNSIGNED_BYTE, newimgdata );
		delete[] newimgdata;
	}else
	{
		gluBuild2DMipmaps( GL_TEXTURE_2D, bpp, width , height, GL_RGBA, GL_UNSIGNED_BYTE, imgdata   );
	}
	boost::format fmt(" w:%d h:%d ");
	fmt % width % height;
	Log << aeLog::debug << "aeTexture::LoadSpriteFromLod: Loading sprite "<< fname << fmt << aeLog::endl;
	CheckGlError("aeTexture::LoadSpriteFromLod");
	return true;
}
bool aeTexture::LoadFromLod( )
{
	if( type == TT_Sprite )
		return LoadSpriteFromLod();
	std::string fname = inloddir[type]+basename;
	int size;
	BYTE*data = LodManager.LoadFile( fname , &size);
	boost::scoped_array<BYTE> sdata(data); 
	if(!data)
		return false;
	int psize = *(int*)(data+0x14);
	int unpsize1 = *(int*)(data+0x10);
	unsigned long unpsize2 = *(int*)(data+0x28);
	if( psize+0x30+0x300 != size )
		return false;
	if( unpsize2 && unpsize2 < unpsize1)
		return false;
	BYTE* pal = data + 0x30 + psize;
	BYTE*unpdata = new BYTE[unpsize2 ];
	boost::scoped_array<BYTE> sunpdata(unpdata);


	if ( uncompress( unpdata, &unpsize2 , data + 0x30, psize ) != Z_OK )
		return false;
	
	
	width  = *(WORD*)(data+0x18);
	height = *(WORD*)(data+0x1a);
	bpp = 4;
	if( !CheckPOW2(width) || !CheckPOW2(height))
		return false;

	int imgsize = width*height;
	BYTE* rgbdata = new BYTE[imgsize*bpp];
	boost::scoped_array<BYTE> srgbdata(rgbdata);

	int w = width,h=height;

	glGenTextures( 1, ( GLuint * ) & GLID);
	glBindTexture( GL_TEXTURE_2D,  GLID );
	if( type == TT_Texture )
	{
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	}else
	{
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
	}
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST ); //GL_LINEAR
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
	//glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    int n=0,off=0; 
	for ( n = 0,off= 0; off < unpsize2 ;  n++)
	{
		if( w < 1 || h <1 )
			break;
		for ( int i = 0; i < imgsize; i++)
		{
			int index = unpdata[off+i];
			int r = pal[index*3+0];
			int g = pal[index*3+1];
			int b = pal[index*3+2];

			if( index == 0 && ((r == 0 && g >250 && b > 250) || (r > 250 && g ==0 && b > 250)))
			{
				r=g=b=0;
				rgbdata[i*bpp+3]= 0;
			}
			else
				rgbdata[i*bpp+3]= 0xff;
			rgbdata[i*bpp+0]= r;
			rgbdata[i*bpp+1]= g;
			rgbdata[i*bpp+2]= b;
		}
		off += imgsize;
		imgsize/=4;

		if( n == 0)	
			gluBuild2DMipmaps( GL_TEXTURE_2D, bpp, width , height, GL_RGBA, GL_UNSIGNED_BYTE, rgbdata  );
		else
    		glTexImage2D( GL_TEXTURE_2D, n, bpp, w , h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbdata );
		w/=2;
		h/=2;
	}
	boost::format fmt(" w:%d h:%d %d mipmaps");
	fmt % width % height % n;
	Log << aeLog::debug << "aeTexture::LoadFromLod: Loading texture "<< fname << fmt << aeLog::endl;
	CheckGlError("aeTexture::LoadFromLod");
	return true;
}
aeTexture::aeTexture( const std::string& tname, const std::string& basename,uTexType  textype, aeTextureManager* texman):
name( tname ), basename(basename), type(textype), texman(texman)
{
	if( type != TT_Font && LoadFromLod() )
		return;
	
	if( LoadPNG() )
		return;
	
	//default texture
	const static BYTE data[] = {  
		0,0xff,0xff,0xff,
		0,0xff,0xff,0xff,
		0,0xff,0xff,0xff,
		0,0xff,0xff,0xff
	};
	width = 2;
	height = 2;
	bpp = 4;
	Log << "Cannot Load texture " << tname << " generate default"<< aeLog::endl;
	glGenTextures( 1, ( GLuint * ) & GLID);
	glBindTexture( GL_TEXTURE_2D,  GLID );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST ); //GL_LINEAR
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, 4, 2, 2, 0, GL_RGBA	, GL_UNSIGNED_BYTE, data );
	glTexImage2D( GL_TEXTURE_2D, 1, 4, 1, 1, 0, GL_RGBA	, GL_UNSIGNED_BYTE, data );
	
//	throw error( (boost::format("aeTexture:Cannot Load Texture %s") % fname).str() );
}

aeTexture::~aeTexture()
{
	Disable();
	glDeleteTextures(1, &GLID);
	//texman->RemoveTexture( name );
	Log << aeLog::debug << "Remove texture "<< name << aeLog::endl;
}


aeTextureManager::aeTextureManager()
{
}

aeTextureManager::~aeTextureManager()
{
/*	texmap_iter_t ii = items.begin();
	for( ; ii != items.end(); ++ii )
	{
	        p_aeTexture p = (*ii).second.lock();
            if( p )
            {
                        p.reset();      	//текстура (*ii).first не освобождена
        	}
    }*/
}

p_aeTexture     aeTextureManager::GetTexture(const std::string& name, uTexType  textype)
{
	std::string tname( typenames[textype] + name );

	std::transform( tname.begin(), tname.end(), tname.begin(), tolower);

	texmap_iter_t ii = items.find( tname );

	if( ii != items.end() )
	{
		p_aeTexture p = (*ii).second.lock();
		if( p )
			return p;
		p = p_aeTexture(new aeTexture(tname, name, textype, this));
		(*ii).second = p;
		return p;
	}
	p_aeTexture p(new aeTexture(tname, name, textype, this));
	items[tname] = p;

	return p;
}

void            aeTextureManager::RemoveTexture( const std::string& name )
{
	texmap_iter_t ii = items.find( name );
	if( ii == items.end() )
	{
		Log << aeLog::debug << "RemoveTexture: текстура" << name << " не найдена" << aeLog::endl;
		return;
	}
	items.erase(ii);
}
