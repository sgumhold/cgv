<?xml version="1.0" encoding="Windows-1252"?>
@func(::gen_config; :>ci=0, :>project_name="", :>return="")@{
	@define(:>pj =& projects[project_name])
	@define(:>is_shared=pj::is_shared[ci])
	@define(:>is_debug=(is_dbg[ci] == "true"))@//
    <Configuration
			Name=@"config_name[ci].'|Win32'"
			OutputDirectory=@"CGV_INSTALL.'/'.pj::output_dir[ci]"
			IntermediateDirectory=@"pj::build_dir.'/$(ProjectName)_$(ConfigurationName)'"
			ConfigurationType=@"pj::config_type[ci]"
			CharacterSet="1"
@if(!is_debug)@{@//
			WholeProgramOptimization="1"
@}@//
			>
      <Tool
				Name="VCPreBuildEventTool"
			/>
      <Tool
				Name="doxygen"
			/>
      <Tool
				Name="latex"
			/>
      <Tool
				Name="VCCustomBuildTool"
			/>
      <Tool
				Name="reflect header"
			/>
      <Tool
				Name="pre header rule"
			/>
      <Tool
				Name="VCXMLDataGeneratorTool"
			/>
      <Tool
				Name="VCWebServiceProxyGeneratorTool"
			/>
      <Tool
				Name="VCMIDLTool"
			/>
      <Tool
				Name="VCCLCompilerTool"
				AdditionalIncludeDirectories=@"concat('$(CGV_DIR)'.pj::incDirs[ci%4],';')"
				PreprocessorDefinitions=@"concat(pj::defines[ci],';')"
				RuntimeLibrary=@"ci%4"
@if(is_debug)@{
				Optimization="0"
				DebugInformationFormat="4"
				MinimalRebuild="true"
				BasicRuntimeChecks="3"
@}
@else@{
				DebugInformationFormat="3"
@}
@if(pj::use_ph)@{@//
				UsePrecompiledHeader="2"
				PrecompiledHeaderThrough=@"pj::ph_hdr_file_name"
@}
@else@{@//
				UsePrecompiledHeader="0"
@}@//
				WarningLevel="3"
@if(cgv_compiler_version==8)@{@//
				Detect64BitPortabilityProblems="true"
@}@//
			/>
      <Tool
				Name="VCManagedResourceCompilerTool"
			/>
      <Tool
				Name="VCResourceCompilerTool"
			/>
      <Tool
				Name="VCPreLinkEventTool"
			/>
      <Tool
				Name=@"pj::linker_tool[ci]"
@if(pj::dependencies)@{@if(pj::dependencies[ci%4])@{
				AdditionalDependencies=@"concat(pj::dependencies[ci%4],' ','','.lib')"
@}@}
@if(use_lib_deps[ci])@{
				UseLibraryDependencyInputs="true"
@}
				AdditionalLibraryDirectories=@"concat(['$(CGV_DIR)/lib','$(CGV_BUILD)/lib'].pj::libDirs[ci%4], ';')"
@if(!is_debug && is_shared && (pj::installPath ~~ STRING))@{
				OutputFile=@"pj::installPath.'/$(ProjectName)'.'.'.pj::output_ext[ci]"
@}
@else@{
				OutputFile=@"'$(OutDir)/$(ProjectName)'.pj::output_post[ci].'.'.pj::output_ext[ci]"
@}
@if(pj::linker_tool[ci]=="VCLinkerTool")@{
				LinkIncremental=@"link_inc[ci]"
@if(pj::defFile ~~ STRING)@{@if((!pj::defFile)>0)@{
				ModuleDefinitionFile=@"pj::defFile"
@}@}
				GenerateDebugInformation=@"is_dbg[ci]"
				SubSystem=@"pj::sub_system[ci]"
@if(!pj::is_executable[ci])@{
				ImportLibrary=@"CGV_INSTALL.'/lib/$(TargetName).lib'"
				ProgramDatabaseFile=@"CGV_INSTALL.'/lib/$(TargetName).pdb'"
@}
@}
			/>
      <Tool
				Name="VCALinkTool"
			/>
      <Tool
				Name="VCManifestTool"
			/>
      <Tool
				Name="VCXDCMakeTool"
			/>
      <Tool
				Name="VCBscMakeTool"
			/>
      <Tool
				Name="VCFxCopTool"
			/>
      <Tool
				Name="VCAppVerifierTool"
			/>
      <Tool
				Name="VCWebDeploymentTool"
			/>
      <Tool
				Name="VCPostBuildEventTool"
@if(pj::projectType == "test" && !is_debug)@{
				Description="performing tests"
				CommandLine=@"CGV_INSTALL.'\\'.(pj::output_dir[ci]*clean_path).'\\tester'.pj::output_post[ci].'.exe '.get_command_line_args(<:pj =& pj, <:ci = ci)*'|"|&quot;|'"
@}
			/>
    </Configuration>
@}
@func(::gen_files; :>T = MAP, :>tab = "", :>excl_cfg_idxs=[], :>return="")@{
	@skip(update_excl_cfg_idxs(<:T =& T, <:excl_cfg_idxs =& excl_cfg_idxs))
	@for(:>i=0; i<!T; ++i)@{
		@define(:>fn = get_folder_name(<:T =& T, <:i=i, <:excl_cfg_idxs =& excl_cfg_idxs))
		@if(fn)@{
@(tab)		<Filter Name=@"fn">
@skip(gen_files(<:T =& T[fn], <:tab=tab.'\t', <:excl_cfg_idxs = excl_cfg_idxs))
@(tab)		</Filter>
        @}
		@define(:>excl_idxs = excl_cfg_idxs)
		@define(:>gen_ph = 0)
		@define(fn = get_src_file_name(<:T =& T, <:i = i, <:excl_cfg_idxs =& excl_cfg_idxs, <:excl_idxs =& excl_idxs, <:gen_ph =& gen_ph))
		@if(fn)@{
			@if(!excl_idxs == 0 && !gen_ph)@{@//
@(tab)		<File RelativePath=@"fn*clean_path"></File>
@}
	        @else@{@//
@(tab)		<File RelativePath=@"fn*clean_path">
@if(!excl_idxs > 0)@{
@for(:>ci=0;ci<6;++ci)@{
	@if(ci<excl_idxs)@{@//
			<FileConfiguration Name=@"config_name[ci].'|Win32'" ExcludedFromBuild="true" />
@}
@}
@}
@if(gen_ph)@{
	@for(:>ci=0;ci<6;++ci)@{
		@if(!(ci<excl_idxs))@{@//
					<FileConfiguration Name=@"config_name[ci].'|Win32'">
						<Tool Name="VCCLCompilerTool" UsePrecompiledHeader=@"2-gen_ph" />
					</FileConfiguration>
@}
@}
@}
@(tab)      </File>
			@}
		@}
	@}
@}
@func(::gen_project; :>project_name="", :>return="")@{@//
@define(:>pj =& projects[project_name])
<VisualStudioProject
	ProjectType="Visual C++"
	Version=@"cgv_compiler_version.',00'"
	Name=@"project_name"
	ProjectGUID=@"'{'.pj::projectGUID.'}'"
	RootNamespace=@"project_name"
	Keyword="Win32Proj"
	>
  <Platforms>
    <Platform
			Name="Win32"
		/>
	</Platforms>
	<ToolFiles>
		<ToolFile RelativePath=@"(pj::build_dir.'/'.project_name.'.rules')*clean_path" />@if(pj::rulesFiles)@{@for(:>ri=0;ri<!pj::ruleFiles;++ri)@{
		<ToolFile RelativePath=@"pj::ruleFiles[ri]" />@}@}
	</ToolFiles>
	<Configurations>
@for(:>ci=0;ci<6;++ci)@{@if(ci<pj::config_indices)@{@//
	@skip(gen_config(ci,project_name))@}@}
	</Configurations>
	<References>
	</References>
	<Files>
	@skip(gen_files(<:T =& pj::sourceTree, <:tab = '\t'))
	</Files>
	<Globals>
	</Globals>
</VisualStudioProject>
@}@//
@skip(gen_project(current_project))