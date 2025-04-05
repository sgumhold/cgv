#include "clipboard.h"

#include <iostream>

#ifdef WIN32
#include <windows.h>
#endif

namespace cgv {
	namespace os {

#ifdef WIN32
bool copy_text_to_clipboard(const char* clipboard_text)
{
	if ( !OpenClipboard(NULL) )
	{
	  std::cout << "Cannot open the Clipboard" << std::endl;
	 return false;
	}
	// Remove the current Clipboard contents
	if( !EmptyClipboard() )
	{
	 std::cout << "Cannot empty the Clipboard" << std::endl;
	 return false;
	}
	// ...
	// Get the currently selected data
	// ...
	// For the appropriate data formats...
	HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, 
		(strlen(clipboard_text)+ 1) * sizeof(TCHAR)); 
	if (hglbCopy == NULL) 
	{ 
		CloseClipboard(); 
		return false; 
	} 

	// Lock the handle and copy the text to the buffer. 

	LPTSTR  lptstrCopy = (LPTSTR)GlobalLock(hglbCopy); 
	memcpy(lptstrCopy, clipboard_text, 
		strlen(clipboard_text) * sizeof(TCHAR)); 
	lptstrCopy[strlen(clipboard_text)] = (TCHAR) 0;    // null character 
	GlobalUnlock(hglbCopy); 


	if ( ::SetClipboardData( CF_TEXT, hglbCopy) == NULL )
	{
	 std::cout << "Unable to set Clipboard data" << std::endl;
	 CloseClipboard();
	 return false;
	}
	// ...
	CloseClipboard();
	return true;
}
bool copy_rgb_image_to_clipboard(int w, int h, const unsigned char* image_buffer)
{
	if ( !OpenClipboard(NULL) )
	{
	  std::cout << "Cannot open the Clipboard" << std::endl;
	 return false;
	}
	// Remove the current Clipboard contents
	if( !EmptyClipboard() )
	{
	 std::cout << "Cannot empty the Clipboard" << std::endl;
	 return false;
	}
	// ...
	// Get the currently selected data
	// ...
	// For the appropriate data formats...
	unsigned int src_line_length = w*3;
	unsigned int dst_line_length = src_line_length;
	if ((dst_line_length&3) != 0)
		dst_line_length = (dst_line_length/4+1)*4;
	unsigned int size = sizeof(BITMAPINFO)+dst_line_length*h;
	HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, size); 
	if (hglbCopy == NULL) 
	{ 
		CloseClipboard(); 
		return false; 
	} 

	// Lock the handle and copy the text to the buffer. 
	PBITMAPINFO pbmi = (PBITMAPINFO)GlobalLock(hglbCopy); 
	PBITMAPINFOHEADER pbhi = &pbmi->bmiHeader; 
	pbhi->biSize = sizeof(BITMAPINFOHEADER);
	pbhi->biWidth = w;
	pbhi->biHeight = h;
	pbhi->biPlanes = 3;
	pbhi->biBitCount = 24;
	pbhi->biCompression = BI_RGB; 
	pbhi->biSizeImage = 0; 
	pbhi->biXPelsPerMeter = 1000000; 
	pbhi->biYPelsPerMeter = 1000000; 
	pbhi->biClrUsed = 0; 
	pbhi->biClrImportant = 0;
	unsigned char* dst = (unsigned char*)&pbmi->bmiColors[0];
	const unsigned char* src = image_buffer;
	for (int i=0; i<h; ++i) {
		unsigned char* dst_1 = dst;
		const unsigned char* src_1 = src;
		for (int j = 0; j<w; ++j) {
			*dst_1++ = src_1[2];
			*dst_1++ = src_1[1];
			*dst_1++ = src_1[0];
			src_1 += 3;
		}
		dst += dst_line_length;
		src += src_line_length;
	}
	GlobalUnlock(hglbCopy);

	if ( ::SetClipboardData( CF_DIB, hglbCopy) == NULL )
	{
	 std::cout << "Unable to set Clipboard data" << std::endl;
	 CloseClipboard();
	 return false;
	}
	// ...
	CloseClipboard();
	return true;
}
bool get_text_from_clipboard(std::string& text, bool clear_clipboard)
{
	if (!IsClipboardFormatAvailable(CF_TEXT))
		return false;
	bool result = false;
	if (OpenClipboard(nullptr)) {
		// Get handle of clipboard object for ANSI text
		HANDLE hData = GetClipboardData(CF_TEXT);
		if (hData != nullptr) {
			// Lock the handle to get the actual text pointer
			char* pszText = static_cast<char*>(GlobalLock(hData));
			if (pszText != nullptr) {
				// Save text in a string class instance
				text = std::string(pszText);
				result = true;
			}
			// Release the lock
			GlobalUnlock(hData);
		}
		// if requested, clear the clipboard
		if (clear_clipboard)
			EmptyClipboard();
		// Release the clipboard
		CloseClipboard();
	}
	return result;
}
bool get_rgb_image_from_clipboard(int& w, int& h, std::vector<char>& data, bool clear_clipboard)
{
	if (!IsClipboardFormatAvailable(CF_BITMAP))
		return false;
	bool result = false;
	// Try opening the clipboard
	if (OpenClipboard(nullptr)) {
		// Get handle of clipboard object for ANSI text
		HANDLE hData = GetClipboardData(CF_BITMAP);
		if (hData != nullptr) {
			HBITMAP hBitmap = (HBITMAP)hData;
			BITMAP bmp;
			// Retrieve the bitmap color format, width, and height.
			if (GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&bmp)) {
				BITMAPINFO bmi;
				std::fill((uint8_t*)&bmi, (uint8_t*)(&bmi + 1), 0);
				bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
				w = bmi.bmiHeader.biWidth = bmp.bmWidth;
				h = bmi.bmiHeader.biHeight = bmp.bmHeight;
				bmi.bmiHeader.biPlanes = 1;
				bmi.bmiHeader.biBitCount = 24;
				bmi.bmiHeader.biCompression = BI_RGB;
				data.resize(3 * bmp.bmHeight * bmp.bmWidth);
				if (GetDIBits(GetDC(0), hBitmap, 0, (WORD)bmp.bmHeight, data.data(), &bmi, DIB_RGB_COLORS)) {
					result = true;
				}
			}
		}
		// if requested, clear the clipboard
		if (clear_clipboard)
			EmptyClipboard();
		// Release the clipboard
		CloseClipboard();
	}
	return true;
}

#else

bool copy_text_to_clipboard(const char* text) {
	// FIXME implement this (for now this is a no-op on non-windows systems)
	return false;
}

bool copy_rgb_image_to_clipboard(int w, int h, const unsigned char* image_buffer) {
    // FIXME implement this (for now this is a no-op on non-windows systems)
    return false;
}
bool get_text_from_clipboard(std::string& text, bool clear_clipboard)
{
	// FIXME implement this (for now this is a no-op on non-windows systems)
	return false;
}

/// if clipboard contains image (return true), copy image dims to \c w, \c h and pixel data in rgb24 format to \c data; optionally clear clipboard
bool get_rgb_image_from_clipboard(int& w, int& h, std::vector<char>& data, bool clear_clipboard)
{
	// FIXME implement this (for now this is a no-op on non-windows systems)
	return false;
}

#endif

	}
}