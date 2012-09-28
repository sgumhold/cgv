<?xml version="1.0" encoding="Windows-1252"?>
@//function call to <make/ppp/templates/check_input.tm>
@exclude<make/ppp/templates/check_input.tm>
@//function call to <make/ppp/templates/interpret_input.tm>
@exclude<make/ppp/templates/interpret_input.tm>
@//compute project_map and project_by_type with the function dep_prj
@define(build_target_postfix=".vcproj")
@define(command_postfix=".bat")
@exclude<make/ppp/templates/dep_prj.tm>
@// define further configuration dependent quantities
@define(output_post=["_s".CGV_COMPILER_VERSION,"_sd".CGV_COMPILER_VERSION,
		               "_".CGV_COMPILER_VERSION,"_d".CGV_COMPILER_VERSION])
@define(config_name=["Release","Debug","Release Dll","Debug Dll"])
@define(debug_flavor=["","",0,0])
@//finally start to define the project user file
<VisualStudioUserFile
	ProjectType="Visual C++"
	Version=@"CGV_COMPILER_VERSION.',00'"
	ShowAllFiles="false"
	>
	<Configurations>
@for(ci=0;ci<nr_configs;++ci)@{
	@insert<make/ppp/templates/vs_config_user.tm>@}	</Configurations>
</VisualStudioUserFile>
