/* interpret_input is non recursive function that interprets the
   project input defines and derives new defines.

	Outputs:
	- build_dir:STRING   ... directory in which the project targets are located
	- exec_idx:INT       ... 1 for executables and 0 otherwise
	- projectDeps:LIST of STRING ... list of dependent projects
	- projectDirs:LIST of STRING ... list of project directories
	- nr_configs ... number of configurations, which is 1 for tools and 4 otherwise
*/
@//define output directory of the generated target files 
@define(build_dir=CGV_BUILD.'/'.CGV_COMPILER)
@if(CGV_IDE ~~ STRING)@{
	@define(build_dir=CGV_BUILD.'/'.CGV_IDE)
@}
@define(build_dir_proj=build_dir.'/'.projectName)
@define(rules[0]["rules"][0]["path"] = build_dir_proj)
@if(!?build_dir_proj)@{
	@warning(0; "creating build directory ".build_dir_proj)
	@define(res=0)
	@if(SYSTEM=="windows")@{
		@system('mkdir "'.(build_dir_proj*"|/|\\|").'"'; res)
	@}
	@else@{
		@system('mkdir -p "'.(build_dir_proj*"|\\|/|").'"'; res)
	@}
	@if(res != 0)@{
		@error(0; "could not create build directory ".build_dir_proj)
		@exit(-1)
	@}
@}
@//define exec_idx to be 1 for executable targets and 0 otherwise
@define(exec_idx=0)
@if(projectType=="application" | projectType=="tool")@{
	@define(exec_idx=1)
@}

@//concatenate default and user defined project directories
@define(projectDirs = [CGV_DIR.'/tool',CGV_DIR.'/cgv',CGV_DIR.'/apps'])
@if(addProjectDirs !~ UNDEF)@{
	@define(projectDirs = projectDirs.addProjectDirs)
@}
@//concatenate default and user defined project dependencies
@define(projectDeps = [])
@if(projectType == "test")@{
	@define(projectDeps = projectDeps."tester")
@}
@if(addProjectDeps !~ UNDEF)@{
	@define(projectDeps = projectDeps.addProjectDeps)
@}
@//define numer of configurations
@if(projectType=="tool")@{
	@define(nr_configs=1)
@}
@else@{
	@if(projectType=="static_library")@{
		@define(nr_configs=2)
	@}
	@else@{
		@define(nr_configs=4)
	@}
@}
