#include "display.h"

#include <cgv/utils/convert.h>
#include <iostream>

#ifdef _MSC_VER
#include <windows.h>
#endif

namespace cgv {
	namespace os {

std::string display::last_error;

std::ostream& operator << (std::ostream& os, const display_mode& dm)
{
	return
		os << dm.width << "x"
		   << dm.height << "x"
			<< dm.bit_depth<< "|"
			<< dm.refresh_rate << "Hz";
}

std::ostream& operator << (std::ostream& os, const display_position& dp)
{
	return os << dp.x << "," << dp.y;
}

std::ostream& operator << (std::ostream& os, const display_configuration& dc)
{
	return os << dc.mode << ":" << dc.position;
}

std::ostream& operator << (std::ostream& os, const display& disp)
{
	os << disp.get_name().c_str() << " ('" << disp.get_description().c_str()
		<< "' - " << disp.get_ID().c_str() << ")" 
		<< (disp.is_primary()?" primary":"") 
		<< (disp.is_active()?" attached":"") 
		<< (disp.is_removable()?" removable":"") 
		<< (disp.is_mirror()?" mirror":"") << "\n   ";
	if (disp.is_active())
		os << disp.get_configuration() << " ";
	os << "[" << disp.get_registered_mode() << "]";
	return os;
}

#ifdef _MSC_VER
class windows_display : public display
{
protected:
	DISPLAY_DEVICE dd;
	DEVMODE cdm, rdm;
	unsigned int idx;
public:
	bool update_structures()
	{
		if (!EnumDisplayDevices(NULL,idx,&dd,0)) {
			last_error = "could not enumerate device";
			return false;
		}
		if (!EnumDisplaySettingsEx(dd.DeviceName, ENUM_REGISTRY_SETTINGS, &rdm, 0)) {
			last_error = "could not query registry display settings";
			return false;
		}
		if (is_active()) {
			if (!EnumDisplaySettingsEx(dd.DeviceName, ENUM_CURRENT_SETTINGS, &cdm, 0)) {
				last_error = "could not query current display settings";
				return false;
			}
		}
		return true;
	}
	static bool interpret_error(LONG result)
	{
		switch (result) {
		case DISP_CHANGE_SUCCESSFUL:
			break;
		case DISP_CHANGE_BADFLAGS:
			last_error = "An invalid set of flags was passed in.";
			return false;
		case DISP_CHANGE_BADMODE:
			last_error = "The graphics mode is not supported.";
			return false;
		case DISP_CHANGE_BADPARAM:
			last_error = "An invalid parameter was passed in. This can include an invalid flag or combination of flags.";
			return false;
		case DISP_CHANGE_FAILED:
			last_error = "The display driver failed the specified graphics mode.";
			return false;
		case DISP_CHANGE_NOTUPDATED:
			last_error = "Windows NT/2000/XP: Unable to write settings to the registry.";
			return false;
		case DISP_CHANGE_RESTART:
			last_error = "The computer must be restarted for the graphics mode to work.";
			return false;
		}
		return true;
	}
	bool update_after_display_change(LONG result)
	{
		if (!interpret_error(result))
			return false;
		return update_structures();
	}
	/// enumerate all available display modes. If the display is active only the display modes supported by the attached monitor are enumerated
	void enumerate_display_modes(std::vector<display_mode>& display_modes, bool compatible_with_attached_monitor)
	{
		DEVMODE dm;
		memset(&dm,0,sizeof(DEVMODE));
		dm.dmSize = sizeof(DEVMODE);
		dm.dmDriverExtra = 0;
		unsigned int i = 0;
		while (EnumDisplaySettingsEx(dd.DeviceName, i, &dm, compatible_with_attached_monitor?0:EDS_RAWMODE)) {
			display_mode disp_mode;
			disp_mode.width  = dm.dmPelsWidth;
			disp_mode.height = dm.dmPelsHeight;
			disp_mode.bit_depth = dm.dmBitsPerPel;
			disp_mode.refresh_rate = dm.dmDisplayFrequency;
			display_modes.push_back(disp_mode);
			++i;
		}
	}
	/// print out all available display modes
	void show_display_modes(bool compatible_with_attached_monitor)
	{
		std::vector<display_mode> display_modes;
		enumerate_display_modes(display_modes, compatible_with_attached_monitor);
		for (unsigned int i = 0; i < display_modes.size(); ++i)
			std::cout << "display mode " << i << ": " << display_modes[i] << std::endl;
	}

	windows_display()
	{
		memset(&dd,0,sizeof(DISPLAY_DEVICE));
		dd.cb = sizeof(DISPLAY_DEVICE);
		memset(&cdm,0,sizeof(DEVMODE));
		cdm.dmSize = sizeof(DEVMODE);
		memset(&rdm,0,sizeof(DEVMODE));
		rdm.dmSize = sizeof(DEVMODE);
	}
	bool attach(unsigned int i)
	{
		idx = i;
		return update_structures();
	}
	/// return the name of the display
	std::string get_name() const 
	{
		return cgv::utils::wstr2str(dd.DeviceName);
	}
	/// return the description string of the display
	std::string get_description() const
	{
		return cgv::utils::wstr2str(dd.DeviceString);
	}
	/// return the unique ID string of the display
	std::string get_ID() const
	{
		return cgv::utils::wstr2str(dd.DeviceID);
	}
	/// return whether display is a mirrored display
	bool is_mirror() const
	{
		return (dd.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) != 0;
	}
	/// return whether display is removable
	bool is_removable() const
	{
		return (dd.StateFlags & DISPLAY_DEVICE_REMOVABLE) != 0;
	}

	/// return whether display is active
	bool is_active() const
	{
		return (dd.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) != 0;
	}
	/// activate the display based on the registered settings or change the current display settings to the registered settings if the display is already active
	bool activate()
	{
		return update_after_display_change(ChangeDisplaySettingsEx(dd.DeviceName, NULL, NULL, 0, NULL));
	}
	/// activate the display based on the registered settings or change the current display settings to the registered settings if the display is already active
	bool check_activate() const
	{
		return interpret_error(ChangeDisplaySettingsEx(dd.DeviceName, NULL, NULL, CDS_TEST, NULL));
	}
	/// deactivate the display
	bool deactivate()
	{
		cdm.dmFields = DM_POSITION | DM_PELSWIDTH | DM_PELSHEIGHT ;
		cdm.dmPelsWidth = 0;
		cdm.dmPelsHeight = 0;
		return update_after_display_change(
			ChangeDisplaySettingsEx(dd.DeviceName, &cdm, NULL, CDS_UPDATEREGISTRY, NULL));
	}
	/// return whether display is the primary display
	bool is_primary() const 
	{
		return (dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) != 0;
	}
	/// make this display the primary display
	bool make_primary()
	{
		return update_after_display_change(
			ChangeDisplaySettingsEx(dd.DeviceName, NULL, NULL, CDS_UPDATEREGISTRY|CDS_SET_PRIMARY, NULL));
	}
	/// check if this display can be made primary
	bool can_be_primary() const
	{
		return !is_removable();
	}
	
	/// return the current display configuration
	display_configuration get_configuration() const
	{
		return display_configuration(get_mode(),get_position());
	}
	/// return the currently set display mode
	display_mode get_mode() const
	{
		return display_mode(get_width(),get_height(),get_bit_depth(),get_refresh_rate());
	}
	/// return the current width (only defined if display is active)
	unsigned int get_width() const 
	{
		return cdm.dmPelsWidth;
	}
	/// return the current height (only defined if display is active)
	unsigned int get_height() const
	{
		return cdm.dmPelsHeight;
	}
	/// return the current bit depth of the display pixels (only defined if display is active)
	unsigned int get_bit_depth() const 
	{
		return cdm.dmBitsPerPel;
	}
	/// return the current refresh rate in Hz (only defined if display is active)
	unsigned int get_refresh_rate() const
	{
		return cdm.dmDisplayFrequency;
	}
	/// return the current display position
	display_position get_position() const
	{
		return display_position(get_x(), get_y());
	}
	/// return the current current x coordinate within the virtual screen spanned by the desktop (only defined if display is active)
	int get_x() const
	{
		return cdm.dmPosition.x;
	}
	/// return the current current y coordinate within the virtual screen spanned by the desktop (only defined if display is active)
	int get_y() const
	{
		return cdm.dmPosition.y;
	}

	/// set the display configuration immediately, return whether this was successful.
	bool set_configuration(const display_configuration& dc)
	{
		cdm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY | DM_POSITION;
		cdm.dmPelsWidth  = dc.mode.width;
		cdm.dmPelsHeight = dc.mode.height;
		cdm.dmBitsPerPel = dc.mode.bit_depth;
		cdm.dmDisplayFrequency = dc.mode.refresh_rate;
		cdm.dmPosition.x = dc.position.x;
		cdm.dmPosition.y = dc.position.y;
		return update_after_display_change(
			ChangeDisplaySettingsEx(dd.DeviceName, &cdm, NULL, 0, NULL)); //CDS_UPDATEREGISTRY
	}
	/// set the given display mode immediately, return whether this was successful. Only valid display modes can be set.
	bool set_mode(const display_mode& dm)
	{
		cdm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;
		cdm.dmPelsWidth  = dm.width;
		cdm.dmPelsHeight = dm.height;
		cdm.dmBitsPerPel = dm.bit_depth;
		cdm.dmDisplayFrequency = dm.refresh_rate;
		return update_after_display_change(
			ChangeDisplaySettingsEx(dd.DeviceName, &cdm, NULL, 0, NULL));
	}
	/// set new resolution of the display immediately, return whether this was successful. Only resolutions that correspond to valid display modes can be set.
	bool set_resolution(unsigned int w, unsigned int h)
	{
		cdm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
		cdm.dmPelsWidth = w;
		cdm.dmPelsHeight = h;
		return update_after_display_change(
			ChangeDisplaySettingsEx(dd.DeviceName, &cdm, NULL, 0, NULL));
	}
	/// set new bit depth of the display immediately, return whether this was successful. Only bit depths that correspond to valid display modes can be set.
	bool set_bit_depth(unsigned int bits)
	{
		cdm.dmFields = DM_BITSPERPEL;
		cdm.dmBitsPerPel = bits;
		return update_after_display_change(
			ChangeDisplaySettingsEx(dd.DeviceName, &cdm, NULL, 0, NULL));
	}
	/// set new refresh rate of the display immediately, return whether this was successful. Only refresh rates that correspond to valid display modes can be set.
	bool set_refresh_rate(unsigned int hz)
	{
		cdm.dmFields = DM_DISPLAYFREQUENCY;
		cdm.dmDisplayFrequency = hz;
		return update_after_display_change(
			ChangeDisplaySettingsEx(dd.DeviceName, &cdm, NULL, 0, NULL));
	}
	/// set the display position immediately, return whether this was successful. Only positions that result in a tight non overlapping packing of displays can be set.
	bool set_position(const display_position& dp) 
	{
		return set_position(dp.x, dp.y);
	}
	/// set new position of the display within the virtual screen immediately, return whether this was successful. Only positions that result in a tight non overlapping packing of displays can be set.
	bool set_position(unsigned int x, unsigned int y)
	{
		cdm.dmFields = DM_POSITION;
		cdm.dmPosition.x = x;
		cdm.dmPosition.y = y;
		return update_after_display_change(
			ChangeDisplaySettingsEx(dd.DeviceName, &cdm, NULL, 0, NULL));
	}


	/// return the display mode registered in the registry
	display_mode get_registered_mode() const
	{
		return display_mode(get_registered_width(), get_registered_height(), get_registered_bit_depth(), get_registered_refresh_rate());
	}
	/// return the registered width (this is changed by prepare_resolution)
	unsigned int get_registered_width() const
	{
		return rdm.dmPelsWidth;
	}
	/// return the registered height (this is changed by prepare_resolution)
	unsigned int get_registered_height() const
	{
		return rdm.dmPelsHeight;
	}
	/// return the registered bit depth of a display pixel (this is changed by prepare_bit_depth)
	unsigned int get_registered_bit_depth() const
	{
		return rdm.dmBitsPerPel;
	}
	/// return the registered refresh rate in Hz (this is changed by prepare_refresh_rate)
	unsigned int get_registered_refresh_rate() const
	{
		return rdm.dmDisplayFrequency;
	}

	/// register new display configuration, which is activated in the next activate() or activate_all() call
	bool register_configuration(const display_configuration& dc)
	{
		cdm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY | DM_POSITION;
		cdm.dmPelsWidth  = dc.mode.width;
		cdm.dmPelsHeight = dc.mode.height;
		cdm.dmBitsPerPel = dc.mode.bit_depth;
		cdm.dmDisplayFrequency = dc.mode.refresh_rate;
		cdm.dmPosition.x = dc.position.x;
		cdm.dmPosition.y = dc.position.y;
		return update_after_display_change(
			ChangeDisplaySettingsEx(dd.DeviceName, &cdm, NULL, CDS_UPDATEREGISTRY|CDS_NORESET, NULL));
	}
	/// register new display mode, which is activated in the next activate() or activate_all() call
	bool register_mode(const display_mode& dm)
	{
		cdm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;
		cdm.dmPelsWidth  = dm.width;
		cdm.dmPelsHeight = dm.height;
		cdm.dmBitsPerPel = dm.bit_depth;
		cdm.dmDisplayFrequency = dm.refresh_rate;
		return update_after_display_change(
			ChangeDisplaySettingsEx(dd.DeviceName, &cdm, NULL, CDS_UPDATEREGISTRY|CDS_NORESET, NULL));
	}
	/// register new resolution of the display, which is activated in the next activate() call
	bool register_resolution(unsigned int w, unsigned int h)
	{
		rdm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
		rdm.dmPelsWidth = w;
		rdm.dmPelsHeight = h;
		return update_after_display_change(
			ChangeDisplaySettingsEx(dd.DeviceName, &rdm, NULL, CDS_UPDATEREGISTRY|CDS_NORESET, NULL));
	}
	/// register new bit depth of the display, which is activated in the next activate() call
	bool register_bit_depth(unsigned int bits)
	{
		rdm.dmFields = DM_BITSPERPEL;
		rdm.dmBitsPerPel = bits;
		return update_after_display_change(
			ChangeDisplaySettingsEx(dd.DeviceName, &rdm, NULL, CDS_UPDATEREGISTRY|CDS_NORESET, NULL));
	}
	/// register new refresh rate of the display, which is activated in the next activate() call
	bool register_refresh_rate(unsigned int hz)
	{
		rdm.dmFields = DM_DISPLAYFREQUENCY;
		rdm.dmDisplayFrequency = hz;
		return update_after_display_change(
			ChangeDisplaySettingsEx(dd.DeviceName, &rdm, NULL, CDS_UPDATEREGISTRY|CDS_NORESET, NULL));
	}
	/// register new position immediately, which is activated in the next activate() or activate_all() call
	bool register_position(const display_position& dp)
	{
		return register_position(dp.x,dp.y);
	}
	/// register new position of the display, which is activated in the next activate() call
	bool register_position(unsigned int x, unsigned int y)
	{
		rdm.dmFields = DM_POSITION;
		rdm.dmPosition.x = x;
		rdm.dmPosition.y = y;
		return update_after_display_change(
			ChangeDisplaySettingsEx(dd.DeviceName, &rdm, NULL, CDS_UPDATEREGISTRY|CDS_NORESET, NULL));
	}

	/// check given display configuration
	bool check_configuration(const display_configuration& dc) const
	{
		DEVMODE dm;
		memset(&dm,0,sizeof(DEVMODE));
		dm.dmSize = sizeof(DEVMODE);
		dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY | DM_POSITION;
		dm.dmPelsWidth  = dc.mode.width;
		dm.dmPelsHeight = dc.mode.height;
		dm.dmBitsPerPel = dc.mode.bit_depth;
		dm.dmDisplayFrequency = dc.mode.refresh_rate;
		dm.dmPosition.x = dc.position.x;
		dm.dmPosition.y = dc.position.y;
		return interpret_error(
			ChangeDisplaySettingsEx(dd.DeviceName, &dm, NULL, CDS_TEST, NULL));
	}
	/// check given display mode
	bool check_mode(const display_mode& d) const
	{
		DEVMODE dm;
		memset(&dm,0,sizeof(DEVMODE));
		dm.dmSize = sizeof(DEVMODE);
		dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;
		dm.dmPelsWidth  = d.width;
		dm.dmPelsHeight = d.height;
		dm.dmBitsPerPel = d.bit_depth;
		dm.dmDisplayFrequency = d.refresh_rate;
		return interpret_error(
			ChangeDisplaySettingsEx(dd.DeviceName, &dm, NULL, CDS_TEST, NULL));
	}
	/// check given resolution 
	bool check_resolution(unsigned int w, unsigned int h) const
	{
		DEVMODE dm;
		memset(&dm,0,sizeof(DEVMODE));
		dm.dmSize = sizeof(DEVMODE);
		dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
		dm.dmPelsWidth  = w;
		dm.dmPelsHeight = h;
		return interpret_error(
			ChangeDisplaySettingsEx(dd.DeviceName, &dm, NULL, CDS_TEST, NULL));
	}
	/// check given bit depth
	bool check_bit_depth(unsigned int bits) const
	{
		DEVMODE dm;
		memset(&dm,0,sizeof(DEVMODE));
		dm.dmSize = sizeof(DEVMODE);
		dm.dmFields = DM_BITSPERPEL;
		dm.dmBitsPerPel = bits;
		return interpret_error(
			ChangeDisplaySettingsEx(dd.DeviceName, &dm, NULL, CDS_TEST, NULL));
	}
	/// check given refresh rate 
	bool check_refresh_rate(unsigned int hz) const
	{
		DEVMODE dm;
		memset(&dm,0,sizeof(DEVMODE));
		dm.dmSize = sizeof(DEVMODE);
		dm.dmFields = DM_DISPLAYFREQUENCY;
		dm.dmDisplayFrequency = hz;
		return interpret_error(
			ChangeDisplaySettingsEx(dd.DeviceName, &dm, NULL, CDS_TEST, NULL));
	}
	/// check given position 
	bool check_position(const display_position& dp) const
	{
		return check_position(dp.x,dp.y);
	}
	/// check given position 
	bool check_position(unsigned int x, unsigned int y) const
	{
		DEVMODE dm;
		memset(&dm,0,sizeof(DEVMODE));
		dm.dmSize = sizeof(DEVMODE);
		dm.dmFields = DM_POSITION;
		dm.dmPosition.x = x;
		dm.dmPosition.y = y;
		return interpret_error(
			ChangeDisplaySettingsEx(dd.DeviceName, &dm, NULL, CDS_TEST, NULL));
	}
};
#endif

std::vector<display*>& ref_displays()
{
	static std::vector<display*> displays;
	return displays;
}

const std::vector<display*>& display::get_displays()
{
	return ref_displays();
}

/// clear the list of previously scanned devices and free all allocated memory
void display::clear_displays()
{
	for (unsigned int i=0; i<ref_displays().size(); ++i)
		delete ref_displays()[i];
	ref_displays().clear();
}
/// enumerate the current displays and ignore the mirrored displays that do not correpond to physical devices
void display::scan_displays(bool only_physical_displays)
{
	clear_displays();
#ifdef _MSC_VER
	windows_display* d = new windows_display;
	int i = 0;
	while (d->attach(i)) {
		if (d->is_mirror() && only_physical_displays)
			delete d;
		else
			ref_displays().push_back(d);
		++i;
		d = new windows_display;
	}
	delete d;
#else
	std::cerr << "scan_displays in cgv/utils/display.cxx not implemented" << std::endl;
#endif
}

/// activate all displays according to the registered settings
bool display::activate_all()
{
#ifdef _MSC_VER
	LONG result = ChangeDisplaySettingsEx(NULL, NULL, NULL, 0, NULL);
	bool ret = true;
	for (unsigned int i=0; i<ref_displays().size(); ++i)
		ret = static_cast<windows_display*>(ref_displays()[i])->update_after_display_change(result) && ret;
	return ret;
#else
	std::cerr << "activate_all in cgv/utils/display.cxx not implemented" << std::endl;
	return false;
#endif
}

/// activate all displays according to the registered settings
bool display::check_activate_all()
{
#ifdef _MSC_VER
	return windows_display::interpret_error(ChangeDisplaySettingsEx(NULL, NULL, NULL, CDS_TEST, NULL));
#else
	std::cerr << "check_activate_all in cgv/utils/display.cxx not implemented" << std::endl;
	return false;
#endif
}

/// show all available displays
void display::show_all_displays(bool only_physical_displays)
{
	scan_displays(only_physical_displays);
	const std::vector<display*>& displays = get_displays();
	for (unsigned int i=0; i<displays.size(); ++i)
		std::cout << "display " << i << ": " << *displays[i] << std::endl;
}
	}
}
