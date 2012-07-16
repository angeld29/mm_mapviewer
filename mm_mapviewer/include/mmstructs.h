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

typedef mm_vertex_t short coords[3];

typedef struct  
{
	char	name1[0x20];
	char	name2[0x20];
	DWORD	attribute;
	int		vertex_count;
	DWORD   vertex_offset;
	int		faces_count;
	DWORD   convex_facets_count,faces_offset,ordering_offset;
	int		bsp_node_count;
	DWORD   bsp_node_offset;
	int		DECORATIONS_COUNT;
	int		centerx,centery; 
	//0x70
	int		origin[3];
	int		bbox[2][3];
	int		bbox_cell[2][3];
	int		bboxcenter[3];
	int		bbox_radius;
}mm_odm_bmodel_t;
typedef struct  
{
	int		normal[3],dist; // * 65536.0
	int		zcalc[2],zdist; // * 65536.0
	DWORD   attributes;
	//0x20
	short	vertex_index_list[0x14];
	short	texc_s_list[0x14];
	short	texc_t_list[0x14];
	short	intercept_displacement_list[3][0x14];
	//0x110
	short	bitmap_index;
	short	delta_texc_s,delta_texc_t;
	short	bbox[3][2]; //min x max x min y max y min z 
	//0x122
	short	cog_number,cog_triggered_number;
	WORD	cog_trigger_type,reserved;
	BYTE	gradient_vertex_list[4];
	BYTE	vertex_count, polygon_type;
	//0x130
	DWORD	unknown; // ???BYTE SHADE,VISIBILITY;	WORD hz;
}mm_odm_bmodel_face_t;

typedef mm_odm_bmodel_face_texname_t char[10];

