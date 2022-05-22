<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the geometric "image inspector" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>

    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="smtk::geometry::DataSetInfoInspector" Label="info inspector" BaseType="operation">

      <BriefDescription>Inspect mesh statistics on components with renderable geometry.</BriefDescription>
      <DetailedDescription>
        This tool displays the number of nodes and cells corresponding to the selected components.
        Note that only components with renderable geometry can be selected.
      </DetailedDescription>
      <AssociationsDef Name="source" Label=" " NumberOfRequiredValues="1" Extensible="yes">
        <BriefDescription>The component(s) to inspect.

Space key toggles selection of the component for analysis.
Return key toggles component visibility.</BriefDescription>
        <Accepts>
          <Resource Name="smtk::model::Resource" Filter="*"/>
          <Resource Name="smtk::geometry::Resource" Filter="*"/>
          <Resource Name="smtk::graph::Resource" Filter="*"/>
        </Accepts>
      </AssociationsDef>

      <ItemDefinitions>
      </ItemDefinitions>

    </AttDef>

    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(smtk::geometry::DataSetInfoInspector)" BaseType="result">
      <ItemDefinitions>

        <Group Name="information" NumberOfRequiredGroups="0" Extensible="yes">
          <ItemDefinitions>
            <Component Name="component" NumberOfRequiredValues="1"/>
            <Int Name="point count" Label="#(points)" Extensible="yes"/>
            <!-- An item to hold the count of each type of cell -->
            <Int Name="vtkEmptyCell count" Label="#(empty cells)" NumberOfRequiredValues="1"/>
            <Int Name="vtkVertex count" Label="#(vertices)" NumberOfRequiredValues="1"/>
            <Int Name="vtkPolyVertex count" Label="#(poly-vertices)" NumberOfRequiredValues="1"/>
            <Int Name="vtkLine count" Label="#(lines)" NumberOfRequiredValues="1"/>
            <Int Name="vtkPolyLine count" Label="#(poly-lines)" NumberOfRequiredValues="1"/>
            <Int Name="vtkTriangle count" Label="#(triangles)" NumberOfRequiredValues="1"/>
            <Int Name="vtkTriangleStrip count" Label="#(strips)" NumberOfRequiredValues="1"/>
            <Int Name="vtkPolygon count" Label="#(polygons)" NumberOfRequiredValues="1"/>
            <Int Name="vtkPixel count" Label="#(pixels)" NumberOfRequiredValues="1"/>
            <Int Name="vtkQuad count" Label="#(quadrilaterals)" NumberOfRequiredValues="1"/>
            <Int Name="vtkTetra count" Label="#(tetrahedra)" NumberOfRequiredValues="1"/>
            <Int Name="vtkVoxel count" Label="#(voxels)" NumberOfRequiredValues="1"/>
            <Int Name="vtkHexahedron count" Label="#(hexahedra)" NumberOfRequiredValues="1"/>
            <Int Name="vtkWedge count" Label="#(wedges)" NumberOfRequiredValues="1"/>
            <Int Name="vtkPyramid count" Label="#(pyramids)" NumberOfRequiredValues="1"/>
            <Int Name="vtkPentagonalPrism count" Label="#(pentagonal prisms)" NumberOfRequiredValues="1"/>
            <Int Name="vtkHexagonalPrism count" Label="#(hexagonal prisms)" NumberOfRequiredValues="1"/>
            <Int Name="vtkQuadraticEdge count" Label="#(quadratic edges)" NumberOfRequiredValues="1"/>
            <Int Name="vtkQuadraticTriangle count" Label="#(quadratic triangles)" NumberOfRequiredValues="1"/>
            <Int Name="vtkQuadraticQuad count" Label="#(quadratic quadrilaterals)" NumberOfRequiredValues="1"/>
            <Int Name="vtkQuadraticTetra count" Label="#(quadratic tetrahedra)" NumberOfRequiredValues="1"/>
            <Int Name="vtkQuadraticHexahedron count" Label="#(quadratic hexahedra)" NumberOfRequiredValues="1"/>
            <Int Name="vtkQuadraticWedge count" Label="#(quadratic wedges)" NumberOfRequiredValues="1"/>
            <Int Name="vtkQuadraticPyramid count" Label="#(quadratic pyramids)" NumberOfRequiredValues="1"/>
            <Int Name="vtkBiQuadraticQuad count" Label="#(bi-quadratic quadrilaterals)" NumberOfRequiredValues="1"/>
            <Int Name="vtkTriQuadraticHexahedron count" Label="#(tri-quadratic hexahedra)" NumberOfRequiredValues="1"/>
            <Int Name="vtkQuadraticLinearQuad count" Label="#(quadratic+linear quadrilaterals)" NumberOfRequiredValues="1"/>
            <Int Name="vtkQuadraticLinearWedge count" Label="#(quadratic+linear wedges)" NumberOfRequiredValues="1"/>
            <Int Name="vtkBiQuadraticQuadraticWedge count" Label="#(bi-quadratic wedges)" NumberOfRequiredValues="1"/>
            <Int Name="vtkBiQuadraticQuadraticHexahedron count" Label="#(bi-quadratic hexahedra)" NumberOfRequiredValues="1"/>
            <Int Name="vtkBiQuadraticTriangle count" Label="#(bi-quadratic triangles)" NumberOfRequiredValues="1"/>
            <Int Name="vtkCubicLine count" Label="#(cubic lines)" NumberOfRequiredValues="1"/>
            <Int Name="vtkQuadraticPolygon count" Label="#(quadratic polygons)" NumberOfRequiredValues="1"/>
            <Int Name="vtkTriQuadraticPyramid count" Label="#(tri-quadratic pyramids)" NumberOfRequiredValues="1"/>
            <Int Name="vtkConvexPointSet count" Label="#(convex point sets)" NumberOfRequiredValues="1"/>
            <Int Name="vtkPolyhedron count" Label="#(polyhedra)" NumberOfRequiredValues="1"/>
            <Int Name="vtkParametricCurve count" Label="#(parametric curves)" NumberOfRequiredValues="1"/>
            <Int Name="vtkParametricSurface count" Label="#(parametric surfaces)" NumberOfRequiredValues="1"/>
            <Int Name="vtkParametricTriSurface count" Label="#(parametric tri-surfaces)" NumberOfRequiredValues="1"/>
            <Int Name="vtkParametricQuadSurface count" Label="#(parametric quad-surfaces)" NumberOfRequiredValues="1"/>
            <Int Name="vtkParametricTetraRegion count" Label="#(parametric tetrahedral regions)" NumberOfRequiredValues="1"/>
            <Int Name="vtkParametricHexRegion count" Label="#(parametric hexahedral regions)" NumberOfRequiredValues="1"/>
            <Int Name="vtkHigherOrderEdge count" Label="#(higher order edges)" NumberOfRequiredValues="1"/>
            <Int Name="vtkHigherOrderTriangle count" Label="#(higher order triangles)" NumberOfRequiredValues="1"/>
            <Int Name="vtkHigherOrderQuad count" Label="#(higher order quadrilaterals)" NumberOfRequiredValues="1"/>
            <Int Name="vtkHigherOrderPolygon count" Label="#(higher order polygons)" NumberOfRequiredValues="1"/>
            <Int Name="vtkHigherOrderTetrahedron count" Label="#(higher order tetrahedra)" NumberOfRequiredValues="1"/>
            <Int Name="vtkHigherOrderWedge count" Label="#(higher order wedges)" NumberOfRequiredValues="1"/>
            <Int Name="vtkHigherOrderPyramid count" Label="#(higher order pyramids)" NumberOfRequiredValues="1"/>
            <Int Name="vtkHigherOrderHexahedron count" Label="#(higher order hexahedra)" NumberOfRequiredValues="1"/>
            <Int Name="vtkLagrangeCurve count" Label="#(lagrange curves)" NumberOfRequiredValues="1"/>
            <Int Name="vtkLagrangeQuadrilateral count" Label="#(lagrange quadrilaterals)" NumberOfRequiredValues="1"/>
            <Int Name="vtkLagrangeTriangle count" Label="#(lagrange triangles)" NumberOfRequiredValues="1"/>
            <Int Name="vtkLagrangeTetra count" Label="#(lagrange tetrahedra)" NumberOfRequiredValues="1"/>
            <Int Name="vtkLagrangeHexahedron count" Label="#(lagrange hexahedra)" NumberOfRequiredValues="1"/>
            <Int Name="vtkLagrangeWedge count" Label="#(lagrange wedges)" NumberOfRequiredValues="1"/>
            <Int Name="vtkLagrangePyramid count" Label="#(lagrange pyramids)" NumberOfRequiredValues="1"/>
            <Int Name="vtkBezierCurve count" Label="#(bezier curves)" NumberOfRequiredValues="1"/>
            <Int Name="vtkBezierQuadrilateral count" Label="#(bezier quadrilaterals)" NumberOfRequiredValues="1"/>
            <Int Name="vtkBezierTriangle count" Label="#(bezier triangles)" NumberOfRequiredValues="1"/>
            <Int Name="vtkBezierTetra count" Label="#(bezier tetrahedra)" NumberOfRequiredValues="1"/>
            <Int Name="vtkBezierHexahedron count" Label="#(bezier hexahedra)" NumberOfRequiredValues="1"/>
            <Int Name="vtkBezierWedge count" Label="#(bezier wedges)" NumberOfRequiredValues="1"/>
            <Int Name="vtkBezierPyramid count" Label="#(bezier pyramids)" NumberOfRequiredValues="1"/>
            <Group Name="arrays">
              <ItemDefinitions>
                <String Name="name" NumberOfRequiredValues="1"/>
                <String Name="type" NumberOfRequiredValues="1"/>
                <Int    Name="components" NumberOfRequiredValues="1"/>
                <String Name="storage" NumberOfRequiredValues="1"/>
                <Double Name="cdf" NumberOfRequiredValues="2" Extensible="yes"/>
              </ItemDefinitions>
            </Group>
          </ItemDefinitions>
        </Group>

      </ItemDefinitions>
    </AttDef>

  </Definitions>
  <Views>
    <View Type="smtkDataSetInfoInspectorView" Title="Dataset Info Inspector"
      FilterByAdvanceLevel="false" FilterByCategoryMode="false"
      FilterByCategory="false" IgnoreCategories="true">
      <AttributeTypes>
        <Att Type="smtk::geometry::DataSetInfoInspector" Name="Dataset info inspector">
          <ItemViews>
            <View Path="/source" Type="qtReferenceTree"
              DrawSubtitle="false"
              VisibilityMode="true"
              TextVerticalPad="6"
              TitleFontWeight="1"
              HighlightOnHover="false"
              >
              <PhraseModel Type="smtk::view::ResourcePhraseModel">
                <SubphraseGenerator Type="smtk::view::SubphraseGenerator"/>
                <Badges>
                  <Badge
                    Type="smtk::extension::qt::MembershipBadge"
                    MembershipCriteria="ComponentsWithGeometry"
                    Filter="any"
                    Default="false"/>
                  <Badge
                    Type="smtk::extension::paraview::appcomponents::VisibilityBadge"
                    Default="false"/>
                </Badges>
              </PhraseModel>
            </View>
          </ItemViews>
        </Att>
      </AttributeTypes>
    </View>
  </Views>
</SMTK_AttributeResource>
