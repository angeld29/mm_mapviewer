#include "stdafx.h"
#include <mm_map.h>
#include <odmmap.h>
#include <aversion.h>
const GLubyte* gl_vendor;
const GLubyte* gl_renderer;
const GLubyte* gl_version;
const GLubyte* gl_extensions;

p_aeFont pfont,pfont_small;

void LoadMap(  );
void CheckGlError(  const char *str )
{
	char *errstr;
	GLenum  err = glGetError();
	switch( err )
	{
	case GL_NO_ERROR:
		return;
		break;
	case GL_INVALID_ENUM:
		errstr = "glError: GL_INVALID_ENUM";
		break;
	case GL_INVALID_VALUE:
		errstr =  "glError: GL_INVALID_VALUE";
		break;
	case GL_INVALID_OPERATION:
		errstr = "glError: GL_INVALID_OPERATION";
		break;
	case GL_STACK_OVERFLOW:
		errstr = "glError: GL_STACK_OVERFLOW";
		break;
	case GL_STACK_UNDERFLOW:
		errstr =  "glError: GL_STACK_UNDERFLOW";
		break;
	case GL_OUT_OF_MEMORY:
		errstr =  "glError: GL_OUT_OF_MEMORY";
		break;
	default:
		throw error( (boost::format("Unknown glError %d: %s") % err % str).str());
		return;
	}
	throw error( (boost::format("%s :%s") % errstr % str).str());
}
static void InitMap()
{
	char fname[0x1000];

	GetPrivateProfileString( "Game","mapname" , "", fname, sizeof(fname),".\\config.ini");

	int size;
	BYTE   *data  = LodManager.LoadFile( fname, &size);
	if ( !data )
	{
		LoadMap(  );
		return;
	}
	if( !stricmp( fname + strlen(fname)-4, ".odm"))
	{
		pODMMap.reset( new ODMmap( data, size, fname ));
		pMap.reset();
	}
	else
	{
		pMap.reset( new MMmap( data, size, fname ));
		pODMMap.reset();
	}
	delete[]data;
}

void GL_Init()
{
	int i;

	glGetIntegerv(GL_DEPTH_BITS, &i);;
	Log << "Z-buffer : " << i <<" bits" << aeLog::endl;

	gl_vendor = glGetString (GL_VENDOR);
	Log << "GL_VENDOR: " << gl_vendor << aeLog::endl;
	gl_renderer = glGetString (GL_RENDERER);
	Log << "GL_RENDERER: " << gl_renderer << aeLog::endl;
	gl_version = glGetString (GL_VERSION);
	Log << "GL_VERSION: " << gl_version << aeLog::endl;
	gl_extensions = glGetString (GL_EXTENSIONS);
	Log << "GL_EXTENSIONS: " << gl_extensions << aeLog::endl;

	pfont.reset( new aeFont("arial18"));
	pfont_small.reset( new aeFont("arial12"));
	InitMap();
	glEnable(GL_LINE_SMOOTH); 
	glHint( GL_LINE_SMOOTH_HINT, GL_NICEST);
	//glEnable(GL_POLYGON_SMOOTH);
	//glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	//glShadeModel(GL_SMOOTH);   
}
void GL_Shutdown()
{
	pMap.reset();
	pODMMap.reset();
	pfont.reset();
	pfont_small.reset();
}



void display()
{
//	extern bool skip_update;
//	if( skip_update )	return;
	GL_Window.InitGL3D(  );

	//glEnable( GL_DEPTH_TEST );
	glDepthMask( TRUE );
	//Очищаем буффер цвета и глубины
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glEnable(GL_TEXTURE_2D);
        glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE ); 
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

	
	glColor3f( 1, 1, 1 );
	if( pMap)
		pMap->Render();
	if( pODMMap )
		pODMMap->Render();

	glDisable( GL_TEXTURE_2D );
	glDisable( GL_BLEND );
	GL_Window.InitGL2D();
	int w = GL_Window.Width();
	int h = GL_Window.Height();
	glDisable( GL_TEXTURE_2D );
	glColor3f( 1, 1, 1 );
	int crosssize = 5;


	glBegin( GL_LINES );
	 glVertex2f( w/2 -crosssize, h/2  );
	 glVertex2f( w/2 +crosssize , h/2  );
 	 glVertex2f( w/2  , h/2-crosssize  );
	 glVertex2f( w/2  , h/2+crosssize  );
	 //compas
	 glVertex2f( w - 20 , 50  );
	 glVertex2f( w - 20 + Camera.forward(0)*20, 50 - Camera.forward(1)*20  );

	glEnd();
	glEnable( GL_TEXTURE_2D );

	glColor3f( 1, 1, 1 );
	pfont->DrawString( w - (pfont->Size() * 5) , pfont->Size() , 
		(boost::format("FPS %4d") % Timer->FPS()).str());
	
	pfont->DrawString( 0 , h - pfont->Size()*3, 
		(boost::format("Pos: %4.0f %4.0f %4.0f") % Camera.pos(0)% Camera.pos(1)% Camera.pos(2)).str());

	pfont_small->DrawString( w - (pfont_small->Size() * 22) , h - pfont_small->Size() *2 , 
		"Might&Magic6-8 map viewer by Angel.(C)2005");
	pfont_small->DrawString( w - (pfont_small->Size() * 22) , h - pfont_small->Size() /2 , 
		"http://angel-death.newmail.ru");

	pfont_small->DrawString( 0,  h - pfont_small->Size() /2 , 
		VERSION);

	if( pMap)
		pMap->Draw2DInfo( w , h);
	if( pODMMap )
		pODMMap->Draw2DInfo( w , h);
	GL_Window.EndFrame(  );
}
