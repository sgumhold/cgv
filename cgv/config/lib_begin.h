#ifdef _WIN32
#	if _MSC_VER >= 1400
#		pragma warning(disable : 4275)
#		pragma warning(disable : 4251)
#	endif
#	if defined(CGV_FORCE_STATIC_LIB) || defined(CGV_FORCE_STATIC)
#		define CGV_API
#		define CGV_IS_STATIC
#		define CGV_TEMPLATE
#	else
#		if defined(CGV_EXPORTS) || defined(CGV_FORCE_EXPORT)
#			define CGV_API __declspec(dllexport)
#			define CGV_TEMPLATE
#		else
#			ifndef __INTEL_COMPILER
#				define CGV_API __declspec(dllimport)
#				ifdef _MSC_VER
#					define CGV_TEMPLATE extern
#				endif
#			else
#				define CGV_API
#				define CGV_TEMPLATE
#			endif
#		endif
#	endif
#	ifdef _MSC_VER
#		define FRIEND_MEMBER_API extern CGV_API
#	else
#		define FRIEND_MEMBER_API
#	endif
#else
#	define CGV_API
#	define FRIEND_MEMBER_API
#	define CGV_TEMPLATE
#endif

#undef CGV_EXPORTS
#undef CGV_FORCE_STATIC_LIB
