<?xml version="1.0" encoding="utf-8"?>
@func(::gen_user_config; :>ci=0, :>project_name="", :>return="")@{
  @define(:>debug_flavor=["","",0,0])
  @define(:>pj =& projects[project_name])
  @if(pj::is_shared[ci] && (pj::application_name !~ UNDEF))@{@//
  <PropertyGroup Condition=@('"'."'")$(Configuration)|$(Platform)@("'=='")@(config_name[ci])|Win32@("'".'"')>
    <LocalDebuggerCommand>@(CGV_INSTALL.'\\bin\\'.pj::application_name.pj::output_post[ci].'.exe')</LocalDebuggerCommand>
  </PropertyGroup>
@}
  @define(command_line_args=get_command_line_args(<:pj =& pj, <:ci = ci)*'|"|&quot;|')
  @if((command_line_args != "") || (pj::workingDirectory != "") )@{@//
  <PropertyGroup Condition=@('"'."'")$(Configuration)|$(Platform)@("'=='")@(config_name[ci])|Win32@("'".'"')>
@if(command_line_args != "")@{@//
    <LocalDebuggerCommandArguments>@(command_line_args)</LocalDebuggerCommandArguments>
    <DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor>
@}
  @if(pj::workingDirectory != "")@{@//
    <LocalDebuggerWorkingDirectory>@(pj::workingDirectory)</LocalDebuggerWorkingDirectory>
@}@//
  </PropertyGroup>
@}
@}
@func(::gen_user; :>project_name="", :>return="")@{@//
<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
@define(pj =& projects[project_name])
  @define(:>cfg_ord = [3,2,1,0,4,5])
  @for(:>ci=0;ci<6;++ci)@{
	@define(:>cj = cfg_ord[ci])
    @if(cj<pj::config_indices)@{
	    @skip(gen_user_config(cj,project_name))
    @}
  @}@//
</Project>@//
@}
@skip(gen_user(current_project))