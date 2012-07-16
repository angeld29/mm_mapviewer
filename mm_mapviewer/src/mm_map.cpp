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
#include <zlib.h>
#include <mm_map.h>

boost::scoped_ptr<MMmap> pMap;

/*struct sort_by_texname
{
	bool operator()(const face_t& s1, const face_t& s2) const
	{
		return (s1.texname > s2.texname);
	}
};*/
//std::sort(faces.begin(),faces.end(),sort_by_texname());

bool    MMmap::DetectBLVVersion()
{
	int		off = 0x88;
#define CHECK_OFF( xxx ) { Log << aeLog::debug <<boost::format( " off = %8x size = %8x")% off % (xxx) <<aeLog::endl;\
							off += (xxx);if( off > datasize || off < 0 ) return false;}	

	{//Vertexs
		Log.Print(aeLog::LOG_DEBUG,"Vertex");
		num_vertex = *(int*)(data+off); off+=4;
		vertex_data = (short*)(data+off);
		CHECK_OFF( num_vertex*6 );
	}
	{//Faces
		Log.Print(aeLog::LOG_DEBUG,"Faces");
		num_faces = *(int*)(data+off);  off+=4;
		faces_array = data+off;
		CHECK_OFF( num_faces * facesize );
		facedatasize = *(int*)(data+0x68);
		facedata = (short*)( data+off );
		CHECK_OFF( facedatasize );

		facetextures = data + off;
		CHECK_OFF( num_faces *0x0a );

		numfaceparms = *(int*)(data+off);  off+=4;
		faceparams1 = data +off;
		CHECK_OFF(numfaceparms *0x24);
		faceparams2 = data +off;
		CHECK_OFF(numfaceparms *0x0a);
	}
	{
		Log.Print(aeLog::LOG_DEBUG,"Sectors");
		num_sectors = *(int*)(data+off);off+=4;
		sectorsdata = data +off;
		CHECK_OFF(num_sectors*sectorsize);

		Rdatasize = *(int*)(data+0x6c);
		sectorsRdata = data + off;
		CHECK_OFF(Rdatasize);
		RLdatasize = *(int*)(data+0x70);
		sectorsRLdata = data + off;
		CHECK_OFF(RLdatasize);
	}
	{
		Log.Print(aeLog::LOG_DEBUG,"Sprites");
		num_sprites_hz = *(int*)(data+off); off+=4;
		num_sprites = *(int*)(data+off);off+=4;
		spritesdata = data + off;
		CHECK_OFF(num_sprites*spritesize);
		spritesnamesdata = data + off;
		CHECK_OFF(0x20 * num_sprites);
	}
	{
		Log.Print(aeLog::LOG_DEBUG,"Lights");
		num_lights = *(int*)(data+off); off+=4;
		lightsdata = data + off;
		CHECK_OFF(num_lights*lightsize);
	}
	{
		Log.Print(aeLog::LOG_DEBUG,"Unk9");
		num_unk9 = *(int*)(data+off); off+=4;
		Unknown9data = data + off;
		CHECK_OFF(num_unk9*8);
	}
	{
		Log.Print(aeLog::LOG_DEBUG,"Spawn");
		num_spawn = *(int*)(data+off); off+=4;
		spawndata = data + off;
		CHECK_OFF(num_spawn*spawnsize);
	}
	{
		Log.Print(aeLog::LOG_DEBUG,"Outline");
		num_outline = *(int*)(data+off); off+=4;
		mapoutlinedata= data + off;
		CHECK_OFF(num_outline*0xc);
	}
	if( off != datasize )return false;
	return true;
}
bool    MMmap::PrepareBLV()
{
	Log.Print(aeLog::LOG_DEBUG,"Try BLV6");
	version		= 	 6;
	facesize	= 0x50;  
	sectorsize	= 0x74;
	spritesize	= 0x1c;
	lightsize	= 0x0C; 
	spawnsize	= 0x14;
	if( DetectBLVVersion() ) return true;

	Log.Print(aeLog::LOG_DEBUG,"Try BLV7");
	version		= 	 7;
	facesize	= 0x60;  
	sectorsize	= 0x74;
	spritesize	= 0x20;
	lightsize	= 0x10; 
	spawnsize	= 0x18;
	if( DetectBLVVersion() ) return true;

	Log.Print(aeLog::LOG_DEBUG,"Try BLV8");
	version		= 	 8;
	facesize	= 0x60;  
	sectorsize	= 0x78;
	spritesize	= 0x20;
	lightsize	= 0x14; 
	spawnsize	= 0x18;
	if( DetectBLVVersion() ) return true;
	return false;
}
void MMmap::LoadBLVMap( )
{
	{// get vertexs
		vertexes.resize(num_vertex);

		for ( int i = 0; i < vertexes.size(); i++ )
		{
			vertexes[i].x = vertex_data[i*3+0];
			vertexes[i].y = vertex_data[i*3+1];
			vertexes[i].z = vertex_data[i*3+2];
		}
	}
	{//faces
		faces.resize( num_faces );

		short  *v = ( short * ) facedata;

		for ( int i = 0; i < faces.size(); i++ )
		{
			
			face_t& face = faces[i];
			
			face.texname = (char*)facetextures + i * 0xa;
			face.bindata = faces_array + i * facesize;
			if( version > 6) 
			{
				face.plane7.normal[0]	= (*(float*)(face.bindata+ 0));
				face.plane7.normal[1]	= (*(float*)(face.bindata+ 4));
				face.plane7.normal[2]	= (*(float*)(face.bindata+ 8));
				face.plane7.dist		= (*(float*)(face.bindata+ 0xc));
				face.bindata +=0x10;
			}
			face.plane.normal[0]	= (*(int*)(face.bindata +0))/65536.0;
			face.plane.normal[1]	= (*(int*)(face.bindata +4))/65536.0;
			face.plane.normal[2]	= (*(int*)(face.bindata +8))/65536.0;
			face.plane.dist		    = (*(int*)(face.bindata +0xc)/65536.0);

			if( version == 6 )
				face.plane7 = face.plane;

			face.index = i;
			face.bbox.mins[0] = *(short*)(face.bindata + 0x40);
			face.bbox.maxs[0] = *(short*)(face.bindata + 0x42);
			face.bbox.mins[1] = *(short*)(face.bindata + 0x44);
			face.bbox.maxs[1] = *(short*)(face.bindata + 0x46);
			face.bbox.mins[2] = *(short*)(face.bindata + 0x48);
			face.bbox.maxs[2] = *(short*)(face.bindata + 0x4a);
			face.areas[0] = *(short*)(face.bindata + 0x3c);
			face.areas[1] = *(short*)(face.bindata + 0x3c+2);

			int numv = face.bindata[0x4d];

            if( numv < 3 )
			{
				Log << aeLog::debug <<"Face " << i << " has  < 3 nodes off:"<< face.bindata - data << aeLog::endl;
			}
			face.nodes.resize( numv );
			face.fparm_index = *(WORD*)(face.bindata + 0x38);
			
			if( face.fparm_index  > numfaceparms )
				Log << aeLog::debug <<"Face " << i << " has invalid fparm_index = "<< face.fparm_index << aeLog::endl;
			short ds = *(short*)(faceparams1+face.fparm_index*0x24+0x14);
			short dt = *(short*)(faceparams1+face.fparm_index*0x24+0x14 + 2);

			if( face.texname.size() )
				face.tex = TexManager.GetTexture( face.texname, TT_Texture);
			else
				face.tex.reset();
			for ( int j = 0; j < numv; j++ )
			{
				node_t &node = face.nodes[j];
				node.v_idx = v[j];
				node.tex_c[0] = v[( numv + 1 ) * 4 + j] + ds;
				node.tex_c[1] = v[( numv + 1 ) * 5 + j] + dt; 
				if( face.texname.size() )
				{
					node.tex_c[0]/= face.tex->Width();
					node.tex_c[1]/= face.tex->Height();
				}
				node.vertex = vertexes[ node.v_idx ];
				node.hz[0] = v[( numv + 1 ) * 1 + j];
				node.hz[1] = v[( numv + 1 ) * 2 + j];
				node.hz[2] = v[( numv + 1 ) * 3 + j];
			}
			face.offv = ((BYTE*)v) - data;
			v += ( numv + 1 ) * 6;
		}
		
	}
}

MMmap::MMmap( BYTE * mdata, int size, const char *fname ):data(NULL),datasize(0),selected_face(NULL),showportals(false)
{
	Log <<  "Load map " <<  fname << aeLog::endl;

	if ( *( DWORD * ) ( mdata ) == 0x16741 && *( DWORD * ) ( mdata + 4 ) == 0x6969766d )
	{
		int psize = *( int * ) ( mdata + 8 );

		if ( psize + 0x10 != size )
			throw error ("invalid blv7 file\n");
		datasize = *( int * ) ( mdata + 12 );
		data = new BYTE[datasize];

		if ( uncompress( data, &datasize, mdata + 0x10, psize ) != Z_OK )
		{
			delete[]data;
			data = NULL;
			throw error ("Cannot uncompress blv7\n");
		}
		
		if(!PrepareBLV() )
		{
			delete[] data;
			throw error ("Cannot detect blv version with pak version 7\n");
		};
		if( version < 7 )
			Log << "Warning blv" << version <<"  map  in pak version 7+" <<aeLog::endl;
	} else
	{
		int psize = *( int * ) ( mdata );

		if ( psize + 0x8 != size )
			throw error ("invalid blv6 file\n");
		datasize  = *( int * ) ( mdata + 4 );
		data = new BYTE[datasize];

		if ( uncompress( data, &datasize, mdata + 0x8, psize ) != Z_OK )
		{
			delete[]data;
			data = NULL;
			throw error ("Cannot uncompress blv6\n");
		}
		if(!PrepareBLV() )
		{
			delete[] data;
			throw error ("Cannot detect blv version with pak version 6\n");
		};
		if( version != 6 )
			Log << "Warning blv" << version <<"  map  in pak version 6" <<aeLog::endl;
	}
	LoadBLVMap( );

	mapname = fname;
	pfont.reset( new aeFont("arial18"));
	pfont_small.reset( new aeFont("arial12"));
	Log <<  "Loaded faces: " << faces.size() << ", vertexs: " << vertexes.size() << aeLog::endl;
}

MMmap::~MMmap()
{
	if( data )
		delete[]data;

	vertexes.clear();
	faces.clear();
}

bool MMmap::isPortal( const face_t & face )
{
	if ( face.texname.size(  ) < 1 )
		return true;
	if ( face.bindata[0x1c] & 1 )
		return true;
	return false;
}


void MMmap::RenderPortals(  )
{
	if ( !showportals )
		return;
	glDisable( GL_CULL_FACE );
	glDisable(GL_TEXTURE_2D); 
	glDisable( GL_BLEND );
	glColor3f( 1, 0, 1 );

	glEnableClientState( GL_VERTEX_ARRAY);
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );

	if( selected_face && isPortal( *selected_face ))
	{
//		selected_face->tex->Enable();
		glColor4f( 0, 1, 0, 1 );
		glVertexPointer( 3, GL_FLOAT, sizeof(node_t), selected_face->nodes[0].vertex.vec_array);
		glDrawArrays( GL_TRIANGLE_FAN/*GL_POLYGON*/, 0, selected_face->nodes.size());
		glColor3f( 1, 0, 1 );
	}
	for ( int i = 0; i < faces.size(  ); i++ )
	{
		const face_t & face = faces[i];
		if(face.nodes.size() <3 )
			continue;
		if ( !isPortal( face ) )
			continue;
		//glFrontFace( GL_CW );
		glVertexPointer( 3, GL_FLOAT, sizeof(node_t), face.nodes[0].vertex.vec_array);
		glDrawArrays( GL_TRIANGLE_FAN/*GL_POLYGON*/, 0, face.nodes.size());
		
	}

	glDisableClientState( GL_VERTEX_ARRAY);
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );

	glEnable( GL_BLEND );
	glEnable( GL_CULL_FACE );
	glEnable( GL_TEXTURE_2D ); 
}

void MMmap::Render(  )
{

	glColor3f( 1, 1, 1 );

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	//glEnable( GL_LINE_SMOOTH );

	glEnable( GL_TEXTURE_2D );
	//glDisable( GL_TEXTURE_2D );
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

	
	glEnableClientState( GL_VERTEX_ARRAY);
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );
	if( selected_face && !isPortal( *selected_face ))
	{
		selected_face->tex->Enable();
		glColor4f( 1,0,1, 1 );
		glVertexPointer( 3, GL_FLOAT, sizeof(node_t), selected_face->nodes[0].vertex.vec_array);
		glTexCoordPointer( 2, GL_FLOAT,sizeof(node_t), selected_face->nodes[0].tex_c);
		glDrawArrays( GL_TRIANGLE_FAN/*GL_POLYGON*/, 0, selected_face->nodes.size());
		glColor3f( 1,1,1);
	}

	for( std::vector<face_t>::const_iterator i = faces.begin() ; i != faces.end(); ++i)
	{
		const face_t &face = *i;
		if(face.nodes.size() <3 )
 			 continue;

		if ( isPortal( face ) )
			continue;
		face.tex->Enable();

		glVertexPointer( 3, GL_FLOAT, sizeof(node_t), face.nodes[0].vertex.vec_array);
		glTexCoordPointer( 2, GL_FLOAT,sizeof(node_t), face.nodes[0].tex_c);
		glDrawArrays( GL_TRIANGLE_FAN/*GL_POLYGON*/, 0, face.nodes.size());
	}
	glDisableClientState( GL_VERTEX_ARRAY);
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );
	RenderPortals(  );
}

void MMmap::SelectFace()
{
	vec3_t v,p1,p2;
	v= Camera.forward();
	p1 = Camera.pos();
	float maxdist = zFar;
	p2 = p1 + v* (maxdist);
	
	selected_face = NULL;
	for( std::vector<face_t>::iterator i = faces.begin() ; i != faces.end(); ++i)
	{
		face_t &face = *i;

		if(face.nodes.size() <3 )
		        continue;

		bool _isPortal = isPortal( face );
		if ( !showportals &&  _isPortal )
			continue;

		if( !_isPortal  && DotProduct(v,face.plane7.normal) >= 0)
			continue;

		if( Camera.CullBox( face.bbox ))
			continue;

		float dist1 = DotProduct(face.plane7.normal, face.nodes[0].vertex-p1);
		float dist2 = DotProduct(face.nodes[0].vertex-p2,face.plane7.normal);
		vec3_t w = v * zFar;

		float e = DotProduct( face.plane7.normal , w);

		if( e == 0)
			continue;

		float dist3 = dist1 / e;

		if( dist3 * zFar > maxdist || dist3 < 0 )
			continue;

		vec3_t crosspoint = p1 + w * dist3;

		if( DotProduct( p1-crosspoint, p2-crosspoint) > 0 )
			continue;

		if( crosspoint[0] < face.bbox[0][0] )
			continue;
		if( crosspoint[1] < face.bbox[0][1] )
			continue;
		if( crosspoint[2] < face.bbox[0][2] )
			continue;

		if( crosspoint[0] > face.bbox[1][0] )
			continue;
		if( crosspoint[1] > face.bbox[1][1] )
			continue;
		if( crosspoint[2] > face.bbox[1][2] )
			continue;
		vec3_t basenormal = CrossProduct(face.nodes[0].vertex - crosspoint,face.nodes[1].vertex - crosspoint);		bool q = true;
		for ( int j = 1; j < face.nodes.size(); j++)
		{
			vec3_t pp1 = face.nodes[j].vertex - crosspoint;
			vec3_t pp2 = face.nodes[(j+1)%face.nodes.size()].vertex - crosspoint;
			if( DotProduct(basenormal , CrossProduct(pp1,pp2)) < 0 )
			{
				q = false;
				break;
			}
		}
		if( !q )
			continue;
		maxdist = dist3 * zFar;
		selected_face = &(*i);
	}
}

void MMmap::Draw2DInfo( int w, int h)
{
	pfont->DrawString( 0 , pfont->Size() , (boost::format("Map: %s") % mapname).str());
	pfont->DrawString( 0 , h - pfont->Size()*2 , (boost::format("Faces:%6d") % faces.size()).str());
	int y = pfont->Size()*2 + pfont_small->Size();
	float dx = pfont_small->Size() * 0.55;
	int x = 0;

	if( selected_face )
	{
		const face_t& face = *selected_face;
		{
			boost::format f("face: %s");
			f % face.texname;
			pfont_small->DrawString( 0, y, f.str());
			y+=pfont_small->Size();
		}
		{
			x = 0;
			pfont_small->DrawString( x, y, "plane:");
			x += dx*9;
			boost::format f("%.5f");
			for ( int i = 0; i< 3; ++i)
			{

				f % face.plane7.normal[i];
				pfont_small->DrawString( x, y, f.str());
				x+= dx*8;
			}

			boost::format f1("%5.0f");
			f1 % face.plane7.dist;
			pfont_small->DrawString( x, y, f1.str());
			x+= dx*9;
			y+=pfont_small->Size();
		}
		{
			x = 0;
			pfont_small->DrawString( x, y, "bbox:");
			x += dx*9;
			boost::format f("%5.0f");
			for (int i =0; i< 2 ; i++)
			{
				for ( int j = 0; j < 3; j++)
				{
					f % face.bbox[i][j];
					pfont_small->DrawString( x, y, f.str());
					x+= dx*6;
				}
				x+= dx*1;
			}
			y+=pfont_small->Size();
		}

		{
			boost::format f("areas: %d  %d");
			f % face.areas[0] % face.areas[1];
			pfont_small->DrawString( 0, y, f.str());
			y+=pfont_small->Size();
		}
		{
			x = 0;
			pfont_small->DrawString( x, y, "unk0x10:");
			x += dx*9;
			boost::format f("%.5f");
			for ( int i = 0; i< 2; ++i)
			{
				f % (*(int*)(face.bindata + 0x10 + i*4)/65536.0);
				pfont_small->DrawString( x, y, f.str());
				x+= dx*9;
			}
			boost::format f1("%5.0f");
			f1 % (*(int*)(face.bindata + 0x10 + 8)/65536.0);
			pfont_small->DrawString( x, y, f1.str());
			x+= dx*6;

			y+=pfont_small->Size();
		}
		{
			x = 0;
			pfont_small->DrawString( x, y, "010:");
			x += dx*5;
			boost::format f("%02X");
			for ( int i =0; i< 16; i++)
			{
				f % (int)face.bindata[ 0x10 +i ];
				pfont_small->DrawString( x, y, f.str());
				x += dx*3;
			}
			x = 0;
			y+=pfont_small->Size();

			pfont_small->DrawString( x, y, "020:");
			x += dx*5;
			for ( int i =0; i< 16; i++)
			{
				f % (int)face.bindata[ 0x20 +i ];
				pfont_small->DrawString( x, y, f.str());
				x += dx*3;
			}
			x = 0;
			y+=pfont_small->Size();

			pfont_small->DrawString( x, y, "03A:");
			x += dx*5;
			f % (int)face.bindata[ 0x3A];
			pfont_small->DrawString( x, y, f.str());
			x += dx*3;
			f % (int)face.bindata[ 0x3B ];
			pfont_small->DrawString( x, y, f.str());
			x = 0;
			y+=pfont_small->Size();

			pfont_small->DrawString( x, y, "04C:");
			x += dx*5;
			for ( int i =0; i< 4; i++)
			{
				f % (int)face.bindata[ 0x4c +i];
				pfont_small->DrawString( x, y, f.str());
				x += dx*3;
			}
			x = 0;
			y+=pfont_small->Size();
		}
		{
			boost::format f("faceparm #:  %d");
			f % face.fparm_index;
			pfont_small->DrawString( 0, y, f.str());
			y+=pfont_small->Size();
			BYTE* fparmfdata = faceparams1  + face.fparm_index*36;
			boost::format fa("event #:  %d");
			fa % *(int*)(fparmfdata+0x1a);
			pfont_small->DrawString( 0, y, fa.str());
			
			boost::format f1("%03X:");
			boost::format f2("%02X");
			
			for ( int i = 0; i< 36;i++ )
			{
				if( i%16 ==0 )
				{
					y+=pfont_small->Size();
					x = 0;
					f1 % (i);
					pfont_small->DrawString( x, y, f1.str());
					x+= dx*5;
				}
				f2 % (int)fparmfdata[i];
				pfont_small->DrawString( x, y, f2.str());
				x+= dx*3;
			}
			y+=pfont_small->Size();
			
			pfont_small->DrawString( 0, y, "faceparms2:");
			x = dx * 12;
			y+=pfont_small->Size();
			for (int  i = 0; i< 10;i++ )
			{
				f2 % (int)faceparams2[face.fparm_index*10+i];
				pfont_small->DrawString( x, y, f2.str());
				x+= dx*3;
			}
			y+=pfont_small->Size();
		}
	} // selected_face
}

void MMmap::ProcessInput( int key )
{
	switch( key )
	{
	case K_MOUSE2:
		SelectFace();
		break;
	case 'p':
		showportals = (showportals)? 0:1;
		break;
	}
}