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