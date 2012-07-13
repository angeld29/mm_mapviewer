#include "stdafx.h"
#include <lodfile.h>

LodFile LodManager;
typedef struct
{
	char fname[0x10];
	int off,size,hz,num;
}lod_head_typ;

typedef struct
{
	char fname[0x10];
	int  off,size,hz,num;
}lod6_item_typ;

/*typedef struct
{
	char fname[0x10];
	int off,size,hz,num;
}lod7_item_typ;*/


typedef struct
{
	char fname[0x40];
	int  off,size,hz;
}lod8_item_typ;


LodFile::~LodFile()
{
	for ( int i = 0; i< paks.size() ; ++i)
	{
		if( paks[i].f )
			fclose(paks[i].f);
	}
}

bool  LodFile::AddLod( const std::string& fname )
{
	lod_file_t lfile;
	FILE*f;
	f = fopen( fname.c_str(), "rb" );
	
	Log << aeLog::debug << "AddLod: "<< fname << aeLog::endl;
	if(!f)
	{
		
		return false;
	}
	lfile.f = f;
	lfile.name = fname;
	char tmp[0x10];	
	fread(tmp,sizeof(tmp),1,f);
	int version = 6;
	if( strcmp(tmp,"LOD"))
	{
		fclose(f);
		return false;
	}
	if( !strcmp(tmp+4,"MMVIII"))
	{
		version = 8;
		fseek(f,0x100,0);
		lod_head_typ header;
		fread(&header,sizeof(header),1,f);
		std::string dirname = header.fname;
		lod8_item_typ *tbl = new lod8_item_typ[header.num];
		fread(tbl,header.num*sizeof(lod8_item_typ),1,f);
		int baseoff = header.off;
		for (int i =0; i < header.num;++i)
		{
			std::string tname = dirname+"/"+tbl[i].fname;
			std::transform( tname.begin(), tname.end(), tname.begin(), tolower);
			files[tname].off = baseoff+tbl[i].off;
			files[tname].size = tbl[i].size;
			files[tname].lodindex = paks.size();
		}
		delete[]tbl;
		paks.push_back(lfile);
		Log << "Added "<< header.num << " files from " << fname << aeLog::endl;
/*		fclose(f);
		return false;*/
	}else
	{
		fseek(f,0x100,0);
		lod_head_typ header;
		fread(&header,sizeof(header),1,f);
		std::string dirname = header.fname;
		lod6_item_typ *tbl = new lod6_item_typ[header.num];
		fread(tbl,header.num*sizeof(lod6_item_typ),1,f);
		int baseoff = header.off;

		for (int i =0; i < header.num;++i)
		{
			std::string tname = dirname+"/"+tbl[i].fname;
			std::transform( tname.begin(), tname.end(), tname.begin(), tolower);
			files[tname].off = baseoff+tbl[i].off;
			files[tname].size = tbl[i].size;
			files[tname].lodindex = paks.size();
		}
		delete[]tbl;
		paks.push_back(lfile);
		Log << "Added "<< header.num << " files from " << fname << aeLog::endl;
	}
	return true;
}

BYTE* LodFile::LoadFile( const std::string& fname, int* fsize)
{
	std::string tname(fname);
	std::transform( tname.begin(), tname.end(), tname.begin(), tolower);
	std::map<std::string, pak_item_t>::const_iterator ii = files.find( tname );
	if( ii == files.end() )
	{
		Log << "LodFile::LoadFile: file "<< tname << " not found" << aeLog::endl;
		*fsize = 0;
		return NULL;
	}
	*fsize = (*ii).second.size;
	FILE*f = paks[(*ii).second.lodindex].f;
	fseek(f,(*ii).second.off,0);
	BYTE *data = new BYTE[*fsize];
	fread(data,*fsize,1,f);
	return data;
}
int	  LodFile::GetFileList( const boost::regex& re, std::vector <std::string>* filelist)
{
	std::map<std::string, pak_item_t>::const_iterator ii = files.begin();
	int num = 0;
	for ( ; ii != files.end(); ++ii )
	{
		if( boost::regex_match( (*ii).first, re))
		{
			filelist->push_back( (*ii).first );
			num++;
		}
	}
	return num;
}