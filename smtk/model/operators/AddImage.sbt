<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the model "AddImage" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="add image" BaseType="operator" Label=" Model - Add Image">
      <AssociationsDef Name="entities" NumberOfRequiredValues="1">
        <MembershipMask>model</MembershipMask>
      </AssociationsDef>
      <BriefDescription>
        Add am image as  auxiliary geometry (scene geometry not part of the model domain)
        to a model or to another auxiliary geometry instance.
      </BriefDescription>
      <DetailedDescription>
        Add am image as auxiliary geometry (scene geometry not part of the model domain)
        to a model or to another auxiliary geometry instance.

        Auxiliary geometry may be made hierarchical, by adding an auxiliary
        geometry instance with no URL to a model and then adding 1 or more
        auxiliary geometry instances to the first instance above.
        This is useful when you wish to instance multiple files into the
        scene as if they were a single file.
      </DetailedDescription>
      <ItemDefinitions>
        <File Name="url" Label="Image File Name" Optional="true" NumberOfRequiredValues="1"
          IsEnabledByDefault="true" ShouldExist="true"
          FileFilters="Image files (*.tif *.tiff *.dem *.vti);;All files (*.*)">
          <BriefDescription>The file containing the image.</BriefDescription>
        </File>

        <Void Name="separate representation" AdvanceLevel="1" Optional="true" IsEnabledByDefault="true"
          Label="Display as separate representation from model">
          <BriefDescription>
            Should the auxiliary geometry's representation be separate from its owning model's?
          </BriefDescription>
          <DetailedDescription>
            Should the auxiliary geometry's representation be separate from its owning model's?
            If yes, a separate rendering pipeline will be created for auxiliary geometry and
            its representation will be controlled with its own set of display properties;
            if no, the geometry will be shown and controlled as sub-blocks in the model's multiblock dataset,
            which may be less flexible.
          </DetailedDescription>
        </Void>

      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(add image)" BaseType="result">
      <ItemDefinitions>
        <!-- The modified entities are stored in the base result's "modified" item. -->
        <ModelEntity Name="tess_changed" NumberOfRequiredValues="1"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
