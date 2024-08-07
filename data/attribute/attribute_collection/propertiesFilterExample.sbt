
<SMTK_AttributeResource Version="8">
  <Definitions>
    <AttDef Type="Base">
      <Properties>
        <Property Name="alpha" Type="Long"> 100 </Property>
        <Property Name="beta" Type="String">cat</Property>
        <Property Name="gamma" Type="Double">3.141</Property>
      </Properties>
    </AttDef>
    <AttDef Type="A" BaseType="Base">
      <Properties>
        <Property Name="alpha" Type="Long"> 200 </Property>
        <Property Name="beta" Type="String">dog</Property>
      </Properties>
    </AttDef>
  </Definitions>
  <Attributes>
    <Att Name="a" Type="A">
      <Properties>
        <Property Name="alpha" Type="Long"> 500 </Property>
      </Properties>
    </Att>
    <Att Name="a1" Type="A"/>
  </Attributes>
</SMTK_AttributeResource>
