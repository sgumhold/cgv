/* check_input is a non recursive function that completes the input defined
   by the user in the project file */

// Update the list of dependent projects and move libs that are installed
// on the system to the addDependencies define.
@define(excludeProjectDeps=[])

@if(SYSTEM=="linux")@{
	@define(excludeProjectDeps=["fltk","pthread","libjpeg","libpng","zlib","libtiff"])
@}
@if(addProjectDeps !~ UNDEF)@{
	@define(prjDeps=addProjectDeps)
	@define(addProjectDeps=[])
	@for(i=0;i<!prjDeps;++i)@{
		@if(prjDeps[i] < excludeProjectDeps)@{
			@if(addDependencies ~~ UNDEF) @{
				@define(addDependencies = [prjDeps[i]])
			@}
			@else@{
				@define(addDependencies = addDependencies.[prjDeps[i]])
			@}
		@}
		@else@{
			@define(addProjectDeps = addProjectDeps.[prjDeps[i]])
		@}
	@}
@}
@define(found_error = 0)
@if(projectName ~~ UNDEF)@{
	@define(projectName=(INPUT_FILE-"."))
	@warning(0; "project name not defined, derived name is ".projectName)
@}
@if(projectType ~~ UNDEF)@{
	@warning(0; "<projectType> not defined in ".input_path." using 'plugin' as default.")
	@define(projectType="plugin")
@}
@if(projectGUID ~~ UNDEF)@{
	@error(0; "required define <projectGUID> missing in ".input_path)
	@define(found_error = 1)
@}
@define(projectGUID = ++projectGUID)
@if(workspaceGUID ~~ UNDEF)@{
	@define(workspaceGUID="8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942")
@}
@if(defFile ~~ UNDEF)@{
	@define(defFile="")
@}
@if(0 == !sourceDirs)@{
	@if(0 == !sourceFiles)@{
		@define(sourceDirs=[INPUT_DIR])
	@}
@}
@for(i=0;i<!excludeSourceDirs;++i)@{
	@define(excludeSourceDirs[i]=excludeSourceDirs[i]*"|\\|/|")
@}
@if(found_error)@{
	@exit(-1)
@}
