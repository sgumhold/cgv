#include <cgv/os/display.h>
#include <cgv/utils/scan.h>
#include <cgv/utils/tokenizer.h>

using namespace cgv::os;
using namespace cgv::utils;

void show_usage()
{
	std::cout << "usage:\ndisplay [p][+|-] ... show all [physical] [active|passive] displays\ndisplay command1 command2 ... commandn\n\n"
		<< "command syntax:\n"
		<< "   [0-9]  ... show display with given index\n"
		<< "   [0-9]m ... show monitor compatible modes of given display\n"
		<< "   [0-9]M ... show all modes of given display\n"
		<< "   [0-9]! ... make primary display (only possible if display is not removable)\n"
		<< "   [0-9]- ... deactivate display\n"
		<< "   [0-9]+ ... activate display to registered settings\n"
		<< "   [0-9]c'configuration' ... check if display allows configuration\n"
		<< "   [0-9]r'configuration' ... register given configuration for display\n"
		<< "   [0-9]s'configuration' ... set given configuration for display\n"
		<< "   = ... activate all registered configurations\n\n"
		<< "configuration syntax:\n"
		<< "   [WxH[xB][:R]][@X,Y] with\n" 
		<< "      W ... width\n"
		<< "      H ... height\n"
		<< "      B ... bit depth (8/16/32)\n"
		<< "      R ... refresh rate in Hz\n"
		<< "      X ... x-position on virtual screen (can be negative)\n"
		<< "      Y ... y-position on virtual screen (can be negative)\n"
		<< "examples:\n"
		<< "   1s1400x1050:75 ... set resolution and refresh rate of display 1\n"
		<< "   1s@-1400,0     ... set position on virtual screen to left of primary\n"
		<< "   1r320x240x8    ... register resolution 320x240 with 8 bit per pixel\n" << std::endl
		<< "   0r800x600x16:60@0,600 ... register full specification for display 0" << std::endl
		<< std::endl;
}

bool parse_display_configuration(const std::string& command, 
											display_configuration& dc, 
											bool& mode_changed, bool& position_changed)
{
	mode_changed = false;
	position_changed = false;

	std::string s = command.substr(1);
	tokenizer t(s);
	t.set_sep("x:,@");
	std::vector<token> toks;
	t.bite_all(toks);
	int tmp;
	unsigned int i = 0;
	if (is_integer(to_string(toks[0]),tmp)) {
		dc.mode.width = tmp;
		mode_changed = true;
		if (toks.size() < 3)
			return false;
		if (to_string(toks[1]) != "x")
			return false;
		if (!is_integer(to_string(toks[2]),tmp))
			return false;
		dc.mode.height = tmp;
		i = 3;
		if (toks.size() > 3) {
			if (to_string(toks[3]) == "x") {
				if (toks.size() < 5)
					return false;
				if (!is_integer(to_string(toks[4]),tmp))
					return false;
				dc.mode.bit_depth = tmp;
				i = 5;
			}
			if (toks.size() > i) {
				if (to_string(toks[i]) == ":") {
					if (toks.size() < i+2)
						return false;
					if (!is_integer(to_string(toks[i+1]),tmp))
						return false;
					dc.mode.refresh_rate = tmp;
					i += 2;
				}
			}
		}
	}
	if (toks.size() > i) {
		if (toks[i] == "@") {
			if (toks.size() < i+4)
				return false;
			if (!is_integer(to_string(toks[i+1]),tmp))
				return false;
			dc.position.x = tmp;
			if (toks[i+2] != ",")
				return false;
			if (!is_integer(to_string(toks[i+3]),tmp))
				return false;
			dc.position.y = tmp;
			position_changed = true;
			i += 4;
		}
	}
	if (i < toks.size())
		return false;
	return true;
}

bool check(bool result, bool report_success = false)
{
	if (result) {
		if (report_success)
			std::cout << "check resulted in success" << std::endl;
	}
	else
		std::cout << "error: " << display::last_error.c_str() << std::endl;
	return result;
}

int main(int argc, char** argv) 
{
	if (argc == 1) {
		display::show_all_displays();
		return 1;
	}
	if (argc == 2) {
		if (argv[1][0] == 'p' || argv[1][0] == '+' || argv[1][0] == '-') {
			DisplayScanMode mode = DSM_ALL;
			int j = 0;
			while (argv[1][j] != 0 && j < 2) {
				switch (argv[1][j]) {
				case 'p': mode = DisplayScanMode(mode | DSM_PHYSICAL); break;
				case '+': mode = DisplayScanMode(mode | DSM_ACTIVE); break;
				case '-': mode = DisplayScanMode(mode | DSM_PASSIVE); break;
				}
				++j;
			}
			display::show_all_displays(mode);
			return 1;
		}
	}
	const std::vector<display*>& displays = display::get_displays();
	bool mode_changed, position_changed;
	for (int i=1; i<argc; ++i) {
		if (argv[i][0] == '=') {
			check(display::activate_all());
			continue;
		}
		if (is_digit(argv[i][0])) {
			int idx = argv[i][0]-'0';
			if (idx >= (int)displays.size()) {
				std::cerr << "display " << idx << " not available" << std::endl;
				return -1;
			}
			std::string command = std::string(argv[i]+1);
			if (command.empty()) {
				std::cout << "display " << idx << " " << *displays[idx] << std::endl;
				continue;
			}
			display_configuration dc;
			switch (command[0]) {
			case 'm' :
				std::cout << "monitor compatible display modes of display " << idx << ":\n------------------------------------------------\n";
				displays[idx]->show_display_modes();
				std::cout << std::endl;
				break;
			case 'M' :
				std::cout << "all display modes of display " << idx << ":\n-------------------------------\n";
				displays[idx]->show_display_modes(false);
				std::cout << std::endl;
				break;
			case '!' :
				check(displays[idx]->make_primary());
				break;
			case '+' :
				check(displays[idx]->activate());
				break;
			case '-' :
				check(displays[idx]->deactivate());
				break;
			case 'c' :
				dc.mode = displays[idx]->get_mode();
				if (parse_display_configuration(command,dc,mode_changed,position_changed)) {
					if (mode_changed) {
						if (position_changed)
							check(displays[idx]->check_configuration(dc), true);
						else
							check(displays[idx]->check_mode(dc.mode), true);
					} else if (position_changed)
						check(displays[idx]->check_position(dc.position), true);
					break;
				}
			case 'r' :
				dc.mode = displays[idx]->get_registered_mode();
				if (parse_display_configuration(command,dc,mode_changed,position_changed)) {
					if (mode_changed) {
						if (position_changed)
							check(displays[idx]->register_configuration(dc), true);
						else
							check(displays[idx]->register_mode(dc.mode), true);
					} else if (position_changed)
						check(displays[idx]->register_position(dc.position), true);
					break;
				}
			case 's' :
				if (parse_display_configuration(command,dc,mode_changed,position_changed)) {
					check(displays[idx]->set_configuration(dc));
					if (mode_changed) {
						if (position_changed)
							check(displays[idx]->set_configuration(dc), true);
						else
							check(displays[idx]->set_mode(dc.mode), true);
					} else if (position_changed)
						check(displays[idx]->set_position(dc.position), true);
					break;
				}
			default:
				std::cout << "wrong syntax in argument " << i << "!!! (" << argv[i] << ")" << std::endl;
				show_usage();
				break;
			}
		}
		else
			show_usage();
	}
}