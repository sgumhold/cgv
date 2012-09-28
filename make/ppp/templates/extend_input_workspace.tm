/* extend_input_workspace is non recursive function 

	Inputs:

	Outputs:
    - addRuleFiles:LIST of STRING ... list of all rule files to be added to the
	                         project, including exported ones. All extended syntax
							 entries are removed
    - rules:LIST of MAP ... list of all rules which is a concatenation of the
	                         standard rules defined in <cgv/config/make.ppp>, the
							 rules in addRules and the rules exported from other
							 projects in the the addRules define.
*/

@//cout(cfg_deps.[])@cout("\n")
@for(i=0;i<!project_map;++i)@{
	@if(project_map[project_map[i]]::addRuleFiles !~ UNDEF)@{
		@define(prjRules = project_map[project_map[i]]::addRuleFiles)
		@for(j=0;j<!prjRules;++j)@{
			@define(lrule=prjRules[j])
			@if(lrule ~~ LIST)@{
				@define(addRuleFiles=addRuleFiles.[lrule[0]])
			@}
		@}
	@}
	@if(project_map[project_map[i]]::addRules !~ UNDEF)@{
		@define(prjRules = project_map[project_map[i]]::addRules)
		@for(j=0;j<!prjRules;++j)@{
			@define(lrule=prjRules[j])
			@if(lrule ~~ LIST)@{
				@define(addRules=addRules.[lrule[0]])
			@}
		@}
	@}
@}

@//clean up to be added rule files from list syntax
@if(addRuleFiles)@{
	@for(i=0;i<!addRuleFiles;++i)@{
		@if(addRuleFiles[i] ~~ LIST)@{
			@define(ruleFile = addRuleFiles[i][0])
			@define(addRuleFiles[i] = ruleFile)
		@}
	@}
@}
@//concatenate all rules
@define(prj_rules=rules)
@if(addRules)@{
	@for(i=0;i<!addRules;++i)@{
		@if(addRules[i] ~~ LIST)@{
			@define(prj_rules=prj_rules.[addRules[i][0]])
		@}
		@else@{
			@define(prj_rules=prj_rules.[addRules[i]])
		@}
	@}
@}
@define(rules=prj_rules)
