#include "stdafx.h"
#include <zlib.h>
#include <odmmap.h>

boost::scoped_ptr<ODMmap> pODMMap;
bool    ODMmap::SelectVersion()
{
	version = 0;
	if(!strcmp((char*)data+0x40, "MM6 Outdoor v1.11"))
	{
		version = 6;
		entsize = 0x1c;  
		spawnsize = 0x14;
		return true;
	}

	if(!strcmp((char*)data+0x40, "MM6 Outdoor v7.00"))
	{
		version = 7;
		entsize = 0x20;  
		spawnsize = 0x18;
		return true;
	}
	for ( int i = 0x40 ; i < 0x50 ; i++ )
	{
		if(data[i])
			return false;
	}

	version = 8;
	entsize = 0x20;  
	spawnsize = 0x18;
	return true;
}

bool ODMmap::LoadODM()
{
	if(!version)
		return false;
	int off = 0xb0;
	if( version == 8 )
		off += 4;
#define CHECK_OFF( xxx ) { off += (xxx);if( off > (int)datasize || off < 0 ) return false;}	
	{
		mastertile = data[0x5f];
		heightmap = data + off ; CHECK_OFF(0x4000);
		tilemap = data + off ; CHECK_OFF(0x4000);
		zeroedmap = data + off ; CHECK_OFF(0x4000);
		InitTerrain();
	}
	if( version >6 )
	{
		num_TerNorm = *(int*)(data+off); off+=4;
		CMAP1 = data + off ; CHECK_OFF(0x20000);
		CMAP2 = data + off ; CHECK_OFF(0x10000);
		TerNorm = (short*) (data + off) ; CHECK_OFF( num_TerNorm * 12 );
	}else
	{
		num_TerNorm = 0;
		CMAP1 = CMAP2 = NULL;
		TerNorm = NULL;
	}
	{
		num_bddata = *(int*)(data+off); off+=4;
		BDData  = data + off ; CHECK_OFF( num_bddata * 0xbc );
		BDData_facesdata = data + off ;
		bmodels.resize( num_bddata );
		for ( int i = 0 ; i < num_bddata ; i++ )
		{
			odm_bmodel_t& bm = bmodels[i];
			bm.hide=false;
			bm.bindata = BDData + i * 0xbc;
			
			bm.numvertex = *(int*)(bm.bindata + 0x44);
			bm.numfaces  = *(int*)(bm.bindata + 0x4c);
			bm.num3      = *(int*)(bm.bindata + 0x5c);
			bm.vertex_array = (int*)(data+off); CHECK_OFF( bm.numvertex  * 12 );

			bm.face_array     = data + off; CHECK_OFF( bm.numfaces * (0x134) );
			bm.face_unk_array = (short*)( data + off ); CHECK_OFF( bm.numfaces * 2 );
			bm.face_texnames_array = (char*)data +off; CHECK_OFF( bm.numfaces * 10 );
			bm.hz_array     = data + off; CHECK_OFF( bm.num3 * 8 );

			int* iptr = (int*)bm.bindata;
			const int bboxoff = 0x7c/4;
			bm.bbox = bbox_t( iptr[bboxoff ], iptr[bboxoff +1], iptr[bboxoff +2] ,
							  iptr[bboxoff +3], iptr[bboxoff +4], iptr[bboxoff +5]);
			const int o1off = 0x70/4;
			bm.origin1 = vec3_t( iptr[ o1off ], iptr[o1off +1], iptr[o1off +2]);
			const int o2off = 0xac/4;
			bm.origin2 = vec3_t( iptr[ o2off ], iptr[o2off +1], iptr[o2off +2]);
			std::copy( iptr+0x94/4,iptr+0x94/4+6, bm.unk0x94);
			bm.unk0xb8 = iptr[0xb8/4];
			bm.name1 = (char*)bm.bindata;
			bm.name2 = (char*)bm.bindata+0x20;

			bm.faces.resize( bm.numfaces );
			for ( int j = 0; j <  bm.numfaces ; j++)
			{
				odm_bmodel_face_t& face = bm.faces[j];
				face.bindata = bm.face_array + j * 0x134;
				face.unknown = bm.face_unk_array[j];
				face.texname = bm.face_texnames_array + j * 10;
				if( face.texname.size() )
					face.tex = TexManager.GetTexture( face.texname, TT_Texture);
				else
				{
					face.tex.reset();
				}

				face.plane.normal[0]	= (*(int*)(face.bindata +0))/65536.0;
				face.plane.normal[1]	= (*(int*)(face.bindata +4))/65536.0;
				face.plane.normal[2]	= (*(int*)(face.bindata +8))/65536.0;
				face.plane.dist		    = (*(int*)(face.bindata +0xc)/65536.0);
				short* sptr = (short*)face.bindata;
				int bboxoff = 0x116 /2 ;
				face.bbox = bbox_t( sptr[bboxoff +0], sptr[bboxoff +2], sptr[bboxoff +4],
									sptr[bboxoff +1], sptr[bboxoff +3], sptr[bboxoff +5]);
				
				
				face.numv	  = face.bindata[0x12e];
				face.d_texc_s = *(short*)( face.bindata + 0x112);
				face.d_texc_t = *(short*)( face.bindata + 0x114);
				int vindexoff  = 0x20/2;
				int texc_s_off = 0x20/2 + 0x14*1;
				int texc_t_off = 0x20/2 + 0x14*2;
				int hz1_off = 0x20/2 + 0x14*3;
				int hz2_off = 0x20/2 + 0x14*4;
				int hz3_off = 0x20/2 + 0x14*5;
				for ( int k = 0; k < face.numv+1; k++ )
				{
					face.texc[k][0] = sptr[texc_s_off+k] + face.d_texc_s;
					face.texc[k][1] = sptr[texc_t_off+k] + face.d_texc_t;
					if( face.texname.size() )
					{
						face.texc[k][0] /= face.tex->Width();
						face.texc[k][1] /= face.tex->Height();
					}
					face.hz[0][k] = sptr[hz1_off+k];
					face.hz[1][k] = sptr[hz2_off+k];
					face.hz[2][k] = sptr[hz3_off+k];
					int *v = bm.vertex_array + sptr[vindexoff+k] *3;
					face.vertexes[k][0] = v[0];
					face.vertexes[k][1] = v[1];
					face.vertexes[k][2] = v[2];
				}
			}
		}
	}
	{
		num_entites = *(int*)(data+off); off+=4;
		if( num_entites > 0xbb8)
			return false;
		entites  = data + off ; CHECK_OFF( num_entites*entsize );
		entnames = (char*)data + off ; CHECK_OFF( num_entites*0x20 );
		LoadSprites();
	}
	{
		num_idlist = *(int*)(data+off); off+=4;
		idlist  = data + off ; CHECK_OFF( num_idlist*2 );
	}
	{
		OMAP  = data + off ; CHECK_OFF( 0x10000 );
	}
	{
		num_spawn = *(int*)(data+off); off+=4;
		spawn  = data + off ; CHECK_OFF( num_spawn*spawnsize );
	}
	if( off != datasize)
	{
		return false;
	}
	return true;
}

ODMmap::ODMmap( BYTE * mdata, int size, const char *fname ):
		data(NULL),datasize(0),selected_bmodel(NULL),selected_face(NULL)
{
	Log <<  "Load ODM map " <<  fname << aeLog::endl;

	if ( *( DWORD * ) ( mdata ) == 0x16741 && *( DWORD * ) ( mdata + 4 ) == 0x6969766d )
	{
		int psize = *( int * ) ( mdata + 8 );

		if ( psize + 0x10 != size )
			throw error ("invalid ODM7 file\n");
		datasize = *( int * ) ( mdata + 12 );
		data = new BYTE[datasize];

		if ( uncompress( data, &datasize, mdata + 0x10, psize ) != Z_OK )
		{
			delete[]data;
			data = NULL;
			throw error ("Cannot uncompress ODM7\n");
		}
	} else
	{
		int psize = *( int * ) ( mdata );

		if ( psize + 0x8 != size )
			throw error ("invalid ODM6 file\n");
		datasize  = *( int * ) ( mdata + 4 );
		data = new BYTE[datasize];

		if ( uncompress( data, &datasize, mdata + 0x8, psize ) != Z_OK )
		{
			delete[]data;
			data = NULL;
			throw error ("Cannot uncompress ODM6\n");
		}
	}
	if(!SelectVersion())
	{
		delete[]data;
		data = NULL;
		throw error ("Cannot detect ODM map version\n");
	}
	if(!LoadODM())
	{
		delete[]data;
		data = NULL;
		throw error ("Cannot Load ODM map\n");
	}
	mapname = fname;

	pfont.reset( new aeFont("arial18"));
	pfont_small.reset( new aeFont("arial12"));
//	Log <<  "Loaded faces: " << faces.size() << ", vertexs: " << vertexes.size() << aeLog::endl;
}

ODMmap::~ODMmap()
{
	if( data )
		delete[]data;
}
void ODMmap::RenderTerrain(  )
{
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	//glDisable(GL_POLYGON_SMOOTH);
	glDisable( GL_CULL_FACE );

	int posx = (Camera.pos(0)/tilesize)+0x40;
	int posy = (-Camera.pos(1)/tilesize)+0x40;

	int minx = max(0, posx - zFar/tilesize);
	int maxx = min(0x7e, posx + zFar/tilesize);

	int miny = max(0, posy - zFar/tilesize);
	int maxy = min(0x7e, posy + zFar/tilesize);
	odm_terrain_t *p1 = TerrMap[minx]+miny;
	odm_terrain_t *p2 = p1+0x80;
	int delta = 0x80 - ( maxy - miny);
	int lastindex = -1;
	glDepthMask( FALSE );
	//draw water base
	for ( int x = minx; x < maxx; x++,p1+=delta,p2+=delta)
		for ( int y = miny; y < maxy; y++)
		{
			int index = (*p1).tile_index;
			if( !tiletexs2[index] )
			{
				++p1;++p2;
				continue;
			}
			if( lastindex != index )
				tiletexs2[lastindex = index]->Enable();

			glBegin( GL_TRIANGLE_STRIP );
			glTexCoord2f( 1,0 );
			glVertex3fv((*p2).p.vec_array);
			glTexCoord2f( 0,0 );
			glVertex3fv((*p1).p.vec_array);

			glTexCoord2f( 1,1 );
			glVertex3fv((*(++p2)).p.vec_array);
			glTexCoord2f( 0,1 );
			glVertex3fv((*(++p1)).p.vec_array);
			glEnd();
			triangles += 2;
		}
	glDepthMask( TRUE );
	p1 = TerrMap[minx]+miny;
	p2 = p1+0x80;
	
	lastindex = -1;
	for ( int x = minx; x < maxx; x++,p1+=delta,p2+=delta)
		for ( int y = miny; y < maxy; y++)
		{
			if( lastindex != (*p1).tile_index )
			{
				lastindex = (*p1).tile_index;
				tiletexs[lastindex]->Enable();
			}

			glBegin( GL_TRIANGLE_STRIP );
			glTexCoord2f( 1,0 );
			glVertex3fv((*p2).p.vec_array);
			glTexCoord2f( 0,0 );
			glVertex3fv((*p1).p.vec_array);
			
			glTexCoord2f( 1,1 );
			glVertex3fv((*(++p2)).p.vec_array);
			glTexCoord2f( 0,1 );
			glVertex3fv((*(++p1)).p.vec_array);
			glEnd();
			triangles += 2;
		}
		glEnable( GL_CULL_FACE );
//		glEnable( GL_TEXTURE_2D );
//		glEnable( GL_BLEND );
//		glDisable( GL_ALPHA );
        //glEnable(GL_POLYGON_SMOOTH);
}
void ODMmap::RenderBModels(  )
{
	glColor3f( 1, 1, 1 );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	glEnable( GL_TEXTURE_2D );
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	//glDisable( GL_CULL_FACE );
	glEnableClientState( GL_VERTEX_ARRAY);
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );

	if( selected_face )
	{
		selected_face->tex->Enable();
		glColor4f( 1,0,1, 1 );
		glVertexPointer( 3, GL_FLOAT, 0, selected_face->vertexes);
		glTexCoordPointer( 2, GL_FLOAT, 0, selected_face->texc);
		glDrawArrays( GL_TRIANGLE_FAN/*GL_POLYGON*/, 0, selected_face->numv );
		glColor3f( 1,1,1);
	}

	
	for ( std::vector<odm_bmodel_t>::const_iterator bmi = bmodels.begin(); bmi != bmodels.end() ; ++bmi)
	{
		const odm_bmodel_t& bm = *bmi;
		if( bm.hide )
			continue;

		if(Camera.CullBox( bm.bbox ))
			continue;
		for ( std::vector<odm_bmodel_face_t>::const_iterator facei = bm.faces.begin(); 
				facei!= bm.faces.end() ; ++facei)
		{
			const odm_bmodel_face_t& face = *facei;
			if( !face.tex )
				continue;
			/*if( face.bindata[0x1d]&0x20)
				continue;*/
			/*if( DotProduct(face.plane.normal, Camera.forward()) > 0 )
				continue;*/
			face.tex->Enable();
			glVertexPointer( 3, GL_FLOAT, 0, face.vertexes);
			glTexCoordPointer( 2, GL_FLOAT, 0, face.texc);
			glDrawArrays( GL_TRIANGLE_FAN/*GL_POLYGON*/, 0, face.numv );
			triangles += face.numv - 2;
		}
	}
	glDisableClientState( GL_VERTEX_ARRAY);
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );
	//glEnable( GL_CULL_FACE );
	if( selected_bmodel && !selected_bmodel->hide )
		RenderBModelBBox(*selected_bmodel);
	
}
void ODMmap::Render(  )
{
	triangles = 0;
	RenderTerrain();
	RenderBModels();
	RenderSprites();
}


void ODMmap::Draw2DInfo( int w, int h)
{
	pfont->DrawString( 0 , pfont->Size() , (boost::format("Map: %s") % mapname).str());
	pfont->DrawString( 0 , h - pfont->Size()*2 , (boost::format("Triangles: %d") % triangles ).str());

	int y = pfont->Size()*2 + pfont_small->Size();
	float dx = pfont_small->Size() * 0.55;
	int x = 0;
	if( selected_bmodel )
	{
		const odm_bmodel_t& bm = *selected_bmodel;
		{
			boost::format f("bmodel: %8x %s");
			f % ((bm.bindata - BDData) / 0xbc);
			f % bm.name1;
			pfont_small->DrawString( 0, y, f.str());
			y+=pfont_small->Size();
		}
		//bbox
		{
			x = 0;
			pfont_small->DrawString( x, y, "bbox:");
			x += dx*9;
			boost::format f("%5.0f");
			for (int i =0; i< 2 ; i++)
			{
				for ( int j = 0; j < 3; j++)
				{
					f % bm.bbox[i][j];
					pfont_small->DrawString( x, y, f.str());
					x+= dx*7;
				}
				x+= dx*1;
			}
			y+=pfont_small->Size();
		}
		//origins
		{
			x = 0;
			pfont_small->DrawString( x, y, "origin1:");
			x += dx*9;
			boost::format f("%5.0f");
			for ( int i = 0; i< 3; ++i)
			{
				f % bm.origin1[i];
				pfont_small->DrawString( x, y, f.str());
				x+= dx*7;
			}
			y+=pfont_small->Size();
		}
		{
			x = 0;
			pfont_small->DrawString( x, y, "origin2:");
			x += dx*9;
			boost::format f("%5.0f");
			for ( int i = 0; i< 3; ++i)
			{
				f % bm.origin2[i];
				pfont_small->DrawString( x, y, f.str());
				x+= dx*7;
			}
			y+=pfont_small->Size();
		}
		int *iptr = (int*)bm.bindata;
		{
			boost::format f("%6d");
			x = 0;
			pfont_small->DrawString( x, y, "40:");
			x += dx*5;
			f % iptr[ 0x40/4];
			pfont_small->DrawString( x, y, f.str());
			x += dx*7;
			y+=pfont_small->Size();

			x = 0;
			pfont_small->DrawString( x, y, "50:");
			x += dx*5;
			f % iptr[ 0x50/4];
			pfont_small->DrawString( x, y, f.str());
			x += dx*7;
			y+=pfont_small->Size();

			x = 0;
			pfont_small->DrawString( x, y, "60:");
			x += dx*5;
			for ( int i = 0; i <4 ;i++)
			{
				f % iptr[ 0x60/4 +i ];
				pfont_small->DrawString( x, y, f.str());
				x += dx*7;
			}
			y+=pfont_small->Size();

			x = 0;
			pfont_small->DrawString( x, y, "94:");
			x += dx*5;
			for (int i = 0; i < 6  ;i++)
			{
				f % iptr[ 0x94/4 +i ];
				pfont_small->DrawString( x, y, f.str());
				x += dx*7;
			}
			y+=pfont_small->Size();

			x = 0;
			pfont_small->DrawString( x, y, "B8:");
			x += dx*5;
			f % iptr[ 0xb8/4 ];
			pfont_small->DrawString( x, y, f.str());
			x += dx*7;
			y+=pfont_small->Size();
		}
		y+=pfont_small->Size();
	}
	if( selected_face )
	{
		const odm_bmodel_face_t& face = *selected_face;
		{
			boost::format f("off: %8x face: %s");
			f % (face.bindata -data);
			f % face.texname;
			pfont_small->DrawString( 0, y, f.str());
			y+=pfont_small->Size();
		}
		if( face.numv <5)
		{

			boost::format f("u: %f v: %f");
			for (int i =0; i<face.numv;i++)
			{
				f % (face.texc[i][0]);
				f % (face.texc[i][1]);
				pfont_small->DrawString( 0, y, f.str());
				y+=pfont_small->Size();
			}

		}
		{
			x = 0;
			pfont_small->DrawString( x, y, "plane:");
			x += dx*9;
			boost::format f("%.5f");
			for ( int i = 0; i< 3; ++i)
			{
				
				f % face.plane.normal[i];
				pfont_small->DrawString( x, y, f.str());
				x+= dx*8;
			}
			
			boost::format f1("%5.0f");
			f1 % face.plane.dist;
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
			boost::format f("event #:  %d");
			f % *(int*)(face.bindata+0x124);
			pfont_small->DrawString( 0, y, f.str());
			y+=pfont_small->Size();
		}
		{
			boost::format f("order:  %d");
			f % (int)face.unknown;
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
			pfont_small->DrawString( x, y, "110:");
			x += dx*5;
			f % (int)face.bindata[ 0x110];
			pfont_small->DrawString( x, y, f.str());
			x += dx*3;
			f % (int)face.bindata[ 0x111 ];
			pfont_small->DrawString( x, y, f.str());
			x = 0;
			y+=pfont_small->Size();
			pfont_small->DrawString( x, y, "120:");
			x += dx*5;
			for ( int i =0; i< 16; i++)
			{
				f % (int)face.bindata[ 0x120 +i];
				pfont_small->DrawString( x, y, f.str());
				x += dx*3;
			}
			x = 0;
			y+=pfont_small->Size();
			pfont_small->DrawString( x, y, "130:");
			x += dx*5;
			for ( int i =0; i< 4; i++)
			{
				f % (int)face.bindata[ 0x130 +i];
				pfont_small->DrawString( x, y, f.str());
				x += dx*3;
			}
			y+=pfont_small->Size();
		}
	} // selected_face
}
void	ODMmap::RenderBModelBBox(const odm_bmodel_t& bm)
{
	
	glDisable (GL_TEXTURE_2D);
	glDisable (GL_CULL_FACE);
	glDepthMask( FALSE );
	glDisable (GL_DEPTH_TEST);
	glEnable (GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f ( 0, 0, 1, 0.2);

	glBegin(GL_QUADS);
	glVertex3f( bm.bbox[0][0], bm.bbox[0][1], bm.bbox[0][2]);
	glVertex3f( bm.bbox[1][0], bm.bbox[0][1], bm.bbox[0][2]);
	glVertex3f( bm.bbox[1][0], bm.bbox[1][1], bm.bbox[0][2]);
	glVertex3f( bm.bbox[0][0], bm.bbox[1][1], bm.bbox[0][2]);

	glVertex3f( bm.bbox[0][0], bm.bbox[0][1], bm.bbox[0][2]);
	glVertex3f( bm.bbox[0][0], bm.bbox[1][1], bm.bbox[0][2]);
	glVertex3f( bm.bbox[0][0], bm.bbox[1][1], bm.bbox[1][2]);
	glVertex3f( bm.bbox[0][0], bm.bbox[0][1], bm.bbox[1][2]);

	glVertex3f( bm.bbox[0][0], bm.bbox[0][1], bm.bbox[0][2]);
	glVertex3f( bm.bbox[1][0], bm.bbox[0][1], bm.bbox[0][2]);
	glVertex3f( bm.bbox[1][0], bm.bbox[0][1], bm.bbox[1][2]);
	glVertex3f( bm.bbox[0][0], bm.bbox[0][1], bm.bbox[1][2]);


	glVertex3f( bm.bbox[1][0], bm.bbox[1][1], bm.bbox[1][2]);
	glVertex3f( bm.bbox[0][0], bm.bbox[1][1], bm.bbox[1][2]);
	glVertex3f( bm.bbox[0][0], bm.bbox[0][1], bm.bbox[1][2]);
	glVertex3f( bm.bbox[1][0], bm.bbox[0][1], bm.bbox[1][2]);

	glVertex3f( bm.bbox[1][0], bm.bbox[1][1], bm.bbox[1][2]);
	glVertex3f( bm.bbox[1][0], bm.bbox[0][1], bm.bbox[1][2]);
	glVertex3f( bm.bbox[1][0], bm.bbox[0][1], bm.bbox[0][2]);
	glVertex3f( bm.bbox[1][0], bm.bbox[1][1], bm.bbox[0][2]);

	glVertex3f( bm.bbox[1][0], bm.bbox[1][1], bm.bbox[1][2]);
	glVertex3f( bm.bbox[0][0], bm.bbox[1][1], bm.bbox[1][2]);
	glVertex3f( bm.bbox[0][0], bm.bbox[1][1], bm.bbox[0][2]);
	glVertex3f( bm.bbox[1][0], bm.bbox[1][1], bm.bbox[0][2]);
	glEnd();
	//glEnable (GL_DEPTH_TEST);
	glColor4f ( 1, 1, 0, 0.5);
	glBegin(GL_LINES);
	glVertex3f( bm.bbox[0][0], bm.bbox[0][1], bm.bbox[0][2]);
	glVertex3f( bm.bbox[0][0], bm.bbox[0][1], bm.bbox[1][2]);

	glVertex3f( bm.bbox[0][0], bm.bbox[0][1], bm.bbox[0][2]);
	glVertex3f( bm.bbox[0][0], bm.bbox[1][1], bm.bbox[0][2]);

	glVertex3f( bm.bbox[0][0], bm.bbox[0][1], bm.bbox[0][2]);
	glVertex3f( bm.bbox[1][0], bm.bbox[0][1], bm.bbox[0][2]);


	glVertex3f( bm.bbox[1][0], bm.bbox[1][1], bm.bbox[1][2]);
	glVertex3f( bm.bbox[1][0], bm.bbox[1][1], bm.bbox[0][2]);
	
	glVertex3f( bm.bbox[1][0], bm.bbox[1][1], bm.bbox[1][2]);
	glVertex3f( bm.bbox[1][0], bm.bbox[0][1], bm.bbox[1][2]);
	
	glVertex3f( bm.bbox[1][0], bm.bbox[1][1], bm.bbox[1][2]);
	glVertex3f( bm.bbox[0][0], bm.bbox[1][1], bm.bbox[1][2]);
//
	glVertex3f( bm.bbox[0][0], bm.bbox[0][1], bm.bbox[1][2]);
	glVertex3f( bm.bbox[0][0], bm.bbox[1][1], bm.bbox[1][2]);

	glVertex3f( bm.bbox[0][0], bm.bbox[1][1], bm.bbox[0][2]);
	glVertex3f( bm.bbox[0][0], bm.bbox[1][1], bm.bbox[1][2]);

	glVertex3f( bm.bbox[1][0], bm.bbox[0][1], bm.bbox[0][2]);
	glVertex3f( bm.bbox[1][0], bm.bbox[1][1], bm.bbox[0][2]);

	glVertex3f( bm.bbox[0][0], bm.bbox[0][1], bm.bbox[1][2]);
	glVertex3f( bm.bbox[1][0], bm.bbox[0][1], bm.bbox[1][2]);

	glVertex3f( bm.bbox[0][0], bm.bbox[1][1], bm.bbox[0][2]);
	glVertex3f( bm.bbox[1][0], bm.bbox[1][1], bm.bbox[0][2]);

	glVertex3f( bm.bbox[1][0], bm.bbox[0][1], bm.bbox[0][2]);
	glVertex3f( bm.bbox[1][0], bm.bbox[0][1], bm.bbox[1][2]);
	glEnd();


	glEnable (GL_TEXTURE_2D);
	glDepthMask( TRUE );
	glEnable (GL_DEPTH_TEST);
	glEnable (GL_CULL_FACE);
	glColor3f ( 1,1,1);

}

odm_bmodel_t* ODMmap::GetBModelOnCrossHair( float mindist )
{
	odm_bmodel_t* tmodel = NULL;
	for ( std::vector<odm_bmodel_t>::iterator bmi = bmodels.begin(); bmi != bmodels.end() ; ++bmi)
	{
		odm_bmodel_t& bm = *bmi;
		if( bm.hide )
			continue;
		float dist;
		if(isLineCrossBBox(Camera.pos(), Camera.forward(), bm.bbox, &dist))
		{
			if( mindist > dist )
			{
				tmodel =&(*bmi);
				mindist = dist;
			}
		}
	}
	return tmodel;
}

void    ODMmap::SelectFace()
{
	//odm_bmodel_t* bm = GetBModelOnCrossHair(zFar);
	
	//if( !bm )
		//return;
	
	vec3_t v,p1,p2;
	v= Camera.forward();
	p1 = Camera.pos();
	float maxdist = zFar;
	p2 = p1 + v* (maxdist);
	selected_face = NULL;
	for ( std::vector<odm_bmodel_t>::iterator bmi = bmodels.begin(); bmi != bmodels.end() ; ++bmi)
	{
		odm_bmodel_t& bm = *bmi;

		if( bm.hide )
			continue;

		if(Camera.CullBox( bm.bbox ))
			continue;

		for ( std::vector<odm_bmodel_face_t>::iterator facei = bm.faces.begin(); 
			facei!= bm.faces.end() ; ++facei)
		{
			odm_bmodel_face_t& face = *facei;

			if( Camera.CullBox( face.bbox ))
				continue;

			float dist1 = DotProduct(face.plane.normal, vec3_t(face.vertexes[0]) - p1);
			float dist2 = DotProduct(vec3_t(face.vertexes[0]) - p2, face.plane.normal);
			vec3_t w = v * zFar;

			if( DotProduct( v, face.plane.normal) >= 0)
				continue;

			float e = DotProduct( face.plane.normal , w);

			if( e ==0)
				continue;

			float dist3 = dist1 / e;
			
			if( dist3 * zFar > maxdist || dist3 < 0 )
				continue;

			vec3_t crosspoint = p1 + w * dist3;

			if( DotProduct( p1-crosspoint, p2-crosspoint)>0 )
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
			vec3_t basenormal = CrossProduct(face.vertexes[0] - crosspoint,face.vertexes[1] - crosspoint);
			bool q = true;
			for ( int j = 1; j < face.numv; j++)
			{
				vec3_t pp1 = face.vertexes[j] - crosspoint;
				vec3_t pp2 = face.vertexes[(j+1)%face.numv] - crosspoint;
				if( DotProduct(basenormal , CrossProduct(pp1,pp2)) < 0 )
				{
					q = false;
					break;
				}
			}
			if( !q )
				continue;

			maxdist = dist3 * zFar;
			selected_face = &(*facei);
		}
	}
}

void ODMmap::ProcessInput( int key )
{
	switch( key )
	{
	case K_MOUSE2:
		SelectFace();
		break;
	case K_MOUSE1:
		selected_bmodel = GetBModelOnCrossHair(zFar);
		break;
	case 'h':
		if(selected_bmodel)
			selected_bmodel->hide = (selected_bmodel->hide)?false:true;
		break;
	case 'j':
		{
			for ( std::vector<odm_bmodel_t>::iterator bmi = bmodels.begin(); bmi != bmodels.end() ; ++bmi)
			{
				(*bmi).hide = false;
			}

		}
		break;
	}
}


void    ODMmap::InitTerrain()
{
	InitTileSets();
	for ( int x = 0; x < 0x80; x++)
		for ( int y = 0; y < 0x80; y++)
		{
			TerrMap[x][y].p.x = (x-0x40)*tilesize;//-1;
			TerrMap[x][y].p.y = -(y-0x40)*tilesize;//+1;
			TerrMap[x][y].p.z = heightmap[y*0x80+x] * heightsize;
			TerrMap[x][y].tile_index = tilemap[y*0x80+x];
		}
}
void    ODMmap::InitTileSets()
{
#pragma pack(push,1)	
	typedef struct
	{
		char name[20];
		WORD hz[3];
	}dtile_t;
#pragma pack(pop)
	BYTE   *tiledata;
	if( version == 8 )
	{
		int size;
		char *tilename;
		
		switch( mastertile )
		{
			case 0: tilename = "language/dtile.bin";break;
			case 1: tilename = "language/dtile2.bin";break;
			case 2: tilename = "language/dtile3.bin";break;
			default: tilename = "language/dtile.bin";break;
		}
		BYTE   *ptiledata  = LodManager.LoadFile( tilename , &size);
		if( !ptiledata  )
			throw error ("Cannot load dtile.bin\n");

		unsigned long unpsize = *( unsigned long * ) ( ptiledata + 0x58 );
		int psize = *( int * ) ( ptiledata + 0x44 );

		if ( psize + 0x60 != size )
			throw error ("invalid dtile.bin file\n");
		tiledata =new BYTE[unpsize];
		if ( uncompress( tiledata, &unpsize, ptiledata + 0x60, psize ) != Z_OK )
		{
			delete[]tiledata;
			delete[]ptiledata;
			throw error ("Cannot uncompress dtile.bin\n");
		}
		delete[] ptiledata;
		
	}else
	{
		int size;
		BYTE   *ptiledata  = LodManager.LoadFile( "icons/dtile.bin", &size);
		if( !ptiledata  )
			throw error ("Cannot load dtile.bin\n");

		unsigned long unpsize = *( unsigned long * ) ( ptiledata + 0x28 );
		int psize = *( int * ) ( ptiledata + 0x14 );

		if ( psize + 0x30 != size )
			throw error ("invalid dtile.bin file\n");
		tiledata =new BYTE[unpsize];
		if ( uncompress( tiledata, &unpsize, ptiledata + 0x30, psize ) != Z_OK )
		{
			delete[]tiledata;
			delete[]ptiledata;
			throw error ("Cannot uncompress dtile.bin\n");
		}
		delete[] ptiledata;
	}
	int num = *(int*)(tiledata);
	dtile_t* tbl = (dtile_t*)(tiledata+4);

	char*name;
	int index;
	for ( int i = 0; i< 256; i++ )
	{
 		 if( i >= 0xc6 )
		 {
			  index =  i - 0xc6 +*(WORD*)(data+0xae);
		 }else
		 {
			 if( i < 0x5a )
			 {
				 name = tbl[i].name;
				 if(!name[0])
					 name = "pending";
				 index = i;
			 }else
			 {
				 int n = (i-0x5a)/0x24;
				 index = *(WORD*)(data+0xa2 +n*4) - n *0x24;
				 index += i-0x5a;
			}	
		 }
		 name = tbl[index].name;
		 if(!name[0])
			 name = "pending";
		 tiletexs[i] = TexManager.GetTexture( name, TT_Texture);
		 //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
		 //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
		 tiletexs[i]->Enable();
		 glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );//GL_LINEAR ); 
		 glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR );
		 //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		 //glTexParameteri( GL_TEXTURE_2D,  GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	      //GL_NEAREST_MIPMAP_NEAREST );
		 //GL_LINEAR_MIPMAP_LINEAR );


		 tiletexs2[i].reset();
		 if( tbl[index].hz[2] == 0x300)
		 {
			 int group = tbl[index].hz[0];
			 for (int j = 0 ;j<4;j++)
			 {
				 if(*(WORD*)(data+0xa0+j*4)==group )
				 {
					 int idx2 = *(WORD*)(data+0xa0+j*4+2);
					 char *name2 = tbl[idx2].name;
					 if( name2 && name2[0])
					 {
						 tiletexs2[i] = TexManager.GetTexture( name2, TT_Texture);
						 //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
						 //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
					 }
					 break;
				 }
			 }
		 }
	}
	delete[] tiledata;
}

void    ODMmap::LoadSprites()
{
	for ( int i = 0; i< num_entites; i++ )
	{
		std::string name = entnames + i*0x20;
		vec3_t org(*(int*)(entites+i*entsize + 4),
				 *(int*)(entites+i*entsize + 8),
				 *(int*)(entites+i*entsize + 12));
		if( name == "Party Start")
		{
			Camera.MoveTo(org + vec3_t(0,0,100));
			continue;
		}
		int flags = *(WORD*)(entites+i*entsize + 2);
		if( flags )
			continue;
		odm_sprite_t spr;
		spr.origin = org;
		spr.tex = TexManager.GetTexture( name, TT_Sprite);
		spr.radius = sqrt((float)(spr.tex->Width() * spr.tex->Width() + spr.tex->Height() *spr.tex->Height()));
		entlist.push_back( spr );
		//Log << aeLog::debug << "Loaded sprite " << name << " " << spr.tex->Width() << " " << spr.tex->Height() << aeLog::endl;
	}
}
typedef struct  
{
	odm_sprite_t* spr;
	float range;
}range_spr_t;
struct sort_by_range
{
	bool operator()(const range_spr_t & s1, const range_spr_t & s2) const
	{
		return (s1.range > s2.range);
	}
};

void    ODMmap::RenderSprites()
{
	static std::vector<range_spr_t> sortedspr;
	sortedspr.resize(entlist.size());

	std::vector<odm_sprite_t>::iterator ispr = entlist.begin();
	std::vector<range_spr_t>::iterator ispr2 = sortedspr.begin();
	for (; ispr!=entlist.end(); ++ispr)
	{
		odm_sprite_t& spr = *ispr;
		if(Camera.CullSphere(spr.origin, spr.radius))
			continue;
		float range = (Camera.pos() - spr.origin ).vlen();
		if( range > zFar )
			continue;
		(*ispr2).spr = &(*ispr) ;
		(*ispr2).range = range;
		++ispr2;
	}
	std::sort(sortedspr.begin(),ispr2,sort_by_range());


	glColor3f( 1, 1, 1 );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	glDisable( GL_CULL_FACE );
	glEnable( GL_TEXTURE_2D );
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

	std::vector<range_spr_t>::iterator ispr3 = sortedspr.begin();

	for ( ; ispr3!= ispr2; ++ispr3)
	{
		const odm_sprite_t& spr = *((*ispr3).spr);

		vec3_t p1 = spr.origin - Camera.right() * (spr.tex->Width()/2);
		vec3_t p2 = p1 + Camera.up() * spr.tex->Height();
		vec3_t p3 = p2 + Camera.right() * spr.tex->Width();
		vec3_t p4 = p1 + Camera.right() * spr.tex->Width();

		spr.tex->Enable();
		glBegin(GL_QUADS);
		glTexCoord2f( 0,0 );
		glVertex3fv( p1.vec_array );
		glTexCoord2f( 0,1 );
		glVertex3fv( p2.vec_array );
		glTexCoord2f( 1,1 );
		glVertex3fv( p3.vec_array );
		glTexCoord2f( 1,0 );
		glVertex3fv( p4.vec_array );

		glEnd();
	}
	glEnable( GL_CULL_FACE );
}