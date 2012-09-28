<?xml version="1.0" encoding="UTF-8"?>
@exclude<make/ppp/templates/eclipse_support.tm>
@exclude<make/ppp/templates/check_input.tm>
@exclude<make/ppp/templates/interpret_input.tm>
@exclude<make/ppp/templates/dep_prj.tm>
@exclude<make/ppp/templates/extend_input_workspace.tm>
@exclude<make/ppp/templates/extend_dep_projects.tm>
@exclude<make/ppp/templates/dep_prj.tm>
@func(add_source_files_to_filters; :>prj_path = "", :>src_paths = [], :>src_files = [], :>return=0)
@{
	@//cout("enter add_source_files_to_filters with prj_path = ".prj_path." with di = "._di."\n")
	@define(input_directories=src_paths)
	@define(input_files=src_files)
	@define(collect_recursive=0)
	@define(ignore_files=[])
	@define(ignore_directories=[])
	@if(!excludeSourceFiles)@{
		@define(ignore_files=excludeSourceFiles)@}
	@if(!excludeSourceDirs)@{
		@define(ignore_directories=excludeSourceDirs)@}
	@exclude<make/ppp/templates/collect_files.tm>
	@// generate filters and source file references
	@for(:>fi=0; fi < !folder_list; ++fi)@{
		@define(prj_folder=prj_path)
		@if(folder_list[fi] != "root")@{
			@define(prj_folder=prj_folder."/".folder_list[fi])
			@define(ensure_dir(path = build_dir."/".projectName."/".prj_folder, help = "eclipse workspace folder"))@}
		@define(L=folder_map[folder_list[fi]])
		@define(return = return + !L)
		@for(:>i=0; i < !L; ++i)@{
		<link>
			<name>@(prj_folder."/".drop_path(path = L[i]))</name>
			<type>1</type>
			<location>@(L[i]*"|\\|/|")</location>
		</link>
		@}@}@}
@func(add_source_files; :>src_path = "", :>prj_path = "", :>return = 0) @{
	@//cout("enter add_source_files with prj_path = ".prj_path." with di = "._di."\n")
	@// ensure that the directory in the build path exists
	@define(ensure_dir(path = build_dir."/".projectName."/".prj_path, help = "eclipse workspace"))
	@// perform recursion on subdirs first
	@define(:>subdirs=[])
	@dir(src_path; "."; 0; subdirs)
	@for (:>i = 0; i < !subdirs; ++i) @{
		@if(!(subdirs[i][1]*clean_path < excludeSourceDirs))@{
			@define(add_source_files(src_path = subdirs[i][1], prj_path = <:prj_path."/".subdirs[i][0]));@}@}
	@// collect all files based on rules
	@define(return = return + add_source_files_to_filters(prj_path = <:prj_path, src_paths = [<:src_path]))@}
@func(add_source_dir; :>src_path = "") @{
	@//cout("enter add_source_dir with src_path = ".src_path." with di = "._di."\n")
	@if(:>prj_path ~~ UNDEF)@{
		@define(:>prj_path = drop_path(path = src_path))@}
	@define(add_source_files(src_path = <:src_path, prj_path = <:prj_path))
@}
<projectDescription>
	<name>@(projectName)</name>
	<comment></comment>
	<projects>
	</projects>
	<buildSpec>
		<buildCommand>
			<name>org.eclipse.cdt.managedbuilder.core.genmakebuilder</name>
			<triggers>clean,full,incremental,</triggers>
			<arguments>
				<dictionary>
					<key>?name?</key>
					<value></value>
				</dictionary>
				<dictionary>
					<key>org.eclipse.cdt.make.core.append_environment</key>
					<value>true</value>
				</dictionary>
				<dictionary>
					<key>org.eclipse.cdt.make.core.autoBuildTarget</key>
					<value>all</value>
				</dictionary>
				<dictionary>
					<key>org.eclipse.cdt.make.core.buildArguments</key>
					<value></value>
				</dictionary>
				<dictionary>
					<key>org.eclipse.cdt.make.core.buildCommand</key>
					<value>make</value>
				</dictionary>
				<dictionary>
					<key>org.eclipse.cdt.make.core.cleanBuildTarget</key>
					<value>clean</value>
				</dictionary>
				<dictionary>
					<key>org.eclipse.cdt.make.core.contents</key>
					<value>org.eclipse.cdt.make.core.activeConfigSettings</value>
				</dictionary>
				<dictionary>
					<key>org.eclipse.cdt.make.core.enableAutoBuild</key>
					<value>false</value>
				</dictionary>
				<dictionary>
					<key>org.eclipse.cdt.make.core.enableCleanBuild</key>
					<value>true</value>
				</dictionary>
				<dictionary>
					<key>org.eclipse.cdt.make.core.enableFullBuild</key>
					<value>true</value>
				</dictionary>
				<dictionary>
					<key>org.eclipse.cdt.make.core.fullBuildTarget</key>
					<value>all</value>
				</dictionary>
				<dictionary>
					<key>org.eclipse.cdt.make.core.stopOnError</key>
					<value>true</value>
				</dictionary>
				<dictionary>
					<key>org.eclipse.cdt.make.core.useDefaultBuildCmd</key>
					<value>true</value>
				</dictionary>
			</arguments>
		</buildCommand>
		<buildCommand>
			<name>org.eclipse.cdt.managedbuilder.core.ScannerConfigBuilder</name>
			<arguments>
			</arguments>
		</buildCommand>
	</buildSpec>
	<natures>
		<nature>org.eclipse.cdt.core.cnature</nature>
		<nature>org.eclipse.cdt.core.ccnature</nature>
		<nature>org.eclipse.cdt.managedbuilder.core.managedBuildNature</nature>
		<nature>org.eclipse.cdt.managedbuilder.core.ScannerConfigNature</nature>
	</natures>
	<linkedResources>
@//go through all source directories
@for(_di = 0; _di < (!sourceDirs);++_di) @{
	@define(add_source_dir(src_path = sourceDirs[_di]))
@}
@// define a separate filter for source files in the sourceFiles define
@if(!sourceFiles > 0)@{
	@define(add_source_files_to_filters(prj_path = "further", src_files = sourceFiles))
@}
	</linkedResources>
</projectDescription>
