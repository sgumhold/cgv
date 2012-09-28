Microsoft Visual Studio Solution File, Format Version @if(CGV_COMPILER_VERSION==10)@{@(CGV_COMPILER_VERSION)@}@else@{@(CGV_COMPILER_VERSION+1)@}.00
# Visual Studio @if(CGV_COMPILER_VERSION==10)@{2008@}@else@{@(1981+CGV_COMPILER_VERSION*3)@}
@//function call to <make/ppp/templates/check_input.tm>
@exclude<make/ppp/templates/check_input.tm>
@//function call to <make/ppp/templates/interpret_input.tm>
@exclude<make/ppp/templates/interpret_input.tm>
@//compute project_map and project_by_type with the function dep_prj
@define(build_target_postfix=".vcproj")
@define(command_postfix=".bat")
@exclude<make/ppp/templates/dep_prj.tm>
@//function call to <make/ppp/templates/extend_input_workspace.tm>
@exclude<make/ppp/templates/extend_input_workspace.tm>
@// call extend_dep_projects with the input variables prepared before
@exclude<make/ppp/templates/extend_dep_projects.tm>
@exclude<make/ppp/templates/dep_prj.tm>
@//one more define
@define(config_name=["Release","Debug","Release Dll","Debug Dll"])
@//start solution definition
@//Project(@"'{'.workspaceGUID.'}'") = @"projectName", @"projectName.'.vcproj'", @"'{'.projectGUID.'}'"

Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = @"projectName", @"projectName.'.vcproj'", @"'{'.projectGUID.'}'"
@if(projectDeps !~ UNDEF)@{
	@if((!projectDeps) > 0)@{
	ProjectSection(ProjectDependencies) = postProject
		@for(i=0;i<!projectDeps;++i)@{
		{@(project_map[projectDeps[i]]::projectGUID)} = {@(project_map[projectDeps[i]]::projectGUID)}
		@}
	EndProjectSection
	@}
@}
EndProject
@for(i=0;i<!project_map;++i)@{
	@define(info = project_map[project_map[i]])
Project(@"'{'.workspaceGUID.'}'") = @"info::projectName", @"'..\\'.(info::projectName).'\\'.(info::projectName).'.vcproj'", @"'{'.(info::projectGUID).'}'"
	@if(info::projectDeps !~ UNDEF)@{
		@if((!info::projectDeps)>0)@{
	ProjectSection(ProjectDependencies) = postProject
			@for(j=0;j<!info::projectDeps;++j)@{
		{@(project_map[(info::projectDeps)[j]]::projectGUID)} = {@(project_map[(info::projectDeps)[j]]::projectGUID)}
			@}
	EndProjectSection
		@}
	@}
EndProject
@}
@define(added_project_folder=0)
@define(project_folders=["plugin","test","application","library","static_library","tool"])
@if(project_folder_support)@{
	@for(i=0; i<!project_folders; ++i)@{
		@if(project_by_type[project_folders[i]] !~ UNDEF)@{
			@if((!project_by_type[project_folders[i]]) > 0) @{
				@define(added_project_folder=1)
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = @"project_folders[i]", @"project_folders[i]", @"'{5A46AE2A-975D-4570-BE19-A5B7B8C13E2'.i.'}'"
EndProject
			@}
		@}
	@}
@}
Global
	GlobalSection(SolutionConfigurationPlatforms) = preSolution
@for(ci=0;ci<nr_configs;++ci)@{
		@(config_name[ci])|Win32 = @(config_name[ci])|Win32@}
	EndGlobalSection
	GlobalSection(ProjectConfigurationPlatforms) = postSolution
@for(ci=0;ci<nr_configs;++ci)@{
		{@(projectGUID)}.@(config_name[ci])|Win32.ActiveCfg = @(config_name[ci])|Win32
		{@(projectGUID)}.@(config_name[ci])|Win32.Build.0 = @(config_name[ci])|Win32@}
@for(i=0;i<!project_map;++i)@{
	@define(info = project_map[project_map[i]])
	@for(ci=0;ci<nr_configs;++ci)@{
		{@(info::projectGUID)}.@(config_name[ci])|Win32.ActiveCfg = @(config_name[ci])|Win32
		{@(info::projectGUID)}.@(config_name[ci])|Win32.Build.0 = @(config_name[ci])|Win32@}
@}
	EndGlobalSection
	GlobalSection(SolutionProperties) = preSolution
		HideSolutionNode = FALSE
	EndGlobalSection
@if(added_project_folder)@{
	GlobalSection(NestedProjects) = preSolution
	@for(i=0; i<!project_folders; ++i)@{
		@if(project_by_type[project_folders[i]] !~ UNDEF)@{
			@if((!project_by_type[project_folders[i]]) > 0) @{
				@for(j=0; j<!project_by_type[project_folders[i]]; ++j)@{
				   @if(project_map[project_by_type[project_folders[i]][j]] ~~ UNDEF)@{
				       @error(0;"could not find project ".project_by_type[project_folders[i]][j]." in lookup table!")
				   @}
				   @else@{
		{@(project_map[project_by_type[project_folders[i]][j]]::projectGUID)} = {5A46AE2A-975D-4570-BE19-A5B7B8C13E2@(i)}
		           @}
				@}
			@}
		@}
	@}
	EndGlobalSection
@}
EndGlobal
