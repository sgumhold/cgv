#include <cgv/utils/big_binary_file.h>
#include <iostream>

#ifdef _MSC_VER
#include <windows.h>
HANDLE& F(void*& file) { return (HANDLE&) file; }
const HANDLE& F(const void*& file) { return (const HANDLE&) file; }
std::wstring str2wstr(const std::string& s)
{
	std::wstring ws;
	ws.resize(s.size());
	int n = MultiByteToWideChar(CP_ACP,0,s.c_str(),(int)s.size(),&ws[0],(int)ws.size());
	ws.resize(n);
	return ws;
}
#else
FILE*& F(void*& file) { return (FILE*&) file; }
const FILE*& F(const void*& file) { return (const FILE*&) file; }
std::string str2wstr(const std::string& s)
{
	std::cerr << "str2wstr(const std::string& s) not implemented" << std::endl;
	return s;
}
#endif


namespace cgv {
	namespace utils{

	#ifdef _MSC_VER

	big_binary_file::big_binary_file(const std::string &filename)
	{
		this->filename = filename;
		fileopened=false;
	};
		
	bool big_binary_file::open( MODE m, const std::string& file_name)
	{	
			if(fileopened)
				close();

			if (!file_name.empty())
				filename = file_name;
#ifdef UNICODE
			std::wstring wfile_name = str2wstr(file_name);
			const wchar_t* name = wfile_name.c_str();
#else
			const char* name = file_name.c_str();
#endif

			access_mode = m;
			if(m == READ) {
				//open for read
				F(file) = CreateFile(name,						
						GENERIC_READ,					
							FILE_SHARE_READ,            
							NULL,                       
							OPEN_EXISTING,               
							FILE_ATTRIBUTE_NORMAL,      
							NULL);

			} 
			else if (m == WRITE) {
				//open for write
				F(file) = CreateFile(name,           
						 GENERIC_WRITE,               
						0,                           
						NULL,                        
						CREATE_ALWAYS,               
						FILE_ATTRIBUTE_NORMAL,      
						NULL);  
			}
			else {
				//open for write
				F(file) = CreateFile(name,           
						GENERIC_READ|GENERIC_WRITE,
						0,                           
						NULL,                        
						OPEN_ALWAYS,               
						FILE_ATTRIBUTE_NORMAL,      
						NULL);  
			}
			if (F(file) != INVALID_HANDLE_VALUE) {
				fileopened=true;
				return true;
			}
			return false;
	}


	void big_binary_file::close()
	{
		if(fileopened)
			CloseHandle(F(file));	
		fileopened=false;

	}

	bool big_binary_file::is_open()
	{
		return fileopened;
	}


	long long big_binary_file::size()
	{
			if(fileopened)
			{
				LARGE_INTEGER fs;
				
				if(GetFileSizeEx(F(file),(PLARGE_INTEGER) &fs))
					return fs.QuadPart;
				else
					return -1;
			}
			else
			{
				open(READ);

				LARGE_INTEGER fs;
				long long value;
				
				if(GetFileSizeEx(F(file),(PLARGE_INTEGER) &fs))
					value = fs.QuadPart;
				else
					value = -1;

				close();
				return value;
			}
		}
		
	bool big_binary_file::read(unsigned char *targetbuffer,unsigned long num, unsigned long *numread)
		{
			if(fileopened && access_mode == READ)
			{
				if(numread == NULL)
				{
					unsigned long _numread;
					ReadFile(F(file),targetbuffer,num,&_numread,NULL);
					return num == _numread;
				}
				else
				{
					ReadFile(F(file),targetbuffer,num,numread,NULL);
					return  num == *numread;
				} 
			}
			return false;
		}

	bool big_binary_file::write(const unsigned char *sourcebuffer,unsigned long num, unsigned long *numwrote)
		{
			if(fileopened && access_mode == WRITE)
			{
				if(numwrote == NULL)
				{
					unsigned long _numwrote;
					WriteFile(F(file),sourcebuffer, num, &_numwrote, NULL);
					return _numwrote == num;
				}
				else
				{
					if(!WriteFile(F(file),sourcebuffer, num, numwrote, NULL))

					{
						std::cerr << "write error"<<std::endl;		
						
					}

					return *numwrote == num;
				}
			}
			return false;
		}
		
	bool big_binary_file::seek(long long index)
		{
			if(fileopened)
			{
				LARGE_INTEGER l;
					l.QuadPart=(index);
			
				if(SetFilePointerEx(F(file),l,NULL,FILE_BEGIN))
					return true;
				else
					return false;
			}
			return false;
		}

	long long big_binary_file::position()
		{
			if(fileopened)
			{
				LARGE_INTEGER l1,l2;
				l1.QuadPart=0;
				if(SetFilePointerEx(F(file),l1,&l2,FILE_CURRENT))
					return l2.QuadPart;
				else
					return -1;

			}
			else
				return false;

		}
		

	big_binary_file::~big_binary_file()
	{
		if(fileopened)
			CloseHandle(F(file));
	}


	#else

/*
	// Linux-Implementation
	
	big_binary_file::big_binary_file(const std::string &filename)
	{
		this->filename = filename;
		fileopened=false;
	};
		
	bool big_binary_file::open(MODE m, const std::string& file_name)
	{	
		if(fileopened)
			close();

		if (!file_name.empty())
			filename = file_name;

		access_mode = m;
		if(m == READ)
		{
			//open for read
			F(file) = fopen64(filename.c_str(), "rb");
		} if (m == WRITE) else
		{
			//open for write
			F(file) = fopen64(filename.c_str(), "wb");
		}
		else {
			//open for write
			F(file) = fopen64(filename.c_str(), "w+b");
		}
		if (file) {
			fileopened=true;
			return true;
		}
		return false;
	}
	
	void big_binary_file::close()
	{
		if(fileopened)
			fclose(F(file));	

		fileopened=false;
	}

	bool big_binary_file::is_open()
	{
		return fileopened;
	}
	
	long long big_binary_file::size()
	{
		if(fileopened)
		{
			off64_t size;

			if(!fseeko64(F(file), 0, SEEK_END))
			{
  				size = ftello64(F(file));
	  			fseeko64(F(file), 0, SEEK_SET);
				return size;
			}
			else
			{
				fseeko64(F(file), 0, SEEK_SET);
				return -1;
			}

		}
		else
		{
			open(READ);

			off64_t size;
			long long value;

			if(!fseeko64(F(file), 0, SEEK_END))
			{
  				size = ftello64(F(file));
	  			fseeko64(F(file), 0, SEEK_SET);
				value = size;
			}
			else
			{
				fseeko64(F(file), 0, SEEK_SET);
				value = -1;
			}

			close();

			return value;
		}
	}

	bool big_binary_file::read(unsigned char *targetbuffer,unsigned long num, unsigned long *numread)
	{
		if(fileopened && access_mode == READ)
		{
			if(numread == NULL)
			{
				unsigned long _numread;
				_numread = fread(targetbuffer, 1, num, F(file));
				return num == _numread;
			}
			else
			{
				*numread = fread(targetbuffer, 1, num, F(file));
				return  num == *numread;
			} 
		}
		return false;
	}

	bool big_binary_file::write(unsigned char *sourcebuffer,unsigned long num, unsigned long *numwrote)
	{
		if(fileopened && access_mode == WRITE)
		{
			if(numwrote == NULL)
			{
				unsigned long _numwrote;
				_numwrote = fwrite(sourcebuffer, 1, num, F(file));
				return _numwrote == num;
			}
			else
			{
				if(!fwrite(sourcebuffer, 1, num, F(file)))
					std::cerr << "write error"<<std::endl;		

				return *numwrote == num;
			}
		}
		return false;
	}

	bool big_binary_file::seek(long long index)
	{
		if(fileopened)
		{
			off64_t pos;
			pos = index;

			if(!fseeko64(F(file), pos, SEEK_SET)) 
				return true;
			else 
				return false;
		}

		return false;
	}

	long long big_binary_file::position()
	{
		if(fileopened)
			return ftello64(F(file));
		else
			return false;
	}
		
	big_binary_file::~big_binary_file()
	{
		if(fileopened)
			fclose(F(file));
	}
*/
	#endif

	}
}
