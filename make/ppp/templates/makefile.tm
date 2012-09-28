@//function call to <make/ppp/templates/check_input.tm>
@exclude<make/ppp/templates/check_input.tm>
include $(CGV_DIR)/make/gcc/rules
@//function call to <make/ppp/templates/interpret_input.tm>
@exclude<make/ppp/templates/interpret_input.tm>
@//compute project_map and project_by_type with the function dep_prj
@define(fix_build_target_name="makefile")
@if(SYSTEM == "windows")@{
	@define(command_postfix=".bat")@}
@else@{
	@define(command_postfix=".sh")@}
@exclude<make/ppp/templates/dep_prj.tm>
@//function call to <make/ppp/templates/extend_input_workspace.tm>
@exclude<make/ppp/templates/extend_input_workspace.tm>
@// call extend_dep_projects with the input variables prepared before
@exclude<make/ppp/templates/extend_dep_projects.tm>
@exclude<make/ppp/templates/dep_prj.tm>
@//function call to <make/ppp/templates/extend_input.tm>
@define(commonDefs=["WIN32", "INPUT_DIR=".(INPUT_DIR*"|\\|/|"), 
		            "CGV_DIR=".(CGV_DIR*"|\\|/|"), ["_WINDOWS","_CONSOLE"][exec_idx] ])
@define(configDefsAdd=[ [["_LIB"],[]][exec_idx], [["_USRDLL"],[]][exec_idx] ])
@define(configDefs=[["NDEBUG","CGV_FORCE_STATIC"].configDefsAdd[0],
		              ["_DEBUG","CGV_FORCE_STATIC"].configDefsAdd[0],
						  ["NDEBUG"].configDefsAdd[1],
						  ["_DEBUG"].configDefsAdd[1]])
@define(mapDeps::opengl = ["opengl32","opengl32","opengl32","opengl32"])
@define(mapDeps::glu    = ["glu32","glu32","glu32","glu32"])
@define(mapDeps::glew   = ["glew32","glew32","glew32.dll","glew32.dll"])
@define(list_separator=" ")
@define(inc_prefix='-I"')
@define(libdir_prefix='-L"')
@define(dir_suffix='"')
@define(def_prefix="-D")
@define(lib_prefix="-l")
@define(lib_postfix="")
@exclude<make/ppp/templates/extend_input.tm>
@define(config_name=["release","debug","shared_release", "shared_debug"])
@define(config_suffix=["_4","_d4","_4", "_d4"])
@define(clean_path_gcc='|\\|/|')
@define(clean_spaces='| |\\\\ |')
@if(SYSTEM == "windows")@{
	@define(target_ext=".exe")
	@define(shared_suffix=".dll")
	@define(clean_path='|/|\\|')
	@define(rm_fct="del")
	@define(rmdir_fct="rmdir /S /Q")
	@define(mkdir_fct="mkdir")
	@define(copy_fct="copy")@}
@else@{
	@define(target_ext="")
	@define(shared_suffix=".so")
	@define(clean_path='|\\|/|')
	@define(rm_fct="rm")
	@define(rmdir_fct="rmdir")
	@define(mkdir_fct="mkdir -p")
	@define(copy_fct="cp")@}
@define(configOptions=[ "-fpermissive -Os","-fpermissive -g", 
					   ["-fpermissive -shared -fPIC -Wl,-soname,lib$(target_name)".config_suffix[2].".dll -Wl,--out-implib,".CGV_INSTALL."/lib/lib$(target_name)".config_suffix[2].".dll.a ", ""][exec_idx]."-Os",
					   ["-fpermissive -shared -fPIC -Wl,-soname,lib$(target_name)".config_suffix[3].".dll -Wl,--out-implib,".CGV_INSTALL."/lib/lib$(target_name)".config_suffix[3].".dll.a ", ""][exec_idx]."-g"])
@define(config_target_name=[(["lib",""][exec_idx])."$(target_name)".config_suffix[0].[".a",target_ext][exec_idx],
		                    (["lib",""][exec_idx])."$(target_name)".config_suffix[1].[".a",target_ext][exec_idx],
		                    "$(target_name)".config_suffix[2].[shared_suffix,target_ext][exec_idx],
		                    "$(target_name)".config_suffix[3].[shared_suffix,target_ext][exec_idx]])
@if(projectType=="tool")@{
	@define(config_target_name[0] = "$(target_name)".target_ext)@}
@define(config_build_dir=[CGV_INSTALL."/".["lib","bin"][exec_idx]."/",
		                  CGV_INSTALL."/".["lib","bin"][exec_idx]."/",
						  CGV_INSTALL."/bin/",
						  CGV_INSTALL."/bin/"])
target_name = @(projectName)
empty = 
export tab = $(empty)	$(empty)
export CC  = gcc
export CXX = g++
@// collect all files based on rules
@define(collect_recursive=1)
@define(input_files=sourceFiles)
@define(input_directories=sourceDirs)
@if(!excludeSourceFiles)@{
	@define(ignore_files=excludeSourceFiles)@}
@if(!excludeSourceDirs)@{
	@define(ignore_directories=excludeSourceDirs)@}
@exclude<make/ppp/templates/collect_files.tm>
@// sort files that match rule sources into a map of rules 
@define(my_rules=[])
@if(!folder_list)@{
	@for(ri=0; ri<(!rules); ++ri)@{
		@if( (rules[ri]::folder < folder_list) & (rules[ri]::rules ~~ LIST) )@{
			@define(R = rules[ri])
			@define(L = folder_map[R::folder])
			@define(S = [])
			@define(T = [])
			@define(t_all = [])
			@for(i=0; i<(!L); ++i)@{
				@define(ext = L[i]+(1+!(L[i]-".")))
				@if(ext < R::extensions)@{
					@define(t=[])
					@define(S=S.[L[i]])
					@for (rj=0; rj<!(R::rules); ++rj)@{
						@define(r=(R::rules)[rj])
						@define(target=(L[i]-"."))
						@if(r::keep_extension !~ UNDEF)@{
							@if(r::keep_extension)@{
								@define(target=L[i])@}@}
						@if(r::path !~ UNDEF)@{
							@define(target=(target+(1+!(target-"/\\:"))))
							@define(target=r::path."/".target)@}
						@if(r::suffix !~ UNDEF)@{
							@define(target=target.r::suffix)@}
						@if(r::extension !~ UNDEF)@{
							@define(target=target.".".r::extension)@}
						@define(target=target)
						@define(t=t.[target])@}
					@define(t_all = t_all.t)
					@define(T = T.[t])@}@}
			@if(!S)@{
				@define(R::sources = S)
				@define(R::targets = T)
				@define(R::all_targets = t_all)
				@define(my_rules = my_rules.[R])@}@}@}@}
@define(state_rules="")
@if(!my_rules)@{
	@define(state_rules="rules")@}
export sources = @for(i=0; i<!(folder_map::sources); ++i)@{@("   ")@(((folder_map::sources)[i]*clean_path_gcc)*clean_spaces) \
@}
export src_names = $(basename $(notdir $(sources)))
export objs = $(addsuffix .o,$(src_names))
export deps = $(addsuffix .d,$(src_names))

@for(j=0;j<!project_map;++j)@{
	@define(prj_name=project_map[j]::projectName)
ifeq ($(@(prj_name)_considered),1)
	ignore_@(prj_name)=1
else
	export @(prj_name)_considered=1
endif
@}

.PHONY: all clean @for(i=0; i<nr_configs;++i)@{@(config_name[i]." ".config_name[i]."_only ".config_name[i]."_makefile ")@}@(state_rules)@for(ri=0; ri<!my_rules; ++ri)@{@(" ".(my_rules[ri]::folder)."_rules")@}

all: release

@if(!my_rules)@{			
rules: @for(ri=0; ri<!my_rules; ++ri)@{
	@(" ".(my_rules[ri])::folder."_rules")@}

@for(ri=0; ri<!my_rules; ++ri)@{
@("\n".my_rules[ri]::folder."_rules:")@for(i=0;i<!(my_rules[ri]::all_targets);++i)@{
	@('   '.(((my_rules[ri]::all_targets)[i]*clean_path_gcc)*clean_spaces).' \\\n')@}

@for(i=0;i<!(my_rules[ri]::targets);++i)@{
	@for(j=0;j<!((my_rules[ri]::targets)[i]);++j)@{
		@((((my_rules[ri]::targets)[i])[j]*clean_path_gcc)*clean_spaces)@(" ")@}@(": ")@(((my_rules[ri]::sources)[i]*clean_path_gcc)*clean_spaces)@("\n")
		@if(my_rules[ri]::folder == "resources")@{@if(my_rules[ri]::tool == "res_prep")@{
			@("\t")"$(CGV_BUILD)\bin\res_prep" @"(my_rules[ri]::sources)[i]*clean_path"@(" ")@"((my_rules[ri]::targets)[i])[0]*clean_path"@("\n\n")
		@}@}
@}@}@}

clean:
@for(i=0; i<nr_configs;++i)@{
	@("\t@".rmdir_fct." ".config_name[i]."_dir")@("\n\t")
	@("@".rm_fct.' "'.(config_build_dir[i].config_target_name[i]*clean_path).'"')
@}
@("\n")@for(i=0; i<nr_configs;++i)@{
	@(config_name[i]): @(config_name[i])_dir/makefile @(config_name[i])_deps@(" ".state_rules)
	@("\n\t@")echo building @(projectName."::".config_name[i])
	$(MAKE) -C @(config_name[i])_dir

@(config_name[i])_only: @(config_name[i])_dir/makefile@(" ".state_rules)
	$(MAKE) -C @(config_name[i])_dir

@(config_name[i])_dir/%.o: @(config_name[i])_dir/makefile
	$(MAKE) -C @(config_name[i])_dir $(notdir $@("@"))

@define(project_dep_list="")
@(config_name[i])_deps:@for(j=0;j<!project_map;++j)@{
	@define(cj=i)
	@if( (project_map[j]::projectType=="plugin")|(project_map[j]::projectType=="library") )@{
		@if(i>1)@{
			@define(project_dep_list=project_dep_list." -l".project_map[j]::projectName.config_suffix[i].shared_suffix)@}
		@else@{
			@define(project_dep_list=project_dep_list." -l".project_map[j]::projectName.config_suffix[i])@}@}
	@else@{@if(project_map[j]::projectType=="static_library")@{
		@if(i>1)@{
			@define(cj=i-2)
			@define(project_dep_list=project_dep_list." -l".project_map[j]::projectName.config_suffix[i-2])@}
		@else@{
			@define(project_dep_list=project_dep_list." -l".project_map[j]::projectName.config_suffix[i])@}@}
	@else@{@if(project_map[j]::projectType=="tool")@{
		@define(cj=0)@}@}@}
ifneq ($(ignore_@(project_map[j]::projectName)),1)
	$(MAKE) -C ../@(project_map[j]::projectName." ".config_name[cj])
endif@}

@(config_name[i])_dir/makefile: makefile @(config_name[i])_dir @((CGV_DIR."/make/ppp/templates/makefile.tm"*clean_path_gcc)*clean_spaces)@(" ")@((CGV_DIR."/make/gcc/rules"*clean_path_gcc)*clean_spaces)@("\n\t")
	@("@echo ")CPPFLAGS = @(configOptions[i]." ".includeDirs." ".config_defines[i]*clean_path) > @(config_name[i])_dir\makefile
	@("@echo ")LIBFLAGS = @(libDirs.project_dep_list." ".dependencies[i]*clean_path) >> @(config_name[i])_dir\makefile
	@("@echo ")target = @(config_build_dir[i].config_target_name[i]*clean_path) >> @(config_name[i])_dir\makefile
	@("@echo.") >> @(config_name[i])_dir\makefile
	@("@echo ").PHONY: all >> @(config_name[i])_dir\makefile
	@("@echo.") >> @(config_name[i])_dir\makefile
	@("@echo ")all: $$(target) >> @(config_name[i])_dir\makefile
	@("@echo.") >> @(config_name[i])_dir\makefile
	@("@echo ")$$(target): $$(objs) >> @(config_name[i])_dir\makefile@if(i + 4*exec_idx < 2)@{
		@("\n\t@echo ")$(tab)ar rv $$(target) $$(objs) >> @(config_name[i])_dir\makefile@("\n\t")@}
	@else@{
		@("\n\t@echo ")$(tab)$$(CXX) $$(CPPFLAGS) -o $$(target) $$(objs) $$(LIBFLAGS) >> @(config_name[i])_dir\makefile@("\n\t")@}
	@("@echo.") >> @(config_name[i])_dir\makefile
	@("@echo ")%%.d: makefile >> @(config_name[i])_dir\makefile
	@("@echo ")$(tab)@('@$$(CC) -MM $$(CPPFLAGS) $$(filter %%/$$(basename $$(notdir $$@)).c %%/$$(basename $$(notdir $$@)).cxx %%/$$(basename $$(notdir $$@)).cpp,$$(sources)) ^> $$@.temp >> '.config_name[i])_dir\makefile
	@("@echo ")$(tab)@('@sed "s,^[^.]*\.o[: ],$$@ &," ^< $$@.temp ^> $$@') >> @(config_name[i])_dir\makefile
	@("@echo ")$(tab)@("@echo $$(tab)$$(CC) $$(CPPFLAGS) -w -c $$(filter %%/$$(basename $$(notdir $$@)).c %%/$$(basename $$(notdir $$@)).cxx %%/$$(basename $$(notdir $$@)).cpp,$$(sources)) -o $$(subst .d,.o,$$@) ^>^> $$@") >> @(config_name[i])_dir\makefile
	@("@echo ")$(tab)@('@$$(CXX) -MM $$(CPPFLAGS) $$(filter %%/$$(basename $$(notdir $$@)).c %%/$$(basename $$(notdir $$@)).cxx %%/$$(basename $$(notdir $$@)).cpp,$$(sources)) ^> $$@.temp >> '.config_name[i])_dir\makefile
	@("@echo ")$(tab)@('@sed "s,^[^.]*\.o[: ],$$@ &," ^< $$@.temp ^> $$@') >> @(config_name[i])_dir\makefile
	@("@echo ")$(tab)@("@echo all: ^> $$@.temp_2") >> @(config_name[i])_dir\makefile
	@("@echo ")$(tab)@('@echo ifeq ("$$(call suffix, $$(filter %%/$$(basename $$(notdir $$@)).c %%/$$(basename $$(notdir $$@)).cxx %%/$$(basename $$(notdir $$@)).cpp,$$(sources)))",".c") ^>^> $$@.temp_2 >> '.config_name[i])_dir\makefile
	@("@echo ")$(tab)@("@echo $$(tab)@echo $$(tab)$$(CC) $$(CPPFLAGS) -w -c $$(filter %%/$$(basename $$(notdir $$@)).c %%/$$(basename $$(notdir $$@)).cxx %%/$$(basename $$(notdir $$@)).cpp,$$(sources)) -o $$(subst .d,.o,$$@) ^^^>^^^> $$@ ^>^> $$@.temp_2") >> @(config_name[i])_dir\makefile
	@("@echo ")$(tab)@('@echo else ^>^> $$@.temp_2 >> '.config_name[i])_dir\makefile
	@("@echo ")$(tab)@("@echo $$(tab)@echo $$(tab)$$(CXX) $$(CPPFLAGS) -w -c $$(filter %%/$$(basename $$(notdir $$@)).c %%/$$(basename $$(notdir $$@)).cxx %%/$$(basename $$(notdir $$@)).cpp,$$(sources)) -o $$(subst .d,.o,$$@) ^^^>^^^> $$@ ^>^> $$@.temp_2") >> @(config_name[i])_dir\makefile
	@("@echo ")$(tab)@('@echo endif ^>^> $$@.temp_2 >> '.config_name[i])_dir\makefile
	@("@echo ")$(tab)@("@$$(MAKE) -f $$@.temp_2") >> @(config_name[i])_dir\makefile
	@("@echo ")$(tab)@("@".rm_fct." $$@.temp_2") >> @(config_name[i])_dir\makefile
	@("@echo ")$(tab)@("@".rm_fct." $$@.temp") >> @(config_name[i])_dir\makefile
	@("@echo.") >> @(config_name[i])_dir\makefile
	@("@echo ")include $(deps) >> @(config_name[i])_dir\makefile

@(config_name[i])_dir:
	@(mkdir_fct." ".config_name[i]."_dir")

@}
