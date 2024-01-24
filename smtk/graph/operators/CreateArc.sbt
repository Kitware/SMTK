<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the "create arc type" Operation -->
<SMTK_AttributeResource Version="7">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="create arc" Label="connect nodes" BaseType="operation">

      <AssociationsDef Name="from nodes"
        NumberOfRequiredValues="1"
        MaximumNumberOfValues="2"
        Extensible="true"
        HoldReference="true">
        <Accepts><Resource Name="smtk::graph::ResourceBase" Filter="*"/></Accepts>
      </AssociationsDef>

      <ItemDefinitions>
        <String Name="arc type">
          <BriefDescription>
            The type of arc used to connect the nodes.
          </BriefDescription>
        </String>

        <Reference Name="to node" NumberOfRequiredValues="0" MaximumNumberOfValues="1" Extensible="true">
          <Accepts><Resource Name="smtk::graph::ResourceBase" Filter="*"/></Accepts>
          <BriefDescription>
            The destination node (for directed arcs).
          </BriefDescription>
          <DetailedDescription>
            For undirected arc creation, this item may
            be unspecified if there are two associated nodes.
            To create a directed arc, there must be one
            associated node and one node referenced by this
            item.
          </DetailedDescription>
        </Reference>

      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(create arc)" BaseType="result">
      <ItemDefinitions>
        <Void Name="allow camera reset" IsEnabledByDefault="true" AdvanceLevel="11"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
