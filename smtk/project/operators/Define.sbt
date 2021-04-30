<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the Project "Define" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="define" Label="Project - Define" BaseType="operation">
      <BriefDescription>
        Define a basic project type.
      </BriefDescription>
      <DetailedDescription>
        &lt;p&gt;Define a basic project type.
        &lt;p&gt;Options are available to white-list associated resources and operations.
      </DetailedDescription>

      <ItemDefinitions>

        <String Name="name" Label="Project Type Name"/>

        <String Name="resources" Label="Restrict Available Resource Types"
                AdvanceLevel="0" Optional="true"
                IsEnabledByDefault="false" >
        </String>

        <String Name="operations" Label="Restrict Available Operation Types"
                AdvanceLevel="0" Optional="true"
                IsEnabledByDefault="false" >
        </String>

      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(define)" BaseType="result"/>

  </Definitions>
</SMTK_AttributeResource>
