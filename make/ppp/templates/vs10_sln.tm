Microsoft Visual Studio Solution File, Format Version 11.00
# Visual Studio 2010
@func(::gen_project; :>projectName="", :>add_rel_path=1, :>return="")@{
	@define(:>pj =& projects[projectName])
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = @"projectName", @('"')@if(add_rel_path)@{@('..\\'.projectName.'\\')@}@(projectName.'.vcxproj"'), @"'{'.pj::projectGUID.'}'"
EndProject@}
@func(::gen_solution; :>projectName="", :>return="")@{
	@define(:>pj =& projects[projectName])
	@define(:>cfg_ord  = [3,1,2,0])
	@for(:>i=0; i<!project_folders; ++i)@{
		@if(pj::all_ref_projects_by_type[project_folders[i]] !~ UNDEF)@{
			@if((!pj::all_ref_projects_by_type[project_folders[i]]) > 0) @{@//
Project("{2150E333-8FDC-42A3-9474-1A3956D46DE8}") = @"project_folders[i]", @"project_folders[i]", @"'{5A46AE2A-975D-4570-BE19-A5B7B8C13E2'.i.'}'"
EndProject
@}@}@}@//
	@skip(gen_project(projectName, 0))
	@if(pj::all_ref_projects)@{
		@for(:>i=0;i<!pj::all_ref_projects;++i)@{@skip(gen_project(pj::all_ref_projects[i],1))@}
	@}
Global
	GlobalSection(SolutionConfigurationPlatforms) = preSolution
@for(:>ci=0;ci<4;++ci)@{@define(:>cj=cfg_ord[ci])@if(cj<pj::config_indices)@{@//
		@(config_name[cj])|Win32 = @(config_name[cj])|Win32
@}@}@//
	EndGlobalSection
	GlobalSection(ProjectConfigurationPlatforms) = postSolution
@for(:>ci=0;ci<4;++ci)@{@define(:>cj=cfg_ord[ci])@if(cj<pj::config_indices)@{@//
		{@(pj::projectGUID)}.@(config_name[cj])|Win32.ActiveCfg = @(config_name[map_cfg_idx_self(cj,pj::config_indices)])|Win32
		{@(pj::projectGUID)}.@(config_name[cj])|Win32.Build.0 = @(config_name[map_cfg_idx_self(cj,pj::config_indices)])|Win32
@}@}
@if(!pj::all_ref_projects > 0)@{
	@for(:>i=0;i<!pj::all_ref_projects;++i)@{		
		@define(:>pkGUID = projects[pj::all_ref_projects[i]]::projectGUID)
		@define(:>pkConfig_indices = projects[pj::all_ref_projects[i]]::config_indices)
		@for(:>ci=0;ci<4;++ci)@{
			@define(:>cj=cfg_ord[ci])
			@if(cj<pj::config_indices)@{
				@define(:>ck=map_cfg_idx(cj,pkConfig_indices))@//
		{@(pkGUID)}.@(config_name[cj])|Win32.ActiveCfg = @(config_name[ck])|Win32
		{@(pkGUID)}.@(config_name[cj])|Win32.Build.0 = @(config_name[ck])|Win32
@}@}@}@}@//
	EndGlobalSection
	GlobalSection(SolutionProperties) = preSolution
		HideSolutionNode = FALSE
	EndGlobalSection
@if(!pj::all_ref_projects > 0)@{
	GlobalSection(NestedProjects) = preSolution
@for(:>i=0; i<!project_folders; ++i)@{@//
	@if(pj::all_ref_projects_by_type)@{@//
		@if(pj::all_ref_projects_by_type[project_folders[i]] !~ UNDEF)@{@//
			@if((!pj::all_ref_projects_by_type[project_folders[i]]) > 0) @{@//
				@for(:>j=0; j<!pj::all_ref_projects_by_type[project_folders[i]]; ++j)@{@//
		{@(projects[pj::all_ref_projects_by_type[project_folders[i]][j]]::projectGUID)} = {5A46AE2A-975D-4570-BE19-A5B7B8C13E2@(i)}
@}@}@}@}@}@//
	EndGlobalSection
@}@//
EndGlobal@}
@skip(gen_solution(current_project))
