<?xml version="1.0"?>
<!--Created by XmlV4StringWriter-->
<SMTK_AttributeResource Version="4">
  <!--**********  Attribute Definitions ***********-->
  <Associations />
  <Definitions>
    <AttDef Type="A" Label="A" BaseType="" Unique="false">
      <ItemDefinitions>
        <File Name="f0" Label="Single File" Optional="true" ShouldExist="true" FileFilters="Exodus Files (*.ex? *.gen);;NetCDF Files (*.ncdf);;All Files (*)">
        </File>
        <File Name="f0a" Label="New File" Optional="true" FileFilters="Exodus Files (*.ex? *.gen);;NetCDF Files (*.ncdf);;All Files (*)">
        </File>
        <File Name="f0b" Label="New Files" Optional="true" FileFilters="Exodus Files (*.ex? *.gen);;NetCDF Files (*.ncdf);;All Files (*)"
          NumberOfRequiredValues="3">
        </File>
        <File Name="f1" Label="Three Files" Optional="true" NumberOfRequiredValues="3" ShouldExist="true">
          <ComponentLabels CommonLabel="File Name" />
        </File>
        <File Name="f2" Label="Extensible Files" Optional="true" Extensible="true" ShouldExist="true">
          <ComponentLabels CommonLabel="File Name" />
        </File>
        <File Name="f3" Label="Using testDir Property" Optional="true" Extensible="true"
          NumberOfRequiredValues="0">
        </File>
      </ItemDefinitions>
    </AttDef>
   </Definitions>
  <!--**********  Attribute Instances ***********-->
  <Views>
    <View Type="Instanced" Title="FileItemTest" Label="Simple File Item Test" TopLevel="true">
      <InstancedAttributes>
        <Att Type="A" Name="fileTestAttribute">
          <ItemViews>
            <View Item="f0" ShowRecentFiles="true"/>
            <View Item="f0a" ShowRecentFiles="true" ShowFileExtensions="true"/>
            <View Item="f0b" ShowRecentFiles="true" ShowFileExtensions="true"/>
            <View Item="f1" ShowRecentFiles="true"/>
            <View Item="f3" DefaultDirectoryProperty="testDir"/>
          </ItemViews>
        </Att>
      </InstancedAttributes>
    </View>
  </Views>
</SMTK_AttributeResource>
