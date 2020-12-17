#pragma once

#include <vector>
#include <iostream>

#include "lib_begin.h"

namespace cgv {
	namespace os {

/// a display mode describes a valid setting of the display parameters
struct display_mode
{
	/// width in pixel
	unsigned int width;
	/// height in pixel
	unsigned int height;
	/// number of bits per pixel
	unsigned int bit_depth;
	/// refresh rate in Hz
	unsigned int refresh_rate;
	/// constructor
	display_mode(unsigned int w = 1024, unsigned int h = 768, unsigned int bd = 32, unsigned int rr = 60)
		: width(w), height(h), bit_depth(bd), refresh_rate(rr) {}
};

/// out streaming of a display mode
extern CGV_API std::ostream& operator << (std::ostream& os, const display_mode& dm);

/// the display position gives the signed x and y coordinates on the virtual screen
struct display_position
{
	/// signed x position on virtual screen
	int x;
	/// signed y position on virtual screen
	int y;
	/// constructor
	display_position(int _x = 0, int _y = 0) : x(_x), y(_y) {}
};

/// out streaming of a display position
extern CGV_API std::ostream& operator << (std::ostream& os, const display_position& dp);

/// the display configuration combines a display mode and the position of the display on the virtual screen
struct display_configuration
{
	/// display mode
	display_mode mode;
	/// display position
	display_position position;
	/// constructor
	display_configuration(const display_mode& dm = display_mode(), const display_position& dp = display_position())
		: mode(dm), position(dp) {}
};

/// out streaming of a display configuration
extern CGV_API std::ostream& operator << (std::ostream& os, const display_configuration& dc);

/** differnt flags for filtering the scanning of displays */
enum DisplayScanMode
{
	DSM_ALL = 0,       /// no restriction results in all displays
	DSM_PHYSICAL = 1,  /// restriction to physical displays
	DSM_ACTIVE = 2,    /// restriction to active displays (can be combined with restriction to physical)
	DSM_PASSIVE = 4    /// restriction to passive displays (can be combined with restriction to physical)
};

/** the display class gives access to the display modes of all 
	 available displays. An example application can be found in
	 <apps/display>. It provides a command line interface to the
	 implementation.
*/
class CGV_API display
{
public:
	/**@name static methods dealing with all displays*/
	//@{
	/// if one of the methods return false, this static member gives textual information about the reason
	static std::string last_error;
	/// scan all available displays and ignore the mirrored displays that do not correspond to physical devices if \c only_physical_displays is set to true
	static void scan_displays(DisplayScanMode mode = DSM_ALL);
	/// return the list of previously scanned display devices
	static const std::vector<display*>& get_displays();
	/// clear the list of previously scanned devices and free all allocated memory
	static void clear_displays();
	/// activate all displays according to the registered settings
	static bool activate_all();
	/// check if all displays can be activated according to the registered settings
	static bool check_activate_all();
	/// show all available displays
	static void show_all_displays(DisplayScanMode scan_mode = DSM_ALL);
	//@}

	/// virtual destructor frees all platform specifically allocated memory
	virtual ~display() {}

	/**@name access to available display modes */
	//@{
	/// enumerate all available display modes. If the display is active only the display modes supported by the attached monitor are enumerated
	virtual void enumerate_display_modes(std::vector<display_mode>& display_modes, bool compatible_with_attached_monitor = true) = 0;
	/// print out all available display modes
	virtual void show_display_modes(bool compatible_with_attached_monitor = true) = 0;
	//@}

	/**@name read only information about the display*/
	//@{
	/// return the name of the display
	virtual std::string get_name() const = 0;
	/// return the description string of the display
	virtual std::string get_description() const = 0;
	/// return the unique ID string of the display
	virtual std::string get_ID() const = 0;
	/// return whether display is a mirrored display
	virtual bool is_mirror() const = 0;
	/// return whether display is removable
	virtual bool is_removable() const = 0;
	//@}

	/**@name global configuration*/
	//@{
	/// return whether display is active
	virtual bool is_active() const = 0;
	/// activate the display based on the registered settings or change the current display settings to the registered settings if the display is already active. Return whether activation or change of settings was successful.
	virtual bool activate() = 0;
	/// check if activation of the registered settings will work
	virtual bool check_activate() const = 0;
	/// deactivate the display, return whether deactivation was successful
	virtual bool deactivate() = 0;
	/// return whether display is the primary display
	virtual bool is_primary() const = 0;
	/// make this display the primary display
	virtual bool make_primary() = 0;
	/// check if this display can be made primary
	virtual bool can_be_primary() const = 0;
	//@}

	/**@name access to current display configuration if display is active*/
	//@{
	/// return the current display configuration
	virtual display_configuration get_configuration() const = 0;
	/// return the currently set display mode
	virtual display_mode get_mode() const = 0;
	/// return the current width (only defined if display is active)
	virtual unsigned int get_width() const = 0;
	/// return the current height (only defined if display is active)
	virtual unsigned int get_height() const = 0;
	/// return the current bit depth of the display pixels (only defined if display is active)
	virtual unsigned int get_bit_depth() const = 0;
	/// return the current refresh rate in Hz (only defined if display is active)
	virtual unsigned int get_refresh_rate() const = 0;
	/// return the current display position
	virtual display_position get_position() const = 0;
	/// return the current current x coordinate within the virtual screen spanned by the desktop (only defined if display is active)
	virtual int get_x() const = 0;
	/// return the current current y coordinate within the virtual screen spanned by the desktop (only defined if display is active)
	virtual int get_y() const = 0;

	/// set the display configuration immediately, return whether this was successful.
	virtual bool set_configuration(const display_configuration& dc) = 0;
	/// set the given display mode immediately, return whether this was successful. Only valid display modes can be set.
	virtual bool set_mode(const display_mode& dm) = 0;
	/// set new resolution of the display immediately, return whether this was successful. Only resolutions that correspond to valid display modes can be set.
	virtual bool set_resolution(unsigned int w, unsigned int h) = 0;
	/// set new bit depth of the display immediately, return whether this was successful. Only bit depths that correspond to valid display modes can be set.
	virtual bool set_bit_depth(unsigned int bits) = 0;
	/// set new refresh rate of the display immediately, return whether this was successful. Only refresh rates that correspond to valid display modes can be set.
	virtual bool set_refresh_rate(unsigned int hz) = 0;
	/// set the display position immediately, return whether this was successful. Only positions that result in a tight non overlapping packing of displays can be set.
	virtual bool set_position(const display_position& dp) = 0;
	/// set new position of the display within the virtual screen immediately, return whether this was successful. Only positions that result in a tight non overlapping packing of displays can be set.
	virtual bool set_position(unsigned int x, unsigned int y) = 0;
	//@}

	/**@name registration of new configuration and test of configuration */
	//@{
	/// return the display mode registered in the registry
	virtual display_mode get_registered_mode() const = 0;
	/// return the registered width (this is changed by prepare_resolution)
	virtual unsigned int get_registered_width() const = 0;
	/// return the registered height (this is changed by prepare_resolution)
	virtual unsigned int get_registered_height() const = 0;
	/// return the registered bit depth of a display pixel (this is changed by prepare_bit_depth)
	virtual unsigned int get_registered_bit_depth() const = 0;
	/// return the registered refresh rate in Hz (this is changed by prepare_refresh_rate)
	virtual unsigned int get_registered_refresh_rate() const = 0;

	/// register new display configuration, which is activated in the next activate() or activate_all() call
	virtual bool register_configuration(const display_configuration& dc) = 0;
	/// register new display mode, which is activated in the next activate() or activate_all() call
	virtual bool register_mode(const display_mode& dm) = 0;
	/// register new resolution of the display, which is activated in the next activate() or activate_all() call
	virtual bool register_resolution(unsigned int w, unsigned int h) = 0;
	/// register new bit depth of the display, which is activated in the next activate() or activate_all() call
	virtual bool register_bit_depth(unsigned int bits) = 0;
	/// register new refresh rate of the display, which is activated in the next activate() or activate_all() call
	virtual bool register_refresh_rate(unsigned int hz) = 0;
	/// register new position immediately, which is activated in the next activate() or activate_all() call
	virtual bool register_position(const display_position& dp) = 0;
	/// register new position of the display, which is activated in the next activate() or activate_all() call
	virtual bool register_position(unsigned int x, unsigned int y) = 0;

	/// check given display configuration
	virtual bool check_configuration(const display_configuration& dc) const = 0;
	/// check given display mode
	virtual bool check_mode(const display_mode& dm) const = 0;
	/// check given resolution 
	virtual bool check_resolution(unsigned int w, unsigned int h) const = 0;
	/// check given bit depth
	virtual bool check_bit_depth(unsigned int bits) const = 0;
	/// check given refresh rate 
	virtual bool check_refresh_rate(unsigned int hz) const = 0;
	/// check given position 
	virtual bool check_position(const display_position& dp) const = 0;
	/// check given position 
	virtual bool check_position(unsigned int x, unsigned int y) const = 0;
	//@}
};

/// print out the information of a display
extern CGV_API std::ostream& operator << (std::ostream& os, const display& disp);

	}
}

#include <cgv/config/lib_end.h>