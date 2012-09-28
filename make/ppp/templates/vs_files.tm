@// collect all sub directories of the currently considered directory
@define(ns[-1]::subdirs=[])
@dir(ns[-1]::dir;".";0;ns[-1]::subdirs)
@// iterate found sub directories
@for(ns[-1]::i=0;ns[-1]::i<(!ns[-1]::subdirs);ns[-1]::i=ns[-1]::i+1)@{
	@//check for exclusion
	@if(!((ns[-1]::subdirs)[ns[-1]::i][1]*"|\\|/|" < excludeSourceDirs))@{
	<Filter Name=@"(ns[-1]::subdirs)[ns[-1]::i][0]">
		@//prepare recursion by appending a nested namespace to the namespace list
		@define(ns = ns.[MAP])
		@//set the to be processed directory to the current sub directory
		@define(ns[-1]::dir = (ns[-2]::subdirs)[ns[-2]::i][1])
		@//perform recursion
		@insert<make/ppp/templates/vs_files.tm>
		@//pop the nested namespace from the list of namespaces
		@define(ns = ns-1)
	</Filter>
	@}
@}
@// collect all files based on rules
@define(collect_recursive=0)
@define(input_directories=[ns[-1]::dir])
@define(ignore_files=[])
@define(ignore_directories=[])
@if(!excludeSourceFiles)@{
	@define(ignore_files=excludeSourceFiles)
@}
@if(!excludeSourceDirs)@{
	@define(ignore_directories=excludeSourceDirs)
@}
@exclude<make/ppp/templates/collect_files.tm>
@// generate filters and source file references
@for(fi=0; fi<(!folder_list); ++fi)@{
	@if(folder_list[fi] != "root")@{
	<Filter Name=@"folder_list[fi]">@}
		@define(L=folder_map[folder_list[fi]])
		@for(i=0; i<(!L); ++i)@{
			<File RelativePath=@"L[i]*'|/|\\|'"></File>@}
	@if(folder_list[fi] != "root")@{
	</Filter>@}@}
