<?xml version="1.0" encoding="utf-8"?>
@func(::gen_xml; :>pj = MAP, :>return="")@{
<ProjectSchemaDefinitions xmlns="clr-namespace:Microsoft.Build.Framework.XamlTypes;assembly=Microsoft.Build.Framework" xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml" xmlns:sys="clr-namespace:System;assembly=mscorlib" xmlns:transformCallback="Microsoft.Cpp.Dev10.ConvertPropertyCallback">
  @for(:>i=0; i<!pj::rules; ++i)@{
     @if(is_rule(<:R =& pj::rules[i]))@{
       @define(:>rid = get_rule_name(<:R =& pj::rules[i]))
  <Rule
    Name=@"rid"
    PageTemplate="tool"
    DisplayName=@"rid-'_'"
    Order="200">
    <Rule.DataSource>
      <DataSource
        Persistence="ProjectFile"
        ItemType=@"rid" />
    </Rule.DataSource>
    <Rule.Categories>
      <Category
        Name="General">
        <Category.DisplayName>
          <sys:String>General</sys:String>
        </Category.DisplayName>
      </Category>
      <Category
        Name="Command Line"
        Subtype="CommandLine">
        <Category.DisplayName>
          <sys:String>Command Line</sys:String>
        </Category.DisplayName>
      </Category>
    </Rule.Categories>
    <StringListProperty
      Name="Inputs"
      Category="Command Line"
      IsRequired="true"
      Switch=" ">
      <StringListProperty.DataSource>
        <DataSource
          Persistence="ProjectFile"
          ItemType=@"rid"
          SourceType="Item" />
      </StringListProperty.DataSource>
    </StringListProperty>
    <StringProperty
      Name="CommandLineTemplate"
      DisplayName="Command Line"
      Visible="False"
      IncludeInCommandLine="False" />
    <DynamicEnumProperty
      Name=@"rid.'BeforeTargets'"
      Category="General"
      EnumProvider="Targets"
      IncludeInCommandLine="False">
      <DynamicEnumProperty.DisplayName>
        <sys:String>Execute Before</sys:String>
      </DynamicEnumProperty.DisplayName>
      <DynamicEnumProperty.Description>
        <sys:String>Specifies the targets for the build customization to run before.</sys:String>
      </DynamicEnumProperty.Description>
      <DynamicEnumProperty.ProviderSettings>
        <NameValuePair
          Name="Exclude"
          Value=@"'^'.rid.'BeforeTargets|^Compute'" />
      </DynamicEnumProperty.ProviderSettings>
      <DynamicEnumProperty.DataSource>
        <DataSource
          Persistence="ProjectFile"
          HasConfigurationCondition="true" />
      </DynamicEnumProperty.DataSource>
    </DynamicEnumProperty>
    <DynamicEnumProperty
      Name=@"rid.'AfterTargets'"
      Category="General"
      EnumProvider="Targets"
      IncludeInCommandLine="False">
      <DynamicEnumProperty.DisplayName>
        <sys:String>Execute After</sys:String>
      </DynamicEnumProperty.DisplayName>
      <DynamicEnumProperty.Description>
        <sys:String>Specifies the targets for the build customization to run after.</sys:String>
      </DynamicEnumProperty.Description>
      <DynamicEnumProperty.ProviderSettings>
        <NameValuePair
          Name="Exclude"
          Value=@"'^'.rid.'AfterTargets|^Compute'" />
      </DynamicEnumProperty.ProviderSettings>
      <DynamicEnumProperty.DataSource>
        <DataSource
          Persistence="ProjectFile"
          ItemType=""
          HasConfigurationCondition="true" />
      </DynamicEnumProperty.DataSource>
    </DynamicEnumProperty>
    <StringListProperty
      Name="Outputs"
      DisplayName="Outputs"
      Visible="False"
      IncludeInCommandLine="False" />
    <StringProperty
      Name="ExecutionDescription"
      DisplayName="Execution Description"
      Visible="False"
      IncludeInCommandLine="False" />
    <StringListProperty
      Name="AdditionalDependencies"
      DisplayName="Additional Dependencies"
      IncludeInCommandLine="False"
      Visible="false" />
    <StringProperty
      Subtype="AdditionalOptions"
      Name="AdditionalOptions"
      Category="Command Line">
      <StringProperty.DisplayName>
        <sys:String>Additional Options</sys:String>
      </StringProperty.DisplayName>
      <StringProperty.Description>
        <sys:String>Additional Options</sys:String>
      </StringProperty.Description>
    </StringProperty>
  </Rule>
  <ItemType
    Name=@"rid"
    DisplayName=@"rid-'_'" />
  <FileExtension
    Name=@"concat(pj::rules[i]::extensions, ';', '*.', '')"
    ContentType=@"rid" />
  <ContentType
    Name=@"rid"
    DisplayName=@"rid-'_'"
    ItemType=@"rid" />
    @}
  @}
</ProjectSchemaDefinitions>
@}

@skip(gen_xml(<:pj =& projects[current_project]))