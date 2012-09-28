<?xml version="1.0" encoding="utf-8"?>
<VisualStudioToolFile
	Name=@"current_project.'_rules'"
	Version="8,00"
	>
@func(::gen_rule; :>R=MAP, :>return="")@{
    <CustomBuildRule
			Name=@"get_rule_name(<:R =& R)"
			DisplayName=@"get_rule_name(<:R =& R)"
			CommandLine=@"rule_command_line(<:R =& R, <:sep=' ', <:prefix='&quot;', <:postfix='&quot;')"
        Outputs=@"rule_targets(<:R =& R, <:sep=';', <:prefix='&quot;', <:postfix='&quot;')"
        FileExtensions=@"concat(R::extensions,';','*.')"
        ExecutionDescription=@"'apply '.get_rule_name(<:R =& R)"
        >
        <Properties>
			</Properties>
		</CustomBuildRule>
@}
@func(::gen_rules; :>Rs=LIST, :>return="")@{
	<Rules>
@define(rule_variable_map = rule_variable_map_vs)
@for(:>i=0; i<!Rs; ++i)@{
    @if(is_rule(<:R =& Rs[i])) @{
        @skip(gen_rule(<:R =& Rs[i]))
    @}
@}
	</Rules>
@}
@skip(gen_rules(<:Rs =& projects[current_project]::rules))
</VisualStudioToolFile>
