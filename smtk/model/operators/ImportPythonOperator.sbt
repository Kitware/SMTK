<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the model "ImportPythonOperator" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="import python operator" Label="Operator - Import from Python"
            BaseType="operator" AdvanceLevel="10">
      <BriefDescription>
        Import a python operator.
      </BriefDescription>
      <DetailedDescription>
        &lt;p&gt;A class for adding python operators to the current session.
        &lt;p&gt;Given a python file that describes an operator, this operator loads the
        python operator into the current session. The new operator is ready for use
        upon the successful completion of this operation (the session does not need
        to be restarted).
      </DetailedDescription>
      <ItemDefinitions>
        <File Name="filename" NumberOfRequiredValues="1"
              ShouldExist="true" FileFilters="Python (*.py)">
        </File>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(import python operator)" BaseType="result"/>
  </Definitions>
</SMTK_AttributeSystem>
