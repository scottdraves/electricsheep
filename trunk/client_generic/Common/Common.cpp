#ifdef WIN32
#include <windows.h>
#else
#include <dirent.h>
#endif

#include <iostream>
#include <vector>
#include <string>
#include <boost/filesystem.hpp>
#include "Log.h"

namespace	Base
{

using namespace boost::filesystem;

bool GetFileList( std::vector<std::string> &_list, const std::string _dir, const std::string _extension, const bool _usegoldsheep, const bool _usefreesheep )
{
	bool gotSheep = false;
	try {
	boost::filesystem::path p(_dir);

	directory_iterator end_itr; // default construction yields past-the-end
	for ( directory_iterator itr( p );
			itr != end_itr;
			++itr )
	{
		std::string dirname(itr->path().filename());
		if (is_directory(itr->status()))
		{
			gotSheep |= GetFileList( _list, (itr->path().string() + std::string("/")), _extension, _usegoldsheep, _usefreesheep );
		}
		else
		{
			std::string fname(itr->path().filename());
			if (itr->path().extension() == std::string(".avi"))
			{
				int generation;
				int id;
				int first;
				int last;

				if( 4 == sscanf( fname.c_str(), "%d=%d=%d=%d.avi", &generation, &id, &first, &last ) )
				{
					std::string xxxname(fname);
					xxxname.replace(fname.size() - 3, 3, "xxx");
				
					if ( !exists( p/xxxname ) ) // is it deleted?
					{
						if ( (_usegoldsheep && generation >= 10000) || (_usefreesheep && generation < 10000) )
						{
							_list.push_back(itr->path().string().c_str());
							gotSheep = true;
						}
					}
				}
			}
		}
	}

	}
	catch(boost::filesystem::filesystem_error& err)
	{
		g_Log->Error( "Path enumeration threw error: %s",  err.what() );
	}

	return gotSheep;
}

}
