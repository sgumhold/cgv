/* dep_prj is a non recursive function that collects information on all
   recursively dependent project as specified in projectDeps. It also builds
	the project files of dependent projects that do not exist by invoking ppp
	recursively.
	
	Inputs:
	- fix_build_target_name:STRING ... if defined this is used directly as build target name
	- build_target_postfix:STRING ... postfix to CGV_BUILD/CGV_COMPILER/projectName for at least one
	      build target, which is checked for existence before ppp is invoked recursively.
	- command_postfix:STRING ... postfix for the script that invokes ppp recursively.

	Outputs:
	- project_map:MAP of MAP ... maps each project name of a dependent project to
	       a map with the following information
			 - projectName:STRING
			 - projectType:STRING
			 - projectGUID:STRING
			 - projectDeps:LIST of STRING
			 - addProjectDirs:LIST of STRING
	- project_by_type:MAP of LIST of STRING ... maps each projectType "library",
	       "plugin", ... to a list of strings specifying the dependent projects
			 of the given projectType.
   - application_name:STRING ... name of application project that is to be launched in
	       case of a plugin project
   - plugin_list:LIST of STRING ... list of plugin names that should be specified in
	       command line arguments to the launched application
	*/
@//prepare maps for output
@define(project_map = MAP)
@define(project_by_type= MAP)
@if(folder_map ~~ UNDEF)@{
	@define(folder_map=MAP)
@}
@if(folder_list ~~ UNDEF)@{
	@define(folder_list=[])
@}
@//prepare temporary maps used to avoid double processing
@define(project_dir_map = MAP)
@define(project_loc_map = MAP)
@//save the project definition
@define(     projectType_tmp=     projectType)
@define(     projectName_tmp=     projectName)
@define(     projectGUID_tmp=     projectGUID)
@define(  build_dir_proj_tmp=  build_dir_proj)
@define(     projectDeps_tmp=     projectDeps)
@define(  addProjectDeps_tmp=  addProjectDeps)
@define(  addProjectDirs_tmp=  addProjectDirs)
@define(      sourceDirs_tmp=      sourceDirs)
@define(     sourceFiles_tmp=     sourceFiles)
@define(excludeSourceDirs_tmp =excludeSourceDirs)
@define(excludeSourceFiles_tmp=excludeSourceFiles)
@define(renameObjectFiles_tmp =renameObjectFiles)
@define(objectFileMap_tmp =objectFileMap)
@define(      addIncDirs_tmp=      addIncDirs)
@define(  addShaderPaths_tmp=      addShaderPaths)
@define(      addDefines_tmp=      addDefines)
@define(addSharedDefines_tmp=addSharedDefines)
@define(addStaticDefines_tmp=addStaticDefines)
@define(      addLibDirs_tmp=      addLibDirs)
@define( addDependencies_tmp= addDependencies)
@define( addCommandLineArguments_tmp= addCommandLineArguments)
@define(     projectDirs_tmp=     projectDirs)
@define(     projectDeps_tmp=     projectDeps)
@define(        exec_idx_tmp=        exec_idx)
@define(      nr_configs_tmp=      nr_configs)
@define(           rules_tmp=           rules)
@define(      folder_map_tmp=      folder_map)
@define(     folder_list_tmp=     folder_list)
@define(         defFile_tmp=         defFile)
@define(       useOpenMP_tmp=       useOpenMP)
@define(cppLanguageStandard_tmp=cppLanguageStandard)
@define(charset_tmp=charset)
@define(subsystem_tmp=subsystem)
@define(    addRuleFiles_tmp=    addRuleFiles)
@define(        addRules_tmp=        addRules)
@if(application_name)@{
	@define(application_name_tmp=application_name)
@}
@//recursive call to dep_prj_rec
@define(ns = [MAP.[["prjs",projectDeps],["dirs",projectDirs]]])
@exclude<make/ppp/templates/dep_prj_rec.tm>
@//recover project definitions
@define(     projectType=     projectType_tmp)
@define(     projectName=     projectName_tmp)
@define(     projectGUID=     projectGUID_tmp)
@define(     projectDeps=     projectDeps_tmp)
@define(  addProjectDeps=  addProjectDeps_tmp)
@define(  addProjectDirs=  addProjectDirs_tmp)
@define(  build_dir_proj=  build_dir_proj_tmp)
@define(      sourceDirs=      sourceDirs_tmp)
@define(      addIncDirs=      addIncDirs_tmp)
@define(  addShaderPaths=  addShaderPaths_tmp)
@define(      addDefines=      addDefines_tmp)
@define(     sourceFiles=     sourceFiles_tmp)
@define(excludeSourceDirs=excludeSourceDirs_tmp)
@define(excludeSourceFiles=excludeSourceFiles_tmp)
@define(renameObjectFiles =renameObjectFiles_tmp)
@define(objectFileMap =objectFileMap_tmp)
@define(addSharedDefines=addSharedDefines_tmp)
@define(addStaticDefines=addStaticDefines_tmp)
@define(      addLibDirs=      addLibDirs_tmp)
@define( addDependencies= addDependencies_tmp)
@define( addCommandLineArguments= addCommandLineArguments_tmp)
@define(     projectDirs=     projectDirs_tmp)
@define(     projectDeps=     projectDeps_tmp)
@define(        exec_idx=        exec_idx_tmp)
@define(      nr_configs=      nr_configs_tmp)
@define(           rules=           rules_tmp)
@define(      folder_map=      folder_map_tmp)
@define(     folder_list=     folder_list_tmp)
@define(       useOpenMP=       useOpenMP_tmp)
@define(cppLanguageStandard=cppLanguageStandard_tmp)
@define(charset=charset_tmp)
@define(subsystem=subsystem_tmp)
@define(         defFile=         defFile_tmp)
@define(    addRuleFiles=    addRuleFiles_tmp)
@define(        addRules=        addRules_tmp)
@if(application_name_tmp)@{
	@define(application_name=application_name_tmp)
@}
@//cout("project map = ")@cout(project_map.[])@cout("\n \n")
@//cout("project by type: ")@cout(project_by_type.[])@cout("\n \n")
@//extract the application name
@if(projectType == "plugin")@{
	@if(project_by_type::application !~ UNDEF)@{
		@define(application_name = (project_by_type::application)[0])
		@if((!(project_by_type::application)) > 1)@{
			@warning(7; "project ".projectName." depends on several applications:")
			@cout("   ")@cout(project_by_type::application)@cout("\n")
			@cout("   ".application_name." chosen as command\n")
		@}
	@}
@}
@elif(projectType == "test")@{
	@define(application_name = "tester")
@}
@//extract plugin list
@define(plugin_list=[])
@if(project_by_type::plugin !~ UNDEF)@{
	@define(plugin_list=project_by_type::plugin)
@}
@if(project_by_type::test !~ UNDEF)@{
	@define(plugin_list=plugin_list.(project_by_type::test))
@}
@if((projectType == "plugin") | (projectType == "test"))@{
	@define(plugin_list=plugin_list.(projectName.""))
@}
@//cout(plugin_list.[])@cout("\n")