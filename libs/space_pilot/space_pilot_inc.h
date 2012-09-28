#pragma once

// Don't use seldom used methods
#define WIN32_LEAN_AND_MEAN		
// some CString constructors will be explicit
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	

#define _ATL_APARTMENT_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit


// Standard includes
#include <stdio.h>
#include <tchar.h>




// Headers for ATL
#include <atlbase.h>
#include <atlcom.h>
#include <atlwin.h>
#include <atltypes.h>
#include <atlctl.h>
#include <atlhost.h>


using namespace ATL;

// Import interfaces from TdxInput.dll (ISimpleDevice, IKeyboard, ISensor ...)
#import "progid:TDxInput.Device" embedded_idl no_namespace
