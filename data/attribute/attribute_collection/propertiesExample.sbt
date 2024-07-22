
<SMTK_AttributeResource Version="8">
  <Properties>
    <Property Name="pi" Type="Int"> 42 </Property>
    <Property Name="pd" Type="double"> 3.141 </Property>
    <Property Name="ps" Type="STRING">Test string</Property>
    <Property Name="pb" Type="bool"> YES </Property>
    <Property Name="animals" Type="vector[string]">
      <Value>the dog</Value>
      <Value>a cat</Value>
    </Property>
  </Properties>
  <Definitions>
    <AttDef Type="Test">
      <Properties>
        <Property Name="alpha" Type="Int"> 100 </Property>
      </Properties>
    </AttDef>
  </Definitions>
  <Attributes>
    <Att Name="foo" Type="Test">
      <Properties>
        <Property Name="pi" Type="int"> 69 </Property>
        <Property Name="pd" Type="Double"> 3.141 </Property>
        <Property Name="ps" Type="String"></Property>
        <Property Name="pb" Type="Bool"> 1 </Property>
        <Property Name="pvd" Type="vector[double]">
          <Value>10.0</Value>
          <Value>20.0</Value>
        </Property>
      </Properties>
    </Att>
  </Attributes>
</SMTK_AttributeResource>
