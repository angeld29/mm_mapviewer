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
#ifndef _ae_mmLod_FILE
#define _ae_mmLod_FILE
#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <cstdio>
#include <boost/regex.hpp>
#include <Windows.h>


class LodFile
{
private:
	typedef struct 
	{
		FILE*		  f;
		std::string   name;
	}lod_file_t;
	typedef struct 
	{
		int lodindex,off,size;
	}pak_item_t;
	std::vector<lod_file_t> paks;
	std::map<std::string, pak_item_t> files;
public:
	LodFile(){}
	~LodFile();
	bool  AddLod( const std::string& fname );
	BYTE* LoadFile( const std::string& fname, int* fsize);
	int	  GetFileList( const boost::regex& re, std::vector <std::string>* filelist);	
protected:
	

};
extern LodFile LodManager;
#endif //_ae_mmLod_FILE