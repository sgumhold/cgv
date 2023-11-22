/* dep_prj_rec is a recursive function 
*/
@//cout('prjs=')@cout(ns[-1]::prjs)@cout('\n')
@//cout('dirs=')@cout(ns[-1]::dirs)@cout('\n')
// process the project dirs non recursively
@for(i=0; i < !ns[-1]::dirs; ++i)@{
	@define(dir = ns[-1]["dirs"][i])
	// check if the project dir has been processed
	@if(project_dir_map[dir] ~~ UNDEF)@{
		// if not mark as processed
		@define(project_dir_map[dir] = 1)
		// scan project files recursively in project dir
		@define(pjs=[])
		@dir(dir;"*.pj";1;pjs)
		@for(j=0;j<(!pjs);++j)@{
			// and store them in the project location map
			@define(project_loc_map[pjs[j][0]-3] = pjs[j][1])
			@//cout('added project location '.(pjs[j][0]-3).': '.pjs[j][1]."\n")
		@}
	@}
@}


@define(callPrefix="call ");
@define(slashReplace="|/|\\|");
@if(SYSTEM=="linux")@{
	@define(callPrefix="");
	@define(slashReplace="|\\|/|");
@}

// process all projects recursively
@for(ns[-1]::i=0; ns[-1]::i < !(ns[-1]::prjs); ns[-1]::i=ns[-1]::i+1)@{
	// process only not yet processed projects
	@if(project_map[(ns[-1]::prjs)[ns[-1]::i]] ~~ UNDEF)@{
		// check if project file location is missing
		@if(project_loc_map[(ns[-1]::prjs)[ns[-1]::i]] ~~ UNDEF)@{
			@error(3;"could not find dependent project ".(ns[-1]::prjs)[ns[-1]::i])
		@}
		@else@{
			// ensure that the project file has been compiled
			@if(fix_build_target_name ~~ STRING)@{
				@define(build_target=build_dir.'/'.(ns[-1]::prjs)[ns[-1]::i].'/'.fix_build_target_name)
			@}
			@else@{
				@define(build_target=build_dir.'/'.(ns[-1]::prjs)[ns[-1]::i].'/'.(ns[-1]::prjs)[ns[-1]::i].build_target_postfix)
			@}

			
			@if(!?build_target)@{
				@define(ppp_postfix=CGV_COMPILER)
				@if(CGV_IDE ~~ STRING)@{
					@define(ppp_postfix=CGV_IDE)
				@}
				@cout("\nrecursive call to ppp for project <".(ns[-1]::prjs)[ns[-1]::i].">\n")
				@define(system_result=0);
				@cout('\n'.callPrefix.'"'.CGV_DIR.'/bin/ppp_'.ppp_postfix.command_postfix.'" "'.project_loc_map[(ns[-1]::prjs)[ns[-1]::i]].'"'*slashReplace)
				@cout("\n\n")
				@system(callPrefix.'"'.CGV_DIR.'/bin/ppp_'.ppp_postfix.command_postfix.'" "'.project_loc_map[(ns[-1]::prjs)[ns[-1]::i]].'"'*slashReplace; system_result)
				@cout("\n")
			@}
			// read the project file of the dependent project
			@define(sourceDirs=[])
			@define(sourceFiles=[])
			@define(INPUT_DIR_temp=INPUT_DIR)
			@exclude(project_loc_map[(ns[-1]::prjs)[ns[-1]::i]])
			@define(INPUT_DIR=(project_loc_map[(ns[-1]::prjs)[ns[-1]::i]]-"/"))
			@exclude<make/ppp/templates/check_input.tm>
			@exclude<make/ppp/templates/interpret_input.tm>
			@exclude<make/ppp/templates/extend_dep_projects.tm>
			@define(INPUT_DIR=INPUT_DIR_temp)
			// incorporate into project_by_type maps
			@if(project_by_type[projectType] ~~ UNDEF)@{
				@define(project_by_type[projectType] = [projectName.''])
			@}
			@else@{
				@define(project_by_type[projectType] = project_by_type[projectType].(projectName.''))
			@}
			// store information in the project map
			@define(prj_map=MAP)
			@define(prj_map::projectName=projectName)
			@define(prj_map::projectType=projectType)
			@define(prj_map::projectDeps=projectDeps)
			@define(prj_map::projectGUID=projectGUID)
			@define(prj_map::addProjectDirs=addProjectDirs)
			@define(prj_map::addIncDirs=addIncDirs)
			@define(prj_map::addShaderPaths=addShaderPaths)
			@define(prj_map::addLibDirs=addLibDirs)
			@define(prj_map::addDefines=addDefines)
			@define(prj_map::addSharedDefines=addSharedDefines)
			@define(prj_map::addStaticDefines=addStaticDefines)
			@define(prj_map::addDependencies=addDependencies)
			@define(prj_map::addRuleFiles=addRuleFiles)
			@define(prj_map::addRules=addRules)
			@//cout('project '.(ns[-1]::prjs)[ns[-1]::i].' = ')@cout(prj_map.[])@cout('\n')
			@define(project_map[(ns[-1]::prjs)[ns[-1]::i]]=prj_map)
			// prepare recursion
			@define(ns = ns.[MAP.[["prjs",projectDeps],["dirs",addProjectDirs]]])
			// recursive call
			@exclude<make/ppp/templates/dep_prj_rec.tm>
			// pop namespace stack
			@define(ns = ns-1)
		@}
	@}
@}
