<?xml version="1.0" encoding="Windows-1252"?>
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
@//function call to <make/ppp/templates/extend_input.tm>
@define(commonDefs=["WIN32", "INPUT_DIR=".(INPUT_DIR*"|\\|/|"), 
		            "CGV_DIR=".(CGV_DIR*"|\\|/|"), ["_WINDOWS","_CONSOLE"][exec_idx] ])
@if(CGV_PLATFORM=="x64")@{
@define(commonDefs=commonDefs.["WIN64"])
@}
@define(configDefsAdd=[ [["_LIB"],[]][exec_idx], [["_USRDLL"],[]][exec_idx] ])
@define(configDefs=[["NDEBUG","CGV_FORCE_STATIC"].configDefsAdd[0],
		              ["_DEBUG","CGV_FORCE_STATIC"].configDefsAdd[0],
						  ["NDEBUG"].configDefsAdd[1],
						  ["_DEBUG"].configDefsAdd[1]])
@define(mapDeps::opengl = ["opengl32","opengl32","opengl32","opengl32"])
@define(mapDeps::glu    = ["glu32","glu32","glu32","glu32"])
@//define(mapDeps::glew   = ["glew32s","glew32s","glew32","glew32"])
@//define(mapDeps::fltk   = ["fltk2.lib fltk2_gl","fltk2d.lib fltk2_gld",
@//		                     "fltk2dll","fltk2dlld"])
@define(list_separator=";")
@define(inc_prefix="")
@define(libdir_prefix="")
@define(def_prefix="")
@define(lib_prefix="")
@define(lib_postfix=".lib")
@exclude<make/ppp/templates/extend_input.tm>
@//define further configuration dependent quantities
@define(sub_system=[2,1][exec_idx])
@define(config_type=[[4,4,2,2],[1,1,1,1]][exec_idx])
@define(output_ext=[["lib","lib","dll","dll"],["exe","exe","exe","exe"]][exec_idx])
@define(output_post=["_s".CGV_COMPILER_VERSION,"_sd".CGV_COMPILER_VERSION,
							"_".CGV_COMPILER_VERSION,"_d".CGV_COMPILER_VERSION])
@if(projectType=="tool")@{
	@define(output_post[0]="")
@}
@define(output_dir=[["lib","lib"],["bin/release".platform_post,"bin/debug".platform_post]][exec_idx].["bin/release".platform_post,"bin/debug".platform_post])
@define(config_name=["Release","Debug","Release Dll","Debug Dll"])
@define(static_linker_tool=["VCLibrarianTool","VCLinkerTool"][exec_idx])
@define(linker_tool=[static_linker_tool,static_linker_tool,"VCLinkerTool","VCLinkerTool"])
@define(link_inc=[1,2,1,2])
@define(use_lib_deps=[1,1,0,0])
@//finally start to define the project file
<VisualStudioProject
	ProjectType="Visual C++"
	Version=@if(CGV_COMPILER_VERSION==10)@{"9,00"@}@else@{@"CGV_COMPILER_VERSION.',00'"@}
	Name=@"projectName"
	ProjectGUID=@"'{'.projectGUID.'}'"
	RootNamespace=@"projectName"
	Keyword="Win32Proj"
	>
  <Platforms>
    <Platform
			Name="Win32"
		/>
  </Platforms>
  <ToolFiles>
    <ToolFile RelativePath=@"CGV_DIR.'/make/vs/cgv_rules.rules'" />
    <ToolFile RelativePath=@"CGV_DIR.'/make/vs/cgv_'.CGV_COMPILER.'_rules.rules'" />@for(ri=0;ri<!addRuleFiles;++ri)@{
	<ToolFile RelativePath=@"addRuleFiles[ri]" />@}
  </ToolFiles>
  <Configurations>
@for(ci=0;ci<nr_configs;++ci)@{
	@insert<make/ppp/templates/vs_config.tm>
@}
	</Configurations>
	<References>
	</References>
	<Files>
@//go through all source directories
@for(dir=0;dir<(!sourceDirs);++dir)@{
	@// add a filter per source dir in case of multiple source dirs or the definition of separate source files
	@if(((!sourceDirs) > 1) | ((!sourceFiles) > 0))@{
		@define(sdn = sourceDirs[dir])
		@if((sdn-'/\\:')!=sdn)@{
			@define(sdn = sdn+((!(sdn-'/\\:'))+1))
		@}
	<Filter Name=@"sdn">
	@}
	@// perform recursive call on vs_files, where the list named "ns"
	@// is used to store the namespaces of the recursive function calls
	@// as MAPs.
	@define(ns=[MAP])
	@// init the local variable "dir" to the current source directory
	@define(ns[-1]::dir = sourceDirs[dir])
	@// perform function call
	@insert<make/ppp/templates/vs_files.tm>
	@if(((!sourceDirs) > 1) | ((!sourceFiles) > 0))@{
	</Filter>
	@}
@}
@// define a separate filter for source files in the sourceFiles define
@if((!sourceFiles) > 0)@{
	@// collect all files based on rules
	@define(collect_recursive=0)
	@define(input_directories=[])
	@define(input_files=sourceFiles)
	@define(ignore_files=[])
	@define(ignore_directories=[])
	@if(!excludeSourceFiles)@{
		@define(ignore_files=excludeSourceFiles)
	@}
	@if(!excludeSourceDirs)@{
		@define(ignore_directories=excludeSourceDirs)
	@}
	@exclude<make/ppp/templates/collect_files.tm>
	@if(!folder_list)@{
	<Filter Name="further">
	@// generate filters and source file references
	@for(fi=0; fi<(!folder_list); ++fi)@{
		@if(folder_list[fi] != "root")@{
		<Filter Name=@"folder_list[fi]">@}
			@define(L=folder_map[folder_list[fi]])
			@for(i=0; i<(!L); ++i)@{
				<File RelativePath=@"L[i]*'|/|\\|'"></File>@}
		@if(folder_list[fi] != "root")@{
		</Filter>@}@}
	</Filter>@}
@}
@// finally add the project file itself as a source file to allow rebuilding the project
@if(!(INPUT_PATH < excludeSourceFiles))@{
	<File RelativePath=@"INPUT_PATH*'|/|\\|'">
			<FileConfiguration
				Name="Release|Win32"
				ExcludedFromBuild="true"
				>
				<Tool
					Name="generate make file"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Debug|Win32"
				ExcludedFromBuild="true"
				>
				<Tool
					Name="generate make file"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Release Dll|Win32"
				ExcludedFromBuild="true"
				>
				<Tool
					Name="generate make file"
				/>
			</FileConfiguration>
			<FileConfiguration
				Name="Debug Dll|Win32"
				ExcludedFromBuild="true"
				>
				<Tool
					Name="generate make file"
				/>
			</FileConfiguration>
	</File>
@}
	</Files>
	<Globals>
	</Globals>
</VisualStudioProject>
