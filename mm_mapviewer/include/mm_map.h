#ifndef _ae_mmMap_FILE
#define _ae_mmMap_FILE
#include "stdafx.h"

typedef struct
{
	int v_idx;
	float   hz[3];
	float	tex_c[2];
	vec3_t	vertex;
}node_t;

typedef struct
{
	std::string texname;
	p_aeTexture tex;

	plane_t plane,plane7;
	bbox_t	bbox;
	int		areas[2];
	int		fparm_index;
	//unknown data
	BYTE	data0x10[0x10],b0x4c;
	WORD	b0x4e;

	std::vector<node_t> nodes;
	BYTE    *bindata;
	int		index,offv;
}face_t;

class MMmap
{
	BYTE    *data;
	unsigned long     datasize;

	short   *vertex_data;
	int		num_vertex;
	BYTE    *faces_array;
	int		num_faces;
	short   *facedata;
	int     facedatasize;
	BYTE    *facetextures;
	
	int		numfaceparms;
	BYTE    *faceparams1;
	BYTE    *faceparams2;
	
	int		num_sectors;
	BYTE	*sectorsdata;
	int		Rdatasize;
	BYTE	*sectorsRdata;
	int		RLdatasize;
	BYTE	*sectorsRLdata;
	int		num_sprites_hz;
	int		num_sprites;
	BYTE	*spritesdata;
	BYTE	*spritesnamesdata;
	int		num_lights;
	BYTE	*lightsdata;
	int		num_unk9;
	BYTE	*Unknown9data;
	int		num_spawn;
	BYTE	*spawndata;
	int		num_outline;
	BYTE	*mapoutlinedata;
	
	int		version;
	int		facesize;
	int		spritesize;
	int		lightsize;
	int		spawnsize;
	int		sectorsize;
	bool	showportals;
	
	std::vector<vec3_t> vertexes;
	std::vector<face_t> faces;
	face_t* selected_face;
	std::string mapname;
	
	bool    PrepareBLV();
	bool    DetectBLVVersion();
	void	LoadBLVMap();
	bool	isPortal( const   face_t & face );
	void	RenderPortals();
	p_aeFont pfont,pfont_small;

	void	SelectFace();
public:
	MMmap( BYTE * mdata, int size, const char *fname );
	~MMmap();
	const std::string& MapName(){ return mapname;}
	//int NumFaces(){ return faces.size(); }
	//int NumVertexs(){ return vertexes.size(); }
	int MapVersion() { return version;}
	void Render();
	void Draw2DInfo( int w, int h );
	void ProcessInput( int key );
};

extern boost::scoped_ptr<MMmap> pMap;
#endif //_ae_mmMap_FILE