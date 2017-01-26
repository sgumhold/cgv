@func(::gen_files; :>pj = MAP, :>T = MAP, :>filter = "", :>return="")@{@//
  @if(T ~~ MAP)@{
    @for(:>i=0; i<!T; ++i)@{
	  @if(T[i] != "excl_cfg_idxs")@{
		@skip(gen_files(<:pj =& pj, <:T =& T[T[i]], <:filter=filter.T[i].'\\'))
      @}
    @}
  @}
  @else@{
@for(:>i=0; i<!T; ++i)@{
		@define(:>fn = get_src_file_name(<:T =& T, <:i = i))
		@if(fn)@{
			@define(rule_id = get_rule_id_vs10(<:pj =& pj, <:ext = get_extension(fn)))@//
			@if(rule_id=="shader_rule")@{
@define(file = drop_path(<:path = fn*clean_path))
#include <@(file).log>
cgv::base::resource_string_registration @(file*"|.|_")_reg(@"file",@(file*"|.|_"));
@}@}@}@//
@}
@}
@skip(gen_files(<:pj =& projects[project_name], <:T =& projects[project_name]::sourceTree))
