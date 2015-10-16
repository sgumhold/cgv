<?xml version="1.0" encoding="utf-8"?>
@func(::gen_targets; :>pj = MAP, :>return="")@{@//
<Project xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup>
@if(CGV_COMPILER == "vs10" || CGV_COMPILER == "vs11")@{@//
    <PropertyPageSchema
      Include="$(MSBuildThisFileDirectory)$(MSBuildThisFileName).xml" />
@}
@for(:>i=0; i<!pj::rules; ++i)@{
     @if(is_rule(<:R =& pj::rules[i]))@{@//
    <AvailableItemName
      Include=@"get_rule_name(<:R =& pj::rules[i])">
      <Targets>_@(get_rule_name(<:R =& pj::rules[i]))</Targets>
    </AvailableItemName>
@}
  @}@//
  </ItemGroup>
@for(:>i=0; i<!pj::rules; ++i)@{
     @if(is_rule(<:R =& pj::rules[i]))@{@//
  <UsingTask
    TaskName=@"get_rule_name(<:R =& pj::rules[i])"
    TaskFactory="XamlTaskFactory"
    AssemblyName="Microsoft.Build.Tasks.v4.0">
    <Task>$(MSBuildThisFileDirectory)$(MSBuildThisFileName).xml</Task>
  </UsingTask>
@}
  @}
  @for(:>i=0; i<!pj::rules; ++i)@{
     @if(is_rule(<:R =& pj::rules[i]))@{
       @define(:>rid = get_rule_name(<:R =& pj::rules[i]))@//
  <Target
    Name=@"'_'.rid"
    BeforeTargets=@"'$('.rid.'BeforeTargets)'"
    AfterTargets=@"'$('.rid.'AfterTargets)'"
    Condition=@('"'."'")@@(@(rid))@("' != ''".'"')
    DependsOnTargets=@('"')$(@(rid)DependsOn);Compute@(rid)Output@('"')
    Outputs=@"'%('.rid.'.Outputs)'"
    Inputs=@"'%('.rid.'.Identity);%('.rid.'.AdditionalDependencies);$(MSBuildProjectFile)'">
    <ItemGroup
      Condition="'@(SelectedFiles)' != ''">
      <@(rid)
        Remove=@"'@('.rid.')'"
        Condition="'%(Identity)' != '@(SelectedFiles)'" />
    </ItemGroup>
    <ItemGroup>
      <@(rid)_tlog
        Include=@"'%('.rid.'.Outputs)'"
        Condition=@('"'."'")%(@(rid).Outputs)@("'") != '' and @("'")%(@(rid).ExcludedFromBuild)@("'") != @("'true'".'"')>
        <Source>@@(@(rid), '|')</Source>
      </@(rid)_tlog>
    </ItemGroup>
    <Message
      Importance="High"
      Text=@"'%('.rid.'.ExecutionDescription)'" />
    <WriteLinesToFile
      Condition=@('"'."'")@@(@(rid)_tlog)@("' != '' and '")%(@(rid)_tlog.ExcludedFromBuild)@("' != 'true'".'"')
      File="$(IntDir)$(ProjectName).write.1.tlog"
      Lines=@('"')^%(@(rid)_tlog.Source);@@(@(rid)_tlog-&gt;'%(Fullpath)')@('"') />
    <@(rid)
      Condition=@('"'."'")@@(@(rid))@("' != '' and '")%(@(rid).ExcludedFromBuild)@("' != 'true'".'"')
      CommandLineTemplate=@('"')%(@(rid).CommandLineTemplate)@('"')
      AdditionalOptions=@('"')%(@(rid).AdditionalOptions)@('"')
      Inputs=@('"')%(@(rid).Identity)@('"') />
  </Target>
  <PropertyGroup>
    <ComputeLinkInputsTargets>
            $(ComputeLinkInputsTargets);
            Compute@(rid)Output;
          </ComputeLinkInputsTargets>
    <ComputeLibInputsTargets>
            $(ComputeLibInputsTargets);
            Compute@(rid)Output;
          </ComputeLibInputsTargets>
  </PropertyGroup>
  <Target
    Name=@('"')Compute@(rid)Output@('"')
    Condition=@('"'."'")@@(@(rid))@("' != ''".'"')>
    <ItemGroup>
      <@(rid)DirsToMake
        Condition=@('"'."'")@@(@(rid))@("' != '' and '")%(@(rid).ExcludedFromBuild)@("' != 'true'".'"')
        Include=@('"')%(@(rid).Outputs)@('"') />
      <Link
        Include=@('"')%(@(rid)DirsToMake.Identity)@('"')
        Condition="'%(Extension)'=='.obj' or '%(Extension)'=='.res' or '%(Extension)'=='.rsc' or '%(Extension)'=='.lib'" />
      <Lib
        Include=@('"')%(@(rid)DirsToMake.Identity)@('"')
        Condition="'%(Extension)'=='.obj' or '%(Extension)'=='.res' or '%(Extension)'=='.rsc' or '%(Extension)'=='.lib'" />
      <ImpLib
        Include=@('"')%(@(rid)DirsToMake.Identity)@('"')
        Condition="'%(Extension)'=='.obj' or '%(Extension)'=='.res' or '%(Extension)'=='.rsc' or '%(Extension)'=='.lib'" />
    </ItemGroup>
    <MakeDir
      Directories=@('"')@@(@(rid)DirsToMake-&gt;'%(RootDir)%(Directory)')@('"') />
  </Target>
@}
  @}@//
</Project>@//
@}
@skip(gen_targets(<:pj =& projects[current_project]))