<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the model "ExportModelJSON" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="save smtk model" Label="Model - Save" BaseType="operator">
      <AssociationsDef Name="models" NumberOfRequiredValues="1" Extensible="true">
        <MembershipMask>model</MembershipMask>
      </AssociationsDef>
      <BriefDescription>
        Save one or more smtk models.
      </BriefDescription>
      <DetailedDescription>
        Save models in SMTK's native JSON format.
      </DetailedDescription>
      <ItemDefinitions>
        <File
          Name="filename"
          Label="SMTK Model File Name "
          FileFilters="SMTK Model (*.smtk)"
          ShouldExist="false"
          NumberOfRequiredValues="1">
          <BriefDescription>The destination file for the JSON.</BriefDescription>
        </File>

        <!--
        <String Name="mode" NumberOfRequiredValues="1">
          <BriefDescription>Choose how the SMTK file and underlying kernel files are saved.</BriefDescription>
          <DetailedDescription>
            Choose how the SMTK file and underlying kernel files are saved:
            <ul>
              <li>Save: Overwrite the existing files with modified versions.
                No model names or URLs are modified.
              </li>
              <li>Save as: Change the name of the SMTK file as directed and,
                if kernel files are modified, save them to a new location.
                Model URLs and model names are changed to reflect the new
                location after the operator completes.
              </li>
              <li>Save a copy: Write all files to a new location.
                If any files already exist at this new location, fail.
                Model URLs and model names are reverted to their previous
                values after the file is saved.
              </li>
            </ul>
          </DetailedDescription>
          <DiscreteInfo DefaultIndex="0">
            <Value Enum="save">save</Value>
            <Value Enum="save as">save as</Value>
            <Value Enum="save a copy">save a copy</Value>
          </DiscreteInfo>
        </String>

        <Void Name="embed data">
          <BriefDescription>Choose whether to embed auxiliary geometry and kernel files.</BriefDescription>
          <DetailedDescription>
            This flag is ignored when the "mode" item is set to "save" (data is never embedded)
            but obeyed when set to "save a copy" (although data should be embedded by default)
            or "save as" (where data should only be embedded if the original URLs were embedded).
          </DetailedDescription>
        </Void>

        <String Name="rename models" Extensible="0" NumberOfRequiredValues="1" AdvanceLevel="2">
          <BriefDescription>Choose whether and which models will be renamed just before export.</BriefDescription>
          <DetailedDescription>
            By default, models with no names or default names (e.g., "model 0") will be renamed
            just prior to export to include the stem of the filename and, if multiple models were
            saved, a number. If the model name starts with the previous filename (obtained from
            the "url" property attached to the model), then it is considered a default name and
            will be changed if the new filename is different.

            You may opt to force all models to be renamed or to prevent renaming.
          </DetailedDescription>
          <DiscreteInfo DefaultIndex="0">
            <Value Enum="only default">only rename models with default names</Value>
            <Value Enum="all">rename all models</Value>
            <Value Enum="none">do not rename models</Value>
          </DiscreteInfo>
        </String>
        -->

        <Void Name="show non-active models" IsEnabledByDefault="true" AdvanceLevel="11">
          <BriefDescription>
            Show non-active models which belong to the active model's session
            in the available models combobox.
          </BriefDescription>
        </Void>

        <!-- Property changes to make before/after saving -->
        <Group Name="property edits" Extensible="true" NumberOfRequiredGroups="0">
					<BriefDescription>
            A set of entities whose properties will change plus key/value pairs
            specifying the changes for each entity.
					</BriefDescription>
					<ItemDefinitions>

            <ModelEntity Name="edit entity" NumberOfRequiredValues="1">
              <BriefDescription>
                A model entity and/or mesh collection UUID whose properties
                should be modified before/after saving.
              </BriefDescription>
            </ModelEntity>

            <Group Name="value pairs" Extensible="true" NumberOfRequiredGroups="1">
							<BriefDescription>
								A string property name ("edit property") and list of values ("edit values")
                to be modified before/after saving.
							</BriefDescription>
							<ItemDefinitions>

                <String Name="edit property" NumberOfRequiredValues="1">
                  <BriefDescription>
                    A property name (commonly "name", "url", and/or "smtk_url").
                  </BriefDescription>
                </String>

						    <String Name="edit values" NumberOfRequiredValues="1" Extensible="true">
						    	<BriefDescription>
						    		A value to store with the property name above before/after saving.
										Typically a name or URL.
						    	</BriefDescription>
						    </String>

							</ItemDefinitions>
						</Group>

					</ItemDefinitions>
				</Group>

        <Void Name="undo edits" IsEnabledByDefault="false" Optional="true">
          <BriefDescription>
            Should the property edit made before saving be undone after saving?
          </BriefDescription>
          <DetailedDescription>
            The "undo edits" item allows this operator to implement the semantics
            of "save a copy" operations, where model names and filenames for the
            saved files are not preserved.
          </DetailedDescription>
        </Void>

        <!-- Actions to take on resources -->
        <String Name="copy files" NumberOfRequiredValues="0" Extensible="true"></String>
        <String Name="save models" NumberOfRequiredValues="0" Extensible="true"></String>
        <MeshEntity Name="save meshes" NumberOfRequiredValues="0" Extensible="true"></MeshEntity>
        <String Name="save mesh urls" NumberOfRequiredValues="0" Extensible="true"></String>
      </ItemDefinitions>
    </AttDef>

    <!-- Result -->
    <AttDef Type="result(save smtk model)" BaseType="result">
      <ItemDefinitions>
        <Void Name="cleanse entities" IsEnabledByDefault="true"></Void>
      </ItemDefinitions>
    </AttDef>
  </Definitions>

  <Views>
     <!--
      The customized view "Type" needs to match the plugin's VIEW_NAME:
      add_smtk_ui_view(...  VIEW_NAME smtkSaveModelView ...)
      -->
    <View Type="smtkSaveModelView" Title="Save Model">
      <AttributeTypes>
        <Att Type="save smtk model"/>
      </AttributeTypes>
    </View>
  </Views>
</SMTK_AttributeSystem>
