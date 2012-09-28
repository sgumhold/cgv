@=

exclude<make/ppp/templates/system.ppp>
command_postfix = script_ext; 
fix_build_target_name=".cproject";
config_name=["release","debug","shared_release", "shared_debug"];

func(clean_gcc_target; :>target="", :>return="")
{
	return = target*"|\\|/|"*"| |\\\\ |"
}
