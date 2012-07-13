#ifndef _ae_ODMMap_FILE
#define _ae_ODMMap_FILE
#include "stdafx.h"

typedef struct
{
	BYTE*	bindata;
	std::string texname;
	p_aeTexture tex;

	plane_t plane;
	bbox_t	bbox;
	int		numv;
	//vec3_t  vertexes[0x14];
	float	vertexes[0x14][3];
	float   texc[0x14][2];
	float	hz[3][0x14];
	float	d_texc_s,d_texc_t;
	short	unknown;
}odm_bmodel_face_t;
typedef struct
{
	BYTE	*bindata;
	int		*vertex_array;
	BYTE	*face_array;
	short*	face_unk_array;
	char*	face_texnames_array;
	BYTE	*hz_array;
	int		numvertex,numfaces,num3;
	bbox_t	bbox;
	vec3_t	origin1,origin2;
	int		unk0x94[6],unk0xb8;
	std::string						name1,name2;
	std::vector<odm_bmodel_face_t>  faces;
	bool	hide;
}odm_bmodel_t;
typedef struct
{
	p_aeTexture tex;
	vec3_t origin;
	float  radius;
}odm_sprite_t;

typedef struct
{
	vec3_t p;
	int	tile_index;
}odm_terrain_t;
class ODMmap
{
	BYTE    *data;
	unsigned long     datasize;
	BYTE	mastertile;
	static const int tilesize = 512;
	static const int heightsize = 32;

	odm_terrain_t	TerrMap[0x80][0x80];
	BYTE*   heightmap;
	BYTE*   tilemap;
	BYTE*   zeroedmap;

	int     num_TerNorm;
	BYTE*   CMAP1;
	BYTE*   CMAP2;
	short*   TerNorm;

	int     num_bddata;
	BYTE*   BDData;
	BYTE*   BDData_facesdata;
	
	std::vector<odm_bmodel_t> bmodels;
	odm_bmodel_t* selected_bmodel;
	odm_bmodel_face_t*selected_face;

	int     num_entites;
	BYTE*   entites;
	char*   entnames;
	std::vector<odm_sprite_t> entlist;

	int     num_idlist;
	BYTE*   idlist;

	BYTE*   OMAP; // size = 0x10000  
	int     num_spawn;
	BYTE*   spawn;

	int version;
	int entsize;
	int spawnsize;
	
	std::string mapname;
	p_aeFont pfont,pfont_small;
	int		triangles;
	p_aeTexture tiletexs[256];
	p_aeTexture tiletexs2[256];

	bool    SelectVersion();
	bool	LoadODM();
	void	InitTerrain();
	void    InitTileSets();
	void    LoadSprites();
	void    RenderSprites();
	void    RenderTerrain();
	void    RenderBModels();
	void	RenderBModelBBox(const odm_bmodel_t& bm);
	void    SelectFace();
	odm_bmodel_t* GetBModelOnCrossHair( float mindist );
public:
	ODMmap( BYTE * mdata, int size, const char *fname );
	~ODMmap();
	const std::string& MapName(){ return mapname;}
	int MapVersion() { return version;}
	void Render();
	void Draw2DInfo( int w, int h );
	void ProcessInput( int key );
};

extern boost::scoped_ptr<ODMmap> pODMMap;
#endif //_ae_ODMMap_FILE