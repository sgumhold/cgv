<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<?fileVersion 4.0.0?>
@exclude<make/ppp/templates/check_input.tm>
@exclude<make/ppp/templates/interpret_input.tm>
@define(config_name=["release","debug","shared_release", "shared_debug"])
<cproject storage_type_id="org.eclipse.cdt.core.XmlProjectDescriptionStorage">
<storageModule moduleId="org.eclipse.cdt.core.settings">
<cconfiguration id="cdt.managedbuild.toolchain.gnu.mingw.base.2137728516">
<storageModule buildSystemId="org.eclipse.cdt.managedbuilder.core.configurationDataProvider" id="cdt.managedbuild.toolchain.gnu.mingw.base.2137728516" moduleId="org.eclipse.cdt.core.settings" name="Default">
<externalSettings/>
<extensions>
<extension id="org.eclipse.cdt.core.PE" point="org.eclipse.cdt.core.BinaryParser"/>
<extension id="org.eclipse.cdt.core.MakeErrorParser" point="org.eclipse.cdt.core.ErrorParser"/>
<extension id="org.eclipse.cdt.core.GCCErrorParser" point="org.eclipse.cdt.core.ErrorParser"/>
<extension id="org.eclipse.cdt.core.GASErrorParser" point="org.eclipse.cdt.core.ErrorParser"/>
<extension id="org.eclipse.cdt.core.GLDErrorParser" point="org.eclipse.cdt.core.ErrorParser"/>
</extensions>
</storageModule>
<storageModule moduleId="cdtBuildSystem" version="4.0.0">
<configuration artifactName=@"projectName" buildProperties="" id="cdt.managedbuild.toolchain.gnu.mingw.base.2137728516" name="Default" parent="org.eclipse.cdt.build.core.emptycfg">
<folderInfo id="cdt.managedbuild.toolchain.gnu.mingw.base.2137728516.1653789009" name="/" resourcePath="">
<toolChain id="cdt.managedbuild.toolchain.gnu.mingw.base.1128069344" name="cdt.managedbuild.toolchain.gnu.mingw.base" superClass="cdt.managedbuild.toolchain.gnu.mingw.base">
<targetPlatform archList="all" binaryParser="org.eclipse.cdt.core.PE" id="cdt.managedbuild.target.gnu.platform.mingw.base.1957129145" name="Debug Platform" osList="win32" superClass="cdt.managedbuild.target.gnu.platform.mingw.base"/>
<builder id="cdt.managedbuild.toolchain.gnu.mingw.base.2137728516.2116020721" managedBuildOn="false" name="Gnu Make Builder.Preference Configuration" superClass="org.eclipse.cdt.build.core.settings.default.builder"/>
<tool id="cdt.managedbuild.tool.gnu.assembler.mingw.base.1519718953" name="GCC Assembler" superClass="cdt.managedbuild.tool.gnu.assembler.mingw.base">
<inputType id="cdt.managedbuild.tool.gnu.assembler.input.325004657" superClass="cdt.managedbuild.tool.gnu.assembler.input"/>
</tool>
<tool id="cdt.managedbuild.tool.gnu.archiver.mingw.base.1770036552" name="GCC Archiver" superClass="cdt.managedbuild.tool.gnu.archiver.mingw.base"/>
<tool id="cdt.managedbuild.tool.gnu.cpp.compiler.mingw.base.1386135478" name="GCC C++ Compiler" superClass="cdt.managedbuild.tool.gnu.cpp.compiler.mingw.base">
<inputType id="cdt.managedbuild.tool.gnu.cpp.compiler.input.1732476077" superClass="cdt.managedbuild.tool.gnu.cpp.compiler.input"/>
</tool>
<tool id="cdt.managedbuild.tool.gnu.c.compiler.mingw.base.2131097432" name="GCC C Compiler" superClass="cdt.managedbuild.tool.gnu.c.compiler.mingw.base">
<inputType id="cdt.managedbuild.tool.gnu.c.compiler.input.1523845007" superClass="cdt.managedbuild.tool.gnu.c.compiler.input"/>
</tool>
<tool id="cdt.managedbuild.tool.gnu.c.linker.mingw.base.614615046" name="MinGW C Linker" superClass="cdt.managedbuild.tool.gnu.c.linker.mingw.base"/>
<tool id="cdt.managedbuild.tool.gnu.cpp.linker.mingw.base.1426207875" name="MinGW C++ Linker" superClass="cdt.managedbuild.tool.gnu.cpp.linker.mingw.base">
<inputType id="cdt.managedbuild.tool.gnu.cpp.linker.input.2129689705" superClass="cdt.managedbuild.tool.gnu.cpp.linker.input">
<additionalInput kind="additionalinputdependency" paths="$(USER_OBJS)"/>
<additionalInput kind="additionalinput" paths="$(LIBS)"/>
</inputType>
</tool>
</toolChain>
</folderInfo>
</configuration>
</storageModule>
<storageModule moduleId="scannerConfiguration">
<autodiscovery enabled="true" problemReportingEnabled="true" selectedProfileId=""/>
<profile id="org.eclipse.cdt.make.core.GCCStandardMakePerProjectProfile">
<buildOutputProvider>
<openAction enabled="true" filePath=""/>
<parser enabled="true"/>
</buildOutputProvider>
<scannerInfoProvider id="specsFile">
<runAction arguments="-E -P -v -dD ${plugin_state_location}/${specs_file}" command="gcc" useDefault="true"/>
<parser enabled="true"/>
</scannerInfoProvider>
</profile>
<profile id="org.eclipse.cdt.make.core.GCCStandardMakePerFileProfile">
<buildOutputProvider>
<openAction enabled="true" filePath=""/>
<parser enabled="true"/>
</buildOutputProvider>
<scannerInfoProvider id="makefileGenerator">
<runAction arguments="-f ${project_name}_scd.mk" command="make" useDefault="true"/>
<parser enabled="true"/>
</scannerInfoProvider>
</profile>
<profile id="org.eclipse.cdt.managedbuilder.core.GCCManagedMakePerProjectProfile">
<buildOutputProvider>
<openAction enabled="true" filePath=""/>
<parser enabled="true"/>
</buildOutputProvider>
<scannerInfoProvider id="specsFile">
<runAction arguments="-E -P -v -dD ${plugin_state_location}/${specs_file}" command="gcc" useDefault="true"/>
<parser enabled="true"/>
</scannerInfoProvider>
</profile>
<profile id="org.eclipse.cdt.managedbuilder.core.GCCManagedMakePerProjectProfileCPP">
<buildOutputProvider>
<openAction enabled="true" filePath=""/>
<parser enabled="true"/>
</buildOutputProvider>
<scannerInfoProvider id="specsFile">
<runAction arguments="-E -P -v -dD ${plugin_state_location}/specs.cpp" command="g++" useDefault="true"/>
<parser enabled="true"/>
</scannerInfoProvider>
</profile>
<profile id="org.eclipse.cdt.managedbuilder.core.GCCManagedMakePerProjectProfileC">
<buildOutputProvider>
<openAction enabled="true" filePath=""/>
<parser enabled="true"/>
</buildOutputProvider>
<scannerInfoProvider id="specsFile">
<runAction arguments="-E -P -v -dD ${plugin_state_location}/specs.c" command="gcc" useDefault="true"/>
<parser enabled="true"/>
</scannerInfoProvider>
</profile>
<profile id="org.eclipse.cdt.managedbuilder.core.GCCWinManagedMakePerProjectProfile">
<buildOutputProvider>
<openAction enabled="true" filePath=""/>
<parser enabled="true"/>
</buildOutputProvider>
<scannerInfoProvider id="specsFile">
<runAction arguments="-c 'gcc -E -P -v -dD &quot;${plugin_state_location}/${specs_file}&quot;'" command="sh" useDefault="true"/>
<parser enabled="true"/>
</scannerInfoProvider>
</profile>
<profile id="org.eclipse.cdt.managedbuilder.core.GCCWinManagedMakePerProjectProfileCPP">
<buildOutputProvider>
<openAction enabled="true" filePath=""/>
<parser enabled="true"/>
</buildOutputProvider>
<scannerInfoProvider id="specsFile">
<runAction arguments="-c 'g++ -E -P -v -dD &quot;${plugin_state_location}/specs.cpp&quot;'" command="sh" useDefault="true"/>
<parser enabled="true"/>
</scannerInfoProvider>
</profile>
<profile id="org.eclipse.cdt.managedbuilder.core.GCCWinManagedMakePerProjectProfileC">
<buildOutputProvider>
<openAction enabled="true" filePath=""/>
<parser enabled="true"/>
</buildOutputProvider>
<scannerInfoProvider id="specsFile">
<runAction arguments="-c 'gcc -E -P -v -dD &quot;${plugin_state_location}/specs.c&quot;'" command="sh" useDefault="true"/>
<parser enabled="true"/>
</scannerInfoProvider>
</profile>
</storageModule>
<storageModule moduleId="org.eclipse.cdt.core.externalSettings"/>
<storageModule moduleId="org.eclipse.cdt.core.language.mapping"/>
<storageModule moduleId="org.eclipse.cdt.make.core.buildtargets">
<buildTargets>@for(ci=0;ci<nr_configs;++ci)@{
<target name=@"config_name[ci]" path="" targetID="org.eclipse.cdt.build.MakeTargetBuilder">
<buildCommand>make</buildCommand>
<buildArguments/>
<buildTarget>@(config_name[ci])</buildTarget>
<stopOnError>true</stopOnError>
<useDefaultCommand>true</useDefaultCommand>
<runAllBuilders>true</runAllBuilders>
</target>
@}
</buildTargets>
</storageModule>
</cconfiguration>
</storageModule>
<storageModule moduleId="cdtBuildSystem" version="4.0.0">
<project id="@(projectName).null.1612486529" name=@"projectName"/>
</storageModule>
</cproject>
