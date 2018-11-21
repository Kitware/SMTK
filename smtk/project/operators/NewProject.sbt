<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the project "NewProject" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
<!--     <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="new project" BaseType="operation" Label="Project - New">
 -->
    <AttDef Type="new-project" Label="New Project" BaseType="" Label="Project - New">
      <BriefDescription>
        Create and initialize a CMB project.
      </BriefDescription>
      <DetailedDescription>
        A CMB project stores and manages the data needed to build
        simulation input decks. The initial version of the project
        operator will take as input two files, (i) an Exodus model
        file and (ii) a simulation template (workflow) file.
        All persistent data are stored in a filesystem directory
        that is assigned when the project is created.
      </DetailedDescription>
      <ItemDefinitions>
        <String Name="project-name" Label="Project Name" NumberOfRequiredValues="1">
          <BriefDescription>An informal string for user reference</BriefDescription>
        </String>
        <Directory Name="project-directory" Label="Project Directory" NumberOfRequiredValues="1">
          <BriefDescription>A new/empty directory for storing project files</BriefDescription>
        </Directory>
        <File Name="template-file" Label="Simulation Template File" NumberOfRequiredValues="1"
          ShouldExist="true" Optional="true" IsEnabledByDefault="true"
          FileFilters="CMB Template Files (*.sbt);;All Files (*)">
          <BriefDescription>The CMB template file (*.sbt) specifying the simulation</BriefDescription>
        </File>
        <File Name="model-file" Label="Model File" NumberOfRequiredValues="1" ShouldExist="true"
          Optional="true" IsEnabledByDefault="true"
          FileFilters="Exodus Files (*.ex? *.gen);;All Files (*)">
          <BriefDescription>The model file to import into the project.</BriefDescription>
          <DetailedDescription>The current implementation only supports Exodus file.</DetailedDescription>
        </File>
        <String Name="model-file-identifier" Label="Model File Label" NumberOfRequiredValues="1" AdvanceLevel="1">
          <BriefDescription>A text label that can be used to identify this model</BriefDescription>
          <DefaultValue>default</DefaultValue>
        </String>
        <Void Name="copy-model-file" Label="Copy Model File Into Project" AdvanceLevel="1"
          Optional="true" IsEnabledByDefault="true">
          <BriefDescription></BriefDescription>
        </Void>
        <String Name="session-type" Label="Modeling Session" AdvanceLevel="1">
          <BriefDescription></BriefDescription>
          <DiscreteInfo DefaultIndex="0">
            <Value Enum="Mesh">mesh</Value>
            <Value Enum="VTK">vtk</Value>>
          </DiscreteInfo>
        </String>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
<!--     <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(new project)" BaseType="result">
      <ItemDefinitions>
      </ItemDefinitions>
    </AttDef>
 -->
  </Definitions>
</SMTK_AttributeResource>
