<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the "create arc type" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="create arc type" Label="create a new arc type" BaseType="operation">

      <AssociationsDef Name="resource" NumberOfRequiredValues="1" HoldReference="true" OnlyResources="true">
        <Accepts>
          <Resource Name="smtk::graph::ResourceBase"/>
          <Resource Name="smtk::markup::Resource"/>
        </Accepts>
        <BriefDescription>
          Choose the resource to hold the new arc type.
        </BriefDescription>
      </AssociationsDef>

      <ItemDefinitions>
        <String Name="type name">
          <BriefDescription>
            A name for the new type of arc.
          </BriefDescription>
          <DetailedDescription>
            This is a name for the new type of arc; it should not match the name of any existing arc type.
          </DetailedDescription>
        </String>

        <String Name="directionality">
          <BriefDescription>
            The directed-ness of the new arc type (directed or unirected).
          </BriefDescription>
          <DetailedDescription>
            The directed-ness of the new arc type (directed or unirected).
            Directed arcs may specify different filter string for the
            head and tail of each arc. Undirected arcs specify a single
            set of filter strings that are accepted at either end of
            each arc.
          </DetailedDescription>
          <ChildrenDefinitions>
            <String Name="from node types" NumberOfRequiredValues="0" Extensible="true">
              <BriefDescription>
                The types of nodes which can serve as "from" endpooints.
              </BriefDescription>
              <DetailedDescription>
                Enter a query-filter string describing the types of nodes which can serve
                as the originating node of arcs of this type (a.k.a. "source" or "from" nodes).

                If you do not provide any strings, then any type of node is allowed – which
                is equivalent to '*'.
              </DetailedDescription>
            </String>
            <String Name="to node types" NumberOfRequiredValues="0" Extensible="true">
              <BriefDescription>
                The types of nodes which can serve as "to" endpooints.
              </BriefDescription>
              <DetailedDescription>
                Enter a query-filter string describing the types of nodes which can serve
                as the destination node of arcs of this type (a.k.a. "target" or "to" nodes).

                If you do not provide any strings, then any type of node is allowed – which
                is equivalent to '*'.
              </DetailedDescription>
            </String>
            <String Name="end node types" NumberOfRequiredValues="0" Extensible="true">
              <BriefDescription>
                The types of nodes which can serve as endpooints.
              </BriefDescription>
              <DetailedDescription>
                Enter a query-filter string describing the types of nodes which can serve
                as either endpoint of arcs of this type.

                If you do not provide any strings, then any type of node is allowed – which
                is equivalent to '*'.
              </DetailedDescription>
            </String>
          </ChildrenDefinitions>
          <DiscreteInfo DefaultIndex="0">
            <Structure>
              <Value Enum="directed">directed</Value>
              <Items>
                <Item>from node types</Item>
                <Item>to node types</Item>
              </Items>
            </Structure>
            <Structure>
              <Value Enum="undirected">undirected</Value>
              <Items>
                <Item>end node types</Item>
              </Items>
            </Structure>
          </DiscreteInfo>
        </String>

      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(create arc type)" BaseType="result">
      <ItemDefinitions>
        <Void Name="allow camera reset" IsEnabledByDefault="true" AdvanceLevel="11"/>
        <Resource Name="resource" NumberOfRequiredValues="0" Extensible="true"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
