<?xml version="1.0" encoding="Windows-1252"?>
@func(::gen_user_config; :>ci=0, :>project_name="", :>return="")@{
	@define(:>debug_flavor=["","",0,0])
	@define(:>pj =& projects[project_name])	
		<Configuration
			Name=@"config_name[ci].'|Win32'"
			>
			<DebugSettings@if(pj::is_shared[ci] && (pj::application_name !~ UNDEF))@{
				Command=@"CGV_INSTALL.'\\'.(pj::output_dir[ci]*clean_path).'\\'.pj::application_name.pj::output_post[ci].'.exe'"
@}@else@{
				Command="$(TargetPath)"@}@if(pj::workingDirectory)@{
				WorkingDirectory=@"pj::workingDirectory"
@}@else@{
				WorkingDirectory=""@}
				CommandArguments=@"get_command_line_args(<:pj =& pj, <:ci = ci)*'|"|&quot;|'"
				Attach="false"
				DebuggerType="3"
				Remote="1"
				RemoteMachine="ENCHANTRESS"
				RemoteCommand=""
				HttpUrl=""
				PDBPath=""
				SQLDebugging=""
				Environment=""
				EnvironmentMerge="true"
				DebuggerFlavor=@"debug_flavor[ci%4]"
				MPIRunCommand=""
				MPIRunArguments=""
				MPIRunWorkingDirectory=""
				ApplicationCommand=""
				ApplicationArguments=""
				ShimCommand=""
				MPIAcceptMode=""
				MPIAcceptFilter=""
			/>
		</Configuration>
@}
@func(::gen_user; :>project_name="", :>return="")@{
@define(:>pj=projects[project_name])
<VisualStudioUserFile
	ProjectType="Visual C++"
	Version=@"cgv_compiler_version.',00'"
	ShowAllFiles="false"
	>
	<Configurations>
@for(:>ci=0;ci<6;++ci)@{@if(ci<pj::config_indices)@{
	@skip(gen_user_config(ci,project_name))@}@}
	</Configurations>
</VisualStudioUserFile>
@}
@skip(gen_user(current_project))
