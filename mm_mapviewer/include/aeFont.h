#ifndef _ae_Font_FILE
#define _ae_Font_FILE
#include "stdafx.h"

typedef struct 
{
 char name[0x40];
 int fontsize,numchars;
 int ascender,descender,height;// ,line_gap;
 int max_advance_width;
}font_header_typ;

typedef struct 
{
 int charcode;
 float x1,y1,x2,y2;
 int ntex;
 int sx,sy;
 int hDx,hDy,hAdvance;
 int vDx,vDy,vAdvance;
}font_sym_typ;
class aeFont
{
  private:
        p_aeTexture tex;
        int defaultchar;
        font_header_typ header;
        std::map<int,font_sym_typ> tbl;
        inline const   font_sym_typ & GetChar( int charcode );
  public:
	    aeFont( const std::string& name );
        void DrawString(float x,float y,const std::string& str, int fixedwidth =0 );
		int	Size() { return header.fontsize;}
        ~aeFont(){}
};
typedef boost::scoped_ptr<aeFont> p_aeFont;
#endif //_ae_Font_FILE