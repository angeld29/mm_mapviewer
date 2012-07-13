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