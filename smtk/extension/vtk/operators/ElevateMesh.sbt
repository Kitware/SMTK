<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the "interpolate mesh" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="elevate mesh"
            Label="Mesh - Elevate" BaseType="operator">
      <BriefDescription>
        Modify the z-coordinates a mesh's nodes according to an
        external data set.
      </BriefDescription>
      <DetailedDescription>
        &lt;p&gt;Modify the z-coordinates a mesh's nodes according to an
        external data set.
        &lt;p&gt;This operator accepts as input a mesh and an external
        data set, and it computes new z-coordinates at each mesh node
        as a radial average of the scalar values in the external data set.
      </DetailedDescription>
      <ItemDefinitions>
        <ModelEntity Name="auxiliary geometry" Label = "Auxiliary Geometry" NumberOfRequiredValues="1">
          <MembershipMask>aux_geom</MembershipMask>
          <BriefDescription>
            An external data set whose values determine the mesh nodes' elevations.
          </BriefDescription>
        </ModelEntity>

        <MeshEntity Name="mesh" Label="Mesh" NumberOfRequiredValues="1" Extensible="true" >
          <BriefDescription>The mesh to elevate.</BriefDescription>
        </MeshEntity>

        <Double Name="radius" Label="Radius for Averaging Elevation" NumberOfRequiredValues="1" Extensible="false">
          <BriefDescription>The radius used to identify a point locus
          for computing elevation.</BriefDescription>
          <DetailedDescription>
            The radius used to identify a point locus for computing
            elevation.

            For each node in the input mesh, a corresponding point
            locus whose positions projected onto the x-y plane are
            within a radius of the node is used to compute a new
            elevation value.
          </DetailedDescription>
          <DefaultValue>1.</DefaultValue>
        </Double>

        <Double Name="max elevation" Label="Maximum Elevation"
                NumberOfRequiredValues="1" Extensible="false" Optional="true">
          <BriefDescription>Upper limit to the resulting mesh's elevation range.</BriefDescription>
          <DefaultValue>0.0</DefaultValue>
        </Double>

        <Double Name="min elevation" Label="Minimum Elevation"
                NumberOfRequiredValues="1" Extensible="false" Optional="true">
          <BriefDescription>Lower limit to the resulting mesh's elevation range.</BriefDescription>
          <DefaultValue>0.0</DefaultValue>
        </Double>

        <Void Name="invert scalars" Label="Invert Scalar Values" Version="0"
              Optional="true" IsEnabledByDefault="false" AdvanceLevel="1">
          <BriefDescription>This toggle adds a prefactor of -1 to the
          values in the external data set prior to averaging.</BriefDescription>
        </Void>

      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(elevate mesh)" BaseType="result">
      <ItemDefinitions>
        <MeshEntity Name="mesh_modified" NumberOfRequiredValues="0" Extensible="true" AdvanceLevel="11"/>
        <ModelEntity Name="tess_changed" NumberOfRequiredValues="0"
                     Extensible="true" AdvanceLevel="11"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
