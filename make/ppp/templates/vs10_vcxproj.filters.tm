<?xml version="1.0" encoding="utf-8"?>
@func(::gen_filters; :>T = MAP, :>prefix = "", :>return="")@{
  @for(:>i=0; i<!T; ++i)@{
	@if(T[i] != "excl_cfg_idxs")@{@//
    <Filter Include=@"prefix.T[i]">
    </Filter>
@if(T[T[i]] ~~ MAP)@{
			@skip(gen_filters(<:T =& T[T[i]], <:prefix=prefix.T[i].'\\'))
		@}
	@}
  @}
@}
@func(::gen_files; :>pj = MAP, :>T = MAP, :>filter = "", :>return="")@{@//
  @if(T ~~ MAP)@{
    @for(:>i=0; i<!T; ++i)@{
	  @if(T[i] != "excl_cfg_idxs")@{
		@skip(gen_files(<:pj =& pj, <:T =& T[T[i]], <:filter=filter.T[i].'\\'))
      @}
    @}
  @}
  @else@{
  <ItemGroup>
@for(:>i=0; i<!T; ++i)@{
		@define(:>fn = get_src_file_name(<:T =& T, <:i = i))
		@if(fn)@{
			@define(rule_id = get_rule_id_vs10(<:pj =& pj, <:ext = get_extension(fn)))@//
    <@(rule_id) Include=@"fn*clean_path">
       <Filter>@(filter-1)</Filter>
    </@(rule_id)>
@}@}@//
  </ItemGroup>
@}
@}
@func(::gen_filter_file; :>project_name="", :>return="")@{@//
<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup>
@skip(gen_filters(<:T =& projects[project_name]::sourceTree))@//
  </ItemGroup>
@skip(gen_files(<:pj =& projects[project_name], <:T =& projects[project_name]::sourceTree))@//
</Project>@//
@}
@skip(gen_filter_file(current_project))