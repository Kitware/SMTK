<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CMB polygon Model "import" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="import image" Label=" Model - Import Image" BaseType="operator">
      <AssociationsDef Name="model" NumberOfRequiredValues="1" Extensible="yes">
        <MembershipMask>model</MembershipMask>
        <BriefDescription>The model that the image will be attached to.</BriefDescription>
        <DetailedDescription>
          The model that the image will be attached to.

          You must not specify both a model and an image file name.
        </DetailedDescription>
      </AssociationsDef>
      <ItemDefinitions>
        <File Name="filename" Label="File Name" NumberOfRequiredValues="1"
          ShouldExist="true"
          FileFilters="Supported files (*.vti *.tif *.tiff *.dem);;Image files (*.vti *.tif *.tiff);;DEM(*.dem);;All files (*.*)">
        </File>
        <Void Name="UseScalarColoring" Label="Map Scalars For Coloring" Optional="true">
          <BriefDescription>Should scalars be mapped with a color table for displaying image? If unchecked, the scalar values will be treated directly as colors.</BriefDescription>
        </Void>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(import image)" BaseType="result"/>
  </Definitions>
</SMTK_AttributeSystem>
