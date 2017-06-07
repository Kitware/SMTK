<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the model "ExportModelJSON" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="export smtk model" Label="Model - Export" BaseType="operator">
      <AssociationsDef Name="models" NumberOfRequiredValues="1" Extensible="true">
        <MembershipMask>model</MembershipMask>
      </AssociationsDef>
      <BriefDescription>
        Export one or more smtk models.
      </BriefDescription>
      <DetailedDescription>
        Export models in SMTK's native JSON format, copying all auxiliary data and meshes
        to a separate location.
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
    <AttDef Type="result(export smtk model)" BaseType="result">
      <ItemDefinitions>
        <Void Name="cleanse entities" IsEnabledByDefault="true"></Void>
      </ItemDefinitions>
    </AttDef>
  </Definitions>

  <Views>
     <!--
      The customized view "Type" needs to match the plugin's VIEW_NAME:
      add_smtk_ui_view(...  VIEW_NAME smtkExportModelView ...)
      -->
    <View Type="smtkExportModelView" Title="Export Model">
      <AttributeTypes>
        <Att Type="export smtk model"/>
      </AttributeTypes>
    </View>
  </Views>
</SMTK_AttributeSystem>
