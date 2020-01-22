#include <cgv/media/font/font_server.h>
#include <cgv/utils/endian.h>
#include <cgv/utils/scan.h>
#include <cgv/utils/dir.h>
#include <cgv/utils/file.h>
#include <stdio.h>
#include <map>

namespace cgv {
	namespace media {
		namespace font {

typedef struct _tagTT_OFFSET_TABLE {    
    uint16_t	uMajorVersion;
    uint16_t	uMinorVersion;
    uint16_t	uNumOfTables;
    uint16_t	uSearchRange;
    uint16_t	uEntrySelector;
    uint16_t	uRangeShift;
} TT_OFFSET_TABLE;

typedef struct _tagTT_TABLE_DIRECTORY {
    char	    szTag[4];			//table name
    uint32_t	uCheckSum;			//Check sum
    uint32_t	uOffset;			//Offset from beginning of file
    uint32_t	uLength;			//length of the table in bytes
} TT_TABLE_DIRECTORY;

typedef struct _tagTT_NAME_TABLE_HEADER {
    uint16_t	uFSelector;			//format selector. Always 0
    uint16_t	uNRCount;			//Name Records count
    uint16_t	uStorageOffset;		//Offset for strings storage, from start of the table
} TT_NAME_TABLE_HEADER;

typedef struct _tagTT_NAME_RECORD {
    uint16_t	uPlatformID;
    uint16_t	uEncodingID;
    uint16_t	uLanguageID;
    uint16_t	uNameID;
    uint16_t	uStringLength;
    uint16_t	uStringOffset;	//from start of storage area
} TT_NAME_RECORD;

bool extract_name_and_face(const std::string& file_path, std::string& font_name, std::string& font_face)
{
	FILE* fp = fopen(file_path.c_str(), "rb");
	if (fp == 0)
		return false;

	TT_OFFSET_TABLE ttOffsetTable;
	if (1 != fread(&ttOffsetTable, sizeof(TT_OFFSET_TABLE), 1, fp))
		return false;

	cgv::utils::convert_endian_from<cgv::utils::E_BIG_32>(ttOffsetTable.uNumOfTables);
	cgv::utils::convert_endian_from<cgv::utils::E_BIG_32>(ttOffsetTable.uMajorVersion);
	cgv::utils::convert_endian_from<cgv::utils::E_BIG_32>(ttOffsetTable.uMinorVersion);

	//check is this is a true type font and the version is 1.0
	if (ttOffsetTable.uMajorVersion != 1 || ttOffsetTable.uMinorVersion != 0)
		return false;

	TT_TABLE_DIRECTORY tblDir;
	bool found = false;

	for (int i = 0; i < ttOffsetTable.uNumOfTables; i++) {
		if (1 != fread(&tblDir, sizeof(TT_TABLE_DIRECTORY), 1, fp))
			return false;
		if (toupper(tblDir.szTag[0]) == 'N' &&
			toupper(tblDir.szTag[1]) == 'A' &&
			toupper(tblDir.szTag[2]) == 'M' &&
			toupper(tblDir.szTag[3]) == 'E')
		{
			found = true;
			cgv::utils::convert_endian_from<cgv::utils::E_BIG_32>(tblDir.uLength);
			cgv::utils::convert_endian_from<cgv::utils::E_BIG_32>(tblDir.uOffset);
			break;
		}
	}

	if (found) {
		fseek(fp, tblDir.uOffset, SEEK_SET);
		TT_NAME_TABLE_HEADER ttNTHeader;
		fread(&ttNTHeader, sizeof(TT_NAME_TABLE_HEADER), 1, fp);
		cgv::utils::convert_endian_from<cgv::utils::E_BIG_32>(ttNTHeader.uNRCount);
		cgv::utils::convert_endian_from<cgv::utils::E_BIG_32>(ttNTHeader.uStorageOffset);
		TT_NAME_RECORD ttRecord;
		found = false;
		for (int i = 0; i < ttNTHeader.uNRCount; i++) {
			fread(&ttRecord, sizeof(TT_NAME_RECORD), 1, fp);
			cgv::utils::convert_endian_from<cgv::utils::E_BIG_32>(ttRecord.uNameID);
			if (ttRecord.uNameID == 1 || ttRecord.uNameID == 2) {
				cgv::utils::convert_endian_from<cgv::utils::E_BIG_32>(ttRecord.uStringLength);
				cgv::utils::convert_endian_from<cgv::utils::E_BIG_32>(ttRecord.uStringOffset);
				long nPos = ftell(fp);
				fseek(fp, tblDir.uOffset + ttRecord.uStringOffset + ttNTHeader.uStorageOffset, SEEK_SET);
				std::string name(ttRecord.uStringLength + 1, char(0));
				fread(&name[0], 1, ttRecord.uStringLength, fp);
				if (name.length() > 0) {
					if (ttRecord.uNameID == 1)
						font_name = &name[0];
					else {
						font_face = &name[0];
						break;
					}
				}
				fseek(fp, nPos, SEEK_SET);
			}
		}
	}
	fclose(fp);
	return true;
}

struct font_file_info {
	std::string file_names[4];
};

typedef std::map<std::string, font_file_info> font_map_type;

font_map_type ref_font_map_type()
{
	static font_map_type font_map;
	return font_map;
}

void analyze_font_dir(const std::string& path)
{
	font_map_type& font_map = ref_font_map_type();

	std::vector<std::string> file_names;
	if (!cgv::utils::dir::glob(path, file_names, "*.ttf"))
		return;

	for (const auto& file_name : file_names) {
		std::string font_name, font_face;
		if (!extract_name_and_face(file_name, font_name, font_face))
			continue;
		if (font_name.empty())
			continue;
		// check for supported font faces
		font_face = cgv::utils::to_upper(font_face);
		int font_face_idx = -1;
		if (font_face.empty() || font_face == "REGULAR")
			font_face_idx = (int)FFA_REGULAR;
		else if (font_face == "BOLD")
			font_face_idx = (int)FFA_BOLD;
		else if (font_face == "ITALIC")
			font_face_idx = (int)FFA_ITALIC;
		else if (font_face == "BOLD ITALIC")
			font_face_idx = (int)FFA_BOLD_ITALIC;

		if (font_face_idx != -1)
			font_map[font_name].file_names[font_face_idx] = cgv::utils::file::get_file_name(file_name);
	}


	for (auto fi = font_map.begin(); fi != font_map.end(); ++fi) {
		std::cout << "[";
		for (int i = 0; i < 4; ++i)
			std::cout << (fi->second.file_names[i].empty()?' ':'*');
		std::cout << "] " << fi->first << std::endl;
	}
	std::cout << font_map.size() << " fonts." << std::endl;
}

font_server_ptr& ref_font_server()
{
	static font_server_ptr fs;
	static bool analyzed = false;
	if (!analyzed) {
		analyze_font_dir("C:\\Windows\\Fonts");
		analyzed = true;
	}
	return fs;
}

/// return the currently installed font server or 0 if no font server available
font_server_ptr get_font_server()
{
	return ref_font_server();
}

/// install a font server
void register_font_server(font_server_ptr fs)
{
	ref_font_server() = fs;
}

		}
	}
}
