<?xml version="1.0" encoding="utf-8"?>
@func(::gen_rule; :>pj = MAP, :>R = MAP, :>return="")@{@//
  <PropertyGroup
    Condition=@('"'."'")$(@(get_rule_name(<:R =& R))BeforeTargets)@("' == '' and '")$(@(get_rule_name(<:R =& R))AfterTargets)@("' == '' and '")$(ConfigurationType)@("' != 'Makefile'".'"')>
    <@(get_rule_name(<:R =& R))BeforeTargets>Midl</@(get_rule_name(<:R =& R))BeforeTargets>
    <@(get_rule_name(<:R =& R))AfterTargets>CustomBuild</@(get_rule_name(<:R =& R))AfterTargets>
  </PropertyGroup>
  <PropertyGroup>
    <@(get_rule_name(<:R =& R))DependsOn
      Condition="'$(ConfigurationType)' != 'Makefile'">_SelectedFiles;$(@(get_rule_name(<:R =& R))DependsOn)</@(get_rule_name(<:R =& R))DependsOn>
  </PropertyGroup>
  <ItemDefinitionGroup>
    <@(get_rule_name(<:R =& R))>
      <CommandLineTemplate>@(rule_command_line(<:R =& R, <:sep=' ', <:prefix='&quot;', <:postfix='&quot;'))</CommandLineTemplate>
      <Outputs>@(rule_targets(<:R =& R, <:sep=';', <:prefix='', <:postfix=''))</Outputs>
      <ExecutionDescription>apply @(get_rule_name(<:R =& R))</ExecutionDescription>
    </@(get_rule_name(<:R =& R))>
  </ItemDefinitionGroup>
@}
@func(::gen_props; :>pj = MAP, :>return="")@{@//
<Project xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
@define(rule_variable_map = rule_variable_map_vs10)
  @for(:>i=0; i<!pj::rules; ++i)@{
     @if(is_rule(<:R =& pj::rules[i]))@{
       @skip(gen_rule(<:pj =& pj, <:R =& pj::rules[i]))
     @}
  @}@//
</Project>@//
@}
@skip(gen_props(<:pj =& projects[current_project]))