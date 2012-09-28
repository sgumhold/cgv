/* extend_input is non recursive function that extends the
   project input defines by configuration specific defines and dependencies

	Inputs:
	- commonDefs:LIST of STRING... list of platform specific defines 
											 which are independent of configuration
	- configDefs:LIST of LIST of STRING ... quadrupels of lists of strings 
	                               that define for each configuration a list
											 of configuration specific defines
	- mapDeps:MAP ... maps a shortcut for a dependent library (i.e. opengl) to
	                  a list of configuration specific library names, one entry for each of the
	                  four configurations
	- inc_prefix:STRING    ... prefix add before each include dir, i.e. '-I"' or ''
    - libdir_prefix:STRING  ... prefix added to each library directory
	                            in the library directories, i.e. '-L"' or ''
	- dir_suffix:STRING ... suffix for include path or library path definition, i.e. '"' or ''
	- list_separator:STRING ... string used to separate include dirs, 
	                            defines and libraries i.e. " " or ";"
	- def_prefix:STRING    ... prefix add before each define, i.e. "-D " or ""
    - lib_prefix:STRING  ... prefix added to each library name 
	                         in the dependencies, i.e. "-l " or ""
    - lib_postfix:STRING ... postfix appended to each library name in 
	                         the dependencies, i.e. "" or ".lib"

	Outputs:
	- config_defines:LIST of STRING ... for each configuration a string 
	                         with all defines
	- dependencies:LIST of STRING ... for each configuration a string 
	                         with all dependencies
	- includeDirs:STRING ... <list_separator>-separated list of include 
	                         directories that are prefixed with <inc_prefix>
	- libDirs:STRING     ... <list_separator>-separated list of library 
							 directories that are prefixed with <libdir_prefix>
*/

@// translate dependencies to config specific lists of library names
@define(cfg_deps=[[],[],[],[]])
@//cout("adddep = ")@cout(addDependencies.[])@cout("\n")
@if(addDependencies)@{
	@for(i=0;i<!addDependencies;++i)@{
		@define(dep=addDependencies[i])
		@//cout("dep = ")@cout(dep)@cout(" / ")@cout(dep.[])@cout("\n")
		@if(dep ~~ LIST)@{
			@define(dep = addDependencies[i][0])
		@}
		@//cout("dep = ")@cout(dep)@cout(" / ")@cout(dep.[])@cout("\n")
		@if(mapDeps[dep])@{
			@define(newDep=mapDeps[dep])
		@}
		@else@{
			@define(newDep=[dep,dep,dep,dep])
		@}
		@for(ci=0;ci<4;++ci)@{
			@if(!(newDep[ci] < cfg_deps[ci]))@{
				@define(cfg_deps[ci]=cfg_deps[ci].[newDep[ci].""])@}
		@}
	@}
@}
@//cout(cfg_deps.[])@cout("\n")
@for(i=0;i<!project_map;++i)@{
	@if(project_map[project_map[i]]::addDefines !~ UNDEF)@{
		@define(prjDefs = project_map[project_map[i]]::addDefines)
		@for(j=0;j<!prjDefs;++j)@{
			@define(ldefs=prjDefs[j])
			@if(ldefs ~~ LIST)@{
				@define(addDefines=addDefines.[ldefs[0].""])
			@}
		@}
	@}
	@if(project_map[project_map[i]]::addSharedDefines !~ UNDEF)@{
		@define(prjDefs = project_map[project_map[i]]::addSharedDefines)
		@for(j=0;j<!prjDefs;++j)@{
			@define(ldefs=prjDefs[j])
			@if(ldefs ~~ LIST)@{
				@define(addSharedDefines=addSharedDefines.[ldefs[0].""])
			@}
		@}
	@}
	@if(project_map[project_map[i]]::addStaticDefines !~ UNDEF)@{
		@define(prjDefs = project_map[project_map[i]]::addStaticDefines)
		@for(j=0;j<!prjDefs;++j)@{
			@define(ldefs=prjDefs[j])
			@if(ldefs ~~ LIST)@{
				@define(addStaticDefines=addStaticDefines.[ldefs[0].""])
			@}
		@}
	@}
	@if(project_map[project_map[i]]::addIncDirs !~ UNDEF)@{
		@define(prjIncs = project_map[project_map[i]]::addIncDirs)
		@for(j=0;j<!prjIncs;++j)@{
			@define(lincs=prjIncs[j])
			@if(lincs ~~ LIST)@{
				@define(addIncDirs=addIncDirs.[lincs[0].""])
			@}
		@}
	@}
	@if(project_map[project_map[i]]::addLibDirs !~ UNDEF)@{
		@define(prjLibs = project_map[project_map[i]]::addLibDirs)
		@for(j=0;j<!prjLibs;++j)@{
			@define(llibs=prjLibs[j])
			@if(llibs ~~ LIST)@{
				@define(addLibDirs=addLibDirs.[llibs[0].""])
			@}
		@}
	@}
	@if(project_map[project_map[i]]::addDependencies !~ UNDEF)@{
		@define(prjDeps = project_map[project_map[i]]::addDependencies)
		@for(j=0;j<!prjDeps;++j)@{
			@define(ldep=prjDeps[j])
			@if(ldep ~~ LIST)@{
				@define(export_type=[])
				@if(ldep[1] == "static")@{@define(export_type=[1,1,0,0])@}
				@if(ldep[1] == "shared")@{@define(export_type=[0,0,1,1])@}
				@if(ldep[1] == "all")@{@define(export_type=[1,1,1,1])@}
				@define(dep = ldep[0])
				@//cout(ldep.[])@cout(" -> ".dep."\n")@cout(export_type.[])@cout("\n")
				@if((!export_type) > 0)@{
					@if(mapDeps[dep])@{
						@define(newDep=mapDeps[dep])
					@}
					@else@{
						@define(newDep=[dep,dep,dep,dep])
					@}
					@for(ci=0;ci<4;++ci)@{
						@if(export_type[ci])@{
							@if(!(newDep[ci] < cfg_deps[ci]))@{@define(cfg_deps[ci]=cfg_deps[ci].[newDep[ci].""])@}
						@}
					@}
				@}
			@}
		@}
	@}
@}



@//concatenate defines that are common for all configurations
@define(cds = commonDefs)
@for(i=0;i<!addDefines;++i)@{
	@if(addDefines[i] ~~ LIST)@{
		@define(cds = cds.[addDefines[i][0].""])
	@}
	@else@{
		@define(cds = cds.[addDefines[i].""])
	@}
@}
@if(!cds)@{
	@define(common_defs = def_prefix.cds[0])
	@for(i=1;i<!cds;++i)@{	   
		@define(common_defs=common_defs.list_separator.def_prefix.cds[i])
	@}
@}
@else@{
	@define(common_defs="")
@}
@define(config_defines = [common_defs,common_defs,common_defs,common_defs])
@//append config defs
@for(i=0;i<4;++i)@{
	@for(j=0; j<!(configDefs[i]); ++j)@{
		@if (config_defines[i] == "")@{
			@define(config_defines[i] = def_prefix.configDefs[i][j])
		@}
		@else@{
			@define(config_defines[i] = config_defines[i].list_separator.def_prefix.(configDefs[i][j]))
		@}
	@}
@}
@//concatenate configuration specific defines 
@for(i=0;i<!addStaticDefines;++i)@{
	@define(sd=addStaticDefines[i])
	@if(addStaticDefines[i] ~~ LIST)@{
		@define(sd=addStaticDefines[i][0])
	@}
	@for(j=0;j<2;++j)@{
		@if (config_defines[j] == "")@{
			@define(config_defines[j] = def_prefix.sd)
		@}
		@else@{
			@define(config_defines[j] = config_defines[j].list_separator.def_prefix.sd)
		@}
	@}
@}
@for(i=0;i<!addSharedDefines;++i)@{
	@define(sd=addSharedDefines[i])
	@if(addSharedDefines[i] ~~ LIST)@{
		@define(sd=addSharedDefines[i][0])
	@}
	@for(j=2;j<4;++j)@{
		@if (config_defines[j] == "")@{
			@define(config_defines[j] = def_prefix.sd)
		@}
		@else@{
			@define(config_defines[j] = config_defines[j].list_separator.def_prefix.sd)
		@}
	@}
@}
@define(dependencies=["","","",""])
@//cout("dependencies of project ".projectName."\n")
@for(ci=0;ci<4;++ci)@{
	@for(i=0;i<!cfg_deps[ci];++i)@{
		@if(i==0)@{
			@define(dependencies[ci] = lib_prefix.cfg_deps[ci][i].lib_postfix)
		@}
		@else@{
			@define(dependencies[ci] = dependencies[ci]." ".lib_prefix.cfg_deps[ci][i].lib_postfix)
		@}
	@}
	@//cout(ci.' : '.dependencies[ci].'\n')
@}

@//concatenate default and user defined include directories
@if(dir_suffix~~UNDEF)@{
	@define(dir_suffix="")
@}
@define(includeDirs=inc_prefix."$(CGV_DIR)".dir_suffix)
@//cout(projectName." inc dirs = ")@cout(addIncDirs.[])@cout("\n")
@if(addIncDirs)@{
	@for(i=0;i<!addIncDirs;++i)@{
		@if(addIncDirs[i] ~~ LIST)@{
			@define(includeDirs=includeDirs.list_separator.inc_prefix.addIncDirs[i][0].dir_suffix)
		@}
		@else@{
			@define(includeDirs=includeDirs.list_separator.inc_prefix.addIncDirs[i].dir_suffix)
		@}
	@}
@}
@//concatenate default and user defined library directories
@define(libDirs=libdir_prefix."$(CGV_DIR)/lib".dir_suffix.list_separator.libdir_prefix."$(CGV_BUILD)/lib".dir_suffix)
@if(addLibDirs)@{
	@for(i=0;i<!addLibDirs;++i)@{
		@if(addLibDirs[i] ~~ LIST)@{
			@define(libDirs=libDirs.list_separator.libdir_prefix.addLibDirs[i][0].dir_suffix)
		@}
		@else@{
			@define(libDirs=libDirs.list_separator.libdir_prefix.addLibDirs[i].dir_suffix)
		@}
	@}
@}
