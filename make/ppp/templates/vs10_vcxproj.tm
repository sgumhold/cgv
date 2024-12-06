﻿<?xml version="1.0" encoding="utf-8"?>
@func(::gen_files; :>pj = MAP, :>return="")@{@//
  @skip(sort_files_by_rule_id(<:pj =& pj))
  @define(F =& pj::file_by_rule)
  @for(:>i=0; i<!F; ++i)@{@//
  <ItemGroup>
@for(:>j=0; j<!F[F[i]]; ++j)@{@//
@if(F[F[i]][j] ~~ STRING)@{@//
    @define(fp = F[F[i]][j]*clean_path)@//
    @if(pj::objectFileMap[fp]~~UNDEF)@{@//
    <@(F[i]) Include=@"fp" />
@}@else@{@//
    <@(F[i]) Include=@"fp">
        <ObjectFileName>$(IntDir)\@(pj::objectFileMap[fp]).obj</ObjectFileName>
    </@(F[i])>
@}@}@else@{@//
    <@(F[i]) Include=@"F[F[i]][j][0]*clean_path">
@for(:>k=0; k<!F[F[i]][j][1]; ++k)@{@//
      <ExcludedFromBuild Condition=@('"'."'")$(Configuration)|$(Platform)@("'=='".config_name[F[F[i]][j][1][k]]."|".vs_platform."'".'"')>true</ExcludedFromBuild>
@}
@if(F[F[i]][j][2] > 0)@{
	@for(:>ci=0;ci<6;++ci)@{
		@define(:>cj=ci)
		@if(cj < pj::config_indices && !(cj < F[F[i]][j][1]))@{@//
      <PrecompiledHeader Condition=@('"'."'$(Configuration)|$(Platform)'=='".config_name[cj]."|".vs_platform."'".'"')>@(["Create","NotUsing"][F[F[i]][j][2]-1])</PrecompiledHeader>
@}@}@}
    </@(F[i])>
@}
@}@//
  </ItemGroup>
@}
@}
@func(::gen_project; :>pj=MAP, :>return="")@{
  @define(:>cfg_ord  = [3,1,2,0,4,5])
  @define(:>cfg_ord1 = [3,2,1,0,4,5])@//
@if(cgv_compiler_version > 14)@{@//
<Project DefaultTargets="Build" ToolsVersion="15.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
@}@else@{@//
<Project DefaultTargets="Build" ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
@}
  <ItemGroup Label="ProjectConfigurations">
@for(:>ci=0;ci<6;++ci)@{
    @define(:>cj=cfg_ord[ci])
    @if(cj < pj::config_indices)@{@//
    <ProjectConfiguration Include=@"config_name[cj].'|'.vs_platform">
      <Configuration>@(config_name[cj])</Configuration>
      <Platform>@(vs_platform)</Platform>
    </ProjectConfiguration>
@}
  @}@//
  </ItemGroup>
  <PropertyGroup Label="Globals">
    <ProjectGuid>{@(pj::projectGUID)}</ProjectGuid>
    <RootNamespace>@(pj::projectName)</RootNamespace>
    <Keyword>@(vs_platform)Proj</Keyword>
	@if(cgv_compiler_version > 14)@{@//
    <WindowsTargetPlatformVersion>@(CGV_WINDOWS_SDK)</WindowsTargetPlatformVersion>
	@}
  </PropertyGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.Default.props" />
@for(:>ci=0;ci<6;++ci)@{
    @define(:>cj=cfg_ord1[ci])
    @if(cj < pj::config_indices)@{@//
  <PropertyGroup Condition=@('"'."'")$(Configuration)|$(Platform)@("'=='")@(config_name[cj])|@(vs_platform)@("'".'"') Label="Configuration">
    <ConfigurationType>@(pj::config_type_vs10[cj])</ConfigurationType>
@if(cgv_compiler_version > 10)@{
	@if(cgv_compiler_version > 14)@{@//
    <PlatformToolset>v@(cgv_compiler_version)</PlatformToolset>
	@}@else@{@//
    <PlatformToolset>v@(cgv_compiler_version*10)</PlatformToolset>
	@}
@}
    <CharacterSet>@(pj::charset)</CharacterSet>
@if(cj%4==0)@{@//
    <WholeProgramOptimization>true</WholeProgramOptimization>
@}@//
  </PropertyGroup>
@}
  @}@//
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.props" />
  <ImportGroup Label="ExtensionSettings">
    <Import Project=@"pj::projectName.'.props'" />
@if(pj::cudaEnabled)@{@//
    <Import Project=@('"')$(VCTargetsPath)\BuildCustomizations\CUDA @(pj::cudaVersion).props@('"') />
@}@//
  </ImportGroup>
@for(:>ci=0;ci<6;++ci)@{
    @define(:>cj=cfg_ord1[ci])
    @if(cj < pj::config_indices)@{@//
  <ImportGroup Condition=@('"'."'")$(Configuration)|$(Platform)@("'=='")@(config_name[cj])|@(vs_platform)@("'".'"') Label="PropertySheets">
    <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
@}
  @}@//
  <PropertyGroup Label="UserMacros" />
  <PropertyGroup>
    <_ProjectFileVersion>10.0.30319.1</_ProjectFileVersion>
@for(:>ci=0;ci<6;++ci)@{
    @define(:>cj=ci)
    @if(cj < pj::config_indices)@{@//
    <OutDir Condition=@('"'."'")$(Configuration)|$(Platform)@("'=='")@(config_name[cj])|@(vs_platform)@("'".'"')>@((CGV_INSTALL."/".pj::output_dir[cj]."/")*clean_path)</OutDir>
    <IntDir Condition=@('"'."'")$(Configuration)|$(Platform)@("'=='")@(config_name[cj])|@(vs_platform)@("'".'"')>@((pj::build_dir."/$(ProjectName)_$(Configuration)/")*clean_path)</IntDir>
    <TargetName Condition=@('"'."'")$(Configuration)|$(Platform)@("'=='")@(config_name[cj])|@(vs_platform)@("'".'"')>$(ProjectName)@(pj::output_post[cj])</TargetName>
@if(pj::is_executable[cj] || pj::is_shared[cj])@{@//
    <LinkIncremental Condition=@('"'."'")$(Configuration)|$(Platform)@("'=='")@(config_name[cj])|@(vs_platform)@("'".'"')>@if(pj::is_debug[cj])@{true@}@else@{false@}</LinkIncremental>
@}      
    @}
  @}@//
  </PropertyGroup>
@for(:>ci=0;ci<6;++ci)@{
    @define(:>cj=ci)
    @if(cj < pj::config_indices)@{@//
  <ItemDefinitionGroup Condition=@('"'."'")$(Configuration)|$(Platform)@("'=='")@(config_name[cj])|@(vs_platform)@("'".'"')>
    <ClCompile>
      <AdditionalIncludeDirectories>@(concat(['$(CGV_DIR)','$(CGV_BUILD)'].pj::incDirs[cj%4], ';')*clean_path);%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
@define(:>D = [])
	@define(:>no_inh = 0)
	@for(:>k=0; k<!pj::defines[cj]; ++k)@{	
		@if(pj::defines[cj][k] == "$(NOINHERIT)")@{
			@define(no_inh = 1)
		@}
		@else@{
			@define(D = D.[pj::defines[cj][k]])
		@}
	@}
	@if(no_inh == 0)@{
		@define(D = D.["%(PreprocessorDefinitions)"])
	@}@//
      <PreprocessorDefinitions>@(concat(D,';'))</PreprocessorDefinitions>
      <RuntimeLibrary>@(pj::runtime_lib_vs10[cj])</RuntimeLibrary>
@if(pj::is_debug[cj])@{@//
      <Optimization>Disabled</Optimization>
@}@//
      <DebugInformationFormat>@(pj::debug_info_vs10[cj])</DebugInformationFormat>
@if(pj::is_debug[cj])@{@//
@if(cgv_compiler_version < 142)@{@//
      <MinimalRebuild>true</MinimalRebuild>
@}@//
      <BasicRuntimeChecks>EnableFastChecks</BasicRuntimeChecks>
@}
@if(pj::use_ph)@{@//
      <PrecompiledHeader>Use</PrecompiledHeader>
      <PrecompiledHeaderFile>@(pj::ph_hdr_file_name)</PrecompiledHeaderFile>
@}
@else@{@//
      <PrecompiledHeader></PrecompiledHeader>
@}
      <WarningLevel>Level3</WarningLevel>
@if(pj::useOpenMP)@{@//
      <OpenMPSupport>true</OpenMPSupport>
@}
@if(pj::cppLanguageStandard != "")@{@//
      <LanguageStandard>@(pj::cppLanguageStandard)</LanguageStandard>
      <AdditionalOptions>/Zc:__cplusplus %(AdditionalOptions)</AdditionalOptions>
@}@//
	</ClCompile>
@if(pj::cudaEnabled)@{@//
    <CudaCompile>
      <Include>@(concat(['$(CGV_DIR)','$(CGV_BUILD)'].pj::incDirs[cj%4], ';')*clean_path);%(Include)</Include>
      <Defines>@(concat(["%(Defines)"].D,';'))</Defines>
      <AdditionalOptions>%(AdditionalOptions)@(" ".pj::cudaCodeGenerationOptions)@(" ".pj::cudaOptions)@if(pj::cppLanguageStandard)@{ -std=c++@(pj::cppLanguageStandard+6)@} -Xcompiler="/EHsc -Zi -Ob0 /bigobj"</AdditionalOptions>
      <CodeGeneration>@(pj::cudaCodeGenerationVS)</CodeGeneration>
      <CompileOut>$(IntDir)%(Filename).obj</CompileOut>
      <CudaRuntime>@if(pj::is_static[cj])@{static@}@else@{shared@}</CudaRuntime>
      <GPUDebugInfo>false</GPUDebugInfo>
      <UseHostInclude>false</UseHostInclude>
    </CudaCompile>
@}@//
    <@(pj::linker_tool_vs10[cj])>
      <AdditionalDependencies>@(concat(pj::dependencies[cj%4],';','','.lib')*clean_path);%(AdditionalDependencies)</AdditionalDependencies>
      <AdditionalLibraryDirectories>@(concat(['$(CGV_DIR)/lib','$(CGV_BUILD)/lib'].pj::libDirs[cj%4], ';')*clean_path);%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>
@if(!pj::is_static[cj] && pj::defFile != "")@{@//
      <ModuleDefinitionFile>@(pj::defFile*clean_path)</ModuleDefinitionFile>
@}
@if(pj::is_executable[cj] || pj::is_shared[cj])@{@//
      <GenerateDebugInformation>@if(pj::is_debug[cj])@{true@}@else@{false@}</GenerateDebugInformation>
      <SubSystem>@(pj::sub_system_vs10[cj])</SubSystem>
@if(pj::is_executable[cj])@{@//
      <OutputFile>$(OutDir)$(ProjectName)@(pj::output_post[cj]).exe</OutputFile>
@}
        @else@{@//
      <ImportLibrary>@((CGV_INSTALL.'/lib/$(TargetName).lib')*clean_path)</ImportLibrary>
      <ProgramDatabaseFile>$(IntDir)$(TargetName).pdb</ProgramDatabaseFile>
@}
      @}@//
    </@(pj::linker_tool_vs10[cj])>
@if(pj::is_executable[cj])@{@//
    <ProjectReference>
      <UseLibraryDependencyInputs>true</UseLibraryDependencyInputs>
    </ProjectReference>
@}@//
@if(pj::cudaEnabled)@{@//
    <CudaLink>
      <AdditionalOptions>-forward-unknown-to-host-compiler -Wno-deprecated-gpu-targets </AdditionalOptions>
      <PerformDeviceLink>false</PerformDeviceLink>
    </CudaLink>
@}@//
  </ItemDefinitionGroup>
@}
  @}
  @skip(gen_files(<:pj =& pj, <:T =& pj::sourceTree))
  @if(!pj::projectRefs)@{@//
  <ItemGroup>
@for(:>i=0; i<!pj::projectRefs; ++i)@{
      @define(:>dep_pn = pj::projectRefs[i])@//
    <ProjectReference Include=@"'..\\'.dep_pn.'\\'.dep_pn.'.vcxproj'">
      <Project>{@(projects[pj::projectRefs[i]]::projectGUID)}</Project>
      <ReferenceOutputAssembly>false</ReferenceOutputAssembly>
    </ProjectReference>
@}@//
  </ItemGroup>
@}@//
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.targets" />
  <ImportGroup Label="ExtensionTargets">
    <Import Project=@"pj::projectName.'.targets'" />
@if(pj::cudaEnabled)@{@//
    <Import Project=@('"')$(VCTargetsPath)\BuildCustomizations\CUDA @(pj::cudaVersion).targets@('"') />
@}@//
  </ImportGroup>
</Project>@//
@}
@skip(gen_project(<:pj =& projects[current_project]))@//