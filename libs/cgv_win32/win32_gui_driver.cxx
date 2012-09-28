#include "win32_gui_driver.h"

#include <Windows.h>
#include <cgv/media/text/convert.h>
#include <cgv/media/text/scan.h>


void prepare_ofn_struct(OPENFILENAME& ofn, wchar_t *szFile, int file_size,
                        const std::string& title, std::wstring& wtitle, 
						const std::string& filter, std::wstring& wfilter, const std::string& path)
{
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = GetForegroundWindow();
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = file_size;
	std::string f = filter;
	cgv::media::text::replace(f,':','\0');
	cgv::media::text::replace(f,'|','\0');
	f += '\0';
	wfilter = cgv::media::text::str2wstr(f);
	ofn.lpstrFilter = wfilter.c_str();
	ofn.nFilterIndex = 0;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	wtitle = cgv::media::text::str2wstr(title);
	ofn.lpstrTitle = wtitle.c_str();
}


namespace cgv {
	namespace gui {

std::string win32_gui_driver::file_open_dialog(const std::string& title, const std::string& filter, const std::string& path)
{
	OPENFILENAME ofn;
	wchar_t szFile[500];
	std::wstring wfilter, wtitle;
	prepare_ofn_struct(ofn, szFile, 500, title, wtitle, filter, wfilter, path);
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	if (GetOpenFileName(&ofn)==TRUE)
		return cgv::media::text::wstr2str(szFile);
	return "";
}

std::string win32_gui_driver::file_save_dialog(const std::string& title, const std::string& filter, const std::string& path)
{
	OPENFILENAME ofn;
	wchar_t szFile[500];
	std::wstring wfilter, wtitle;
	prepare_ofn_struct(ofn, szFile, 500, title, wtitle, filter, wfilter, path);
	ofn.Flags = OFN_OVERWRITEPROMPT;
	if (GetSaveFileName(&ofn)==TRUE)
		return cgv::media::text::wstr2str(szFile);
	return "";
}

	}
}

#include <cgv/config/lib_end.h>