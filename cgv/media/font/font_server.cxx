#include <cgv/media/font/font_server.h>

namespace cgv {
	namespace media {
		namespace font {
			/*
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

#define MAKEWORD(a, b)      ((WORD)(((BYTE)(((DWORD_PTR)(a)) & 0xff)) | ((WORD)((BYTE)(((DWORD_PTR)(b)) & 0xff))) << 8))
#define MAKELONG(a, b)      ((LONG)(((WORD)(((DWORD_PTR)(a)) & 0xffff)) | ((DWORD)((WORD)(((DWORD_PTR)(b)) & 0xffff))) << 16))
#define LOWORD(l)           ((WORD)(((DWORD_PTR)(l)) & 0xffff))
#define HIWORD(l)           ((WORD)((((DWORD_PTR)(l)) >> 16) & 0xffff))
#define LOBYTE(w)           ((BYTE)(((DWORD_PTR)(w)) & 0xff))
#define HIBYTE(w)           ((BYTE)((((DWORD_PTR)(w)) >> 8) & 0xff))

#define SWAPWORD(x)		MAKEWORD(HIBYTE(x), LOBYTE(x))
#define SWAPLONG(x)		MAKELONG(SWAPWORD(HIWORD(x)), SWAPWORD(LOWORD(x)))

#include <string>
#include <stdio.h>

bool extract_name_and_face(const std::string& file_path, std::string& font_name, std::string& font_face)
{
	FILE* fp = fopen(file_path.c_str(), "rb");
	if (fp == 0)
		return false;

	TT_OFFSET_TABLE ttOffsetTable;
	if (1 != fread(&ttOffsetTable, sizeof(TT_OFFSET_TABLE), 1, fp))
		return false;

	ttOffsetTable.uNumOfTables  = SWAPWORD(ttOffsetTable.uNumOfTables);
	ttOffsetTable.uMajorVersion = SWAPWORD(ttOffsetTable.uMajorVersion);
	ttOffsetTable.uMinorVersion = SWAPWORD(ttOffsetTable.uMinorVersion);

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
			found = TRUE;
			tblDir.uLength = SWAPLONG(tblDir.uLength);
			tblDir.uOffset = SWAPLONG(tblDir.uOffset);
			break;
		}
	}

	if (found) {
		fseek(fp, tblDir.uOffset, SEEK_SET);
		TT_NAME_TABLE_HEADER ttNTHeader;
		fread(&ttNTHeader, sizeof(TT_NAME_TABLE_HEADER), 1, fp);
		ttNTHeader.uNRCount = SWAPWORD(ttNTHeader.uNRCount);
		ttNTHeader.uStorageOffset = SWAPWORD(ttNTHeader.uStorageOffset);
		TT_NAME_RECORD ttRecord;
		found = false;
		for (int i = 0; i < ttNTHeader.uNRCount; i++) {
			fread(&ttRecord, sizeof(TT_NAME_RECORD), 1, fp);
			ttRecord.uNameID = SWAPWORD(ttRecord.uNameID);
			if (ttRecord.uNameID == 1 || ttRecord.uNameID == 2) {
				ttRecord.uStringLength = SWAPWORD(ttRecord.uStringLength);
				ttRecord.uStringOffset = SWAPWORD(ttRecord.uStringOffset);
				long nPos = ftell(fp);
				fseek(fp, tblDir.uOffset + ttRecord.uStringOffset + ttNTHeader.uStorageOffset, SEEK_SET);
				std::string name(ttRecord.uStringLength + 1, char(0));
				fread(&name[0], 1, ttRecord.uStringLength, fp);
				if (name.length() > 0) {
					if (ttRecord.uNameID == 1)
						res = &name[0];
					else {
						sub = &name[0];
						break;
					}
				}
				fseek(fp, nPos, SEEK_SET);
			}
		}
	}
	fclose(fp);
	res += '|';
	res += sub;
	return res;
}
*/

font_server_ptr& ref_font_server()
{
	static font_server_ptr fs;
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
