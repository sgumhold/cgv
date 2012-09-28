/* extend_dep_projects examines the source files in order to derive tools necessary
   for executing rules on the source files. These tools are added to the dependent
	projects.
	
	Inputs:
	- sourceFiles:LIST of STRING ... list of source file names.
	- sourceDirs:LIST of STRING ... list of source file directories, which are scaned recursively
	- excludeSourceFiles:LIST of STRING ... list of to be excluded source files
	- excludeSourceDirs:LIST of STRING ... list of to be excluded source directories
	- projectDeps:LIST of STRING ... list of dependent projects

	Outputs:
	- projectDeps:LIST of STRING ... list of dependent projects extended by necessary tools
	- folder_map and folder_list as resulting from collect_files function
	*/
@// call collect_files with recursive directory search
@define(collect_recursive=1)
@if((!sourceFiles) > 0)@{
	@define(input_files=sourceFiles)
@}
@if((!sourceDirs) > 0)@{
	@define(input_directories=sourceDirs)
@}
@if((!excludeSourceFiles) > 0)@{
	@define(ignore_files=excludeSourceFiles)
@}
@if((!excludeSourceDirs) > 0)@{
	@define(ignore_directories=excludeSourceDirs)
@}
@exclude<make/ppp/templates/collect_files.tm>
@// check for each rule
@for(ri=0; ri<!rules; ++ri)@{
	@// with output folder
	@if(rules[ri]::folder !~ UNDEF)@{
		@// that received source files
		@if(folder_map[rules[ri]::folder] ~~ LIST)@{
			@if((!folder_map[rules[ri]::folder]) > 0)@{
				@// whether the rule needs a tool
				@if(rules[ri]::tool !~ UNDEF)@{
					@// and check if this tool is not yet a dependent project
					@if(!(rules[ri]::tool < projectDeps))@{
						@// in this case extend list of dependent projects
						@define(projectDeps = projectDeps.[rules[ri]::tool])
						@//cout("adding project ".rules[ri]::tool." based on rule\n")
@}@}@}@}@}@}
