IO
==

Supporting Vector Properties in XML Attribute Templates
-------------------------------------------------------

Starting in Version 8 XML Template Files, you can now define vector-based Properties on Attribute Resources and Attributes.
Currently vector of doubles and vectors of strings are supported but this can be easily extended.

Here is an example and is available in the data/attributes/attribute_collections directory as propertiesExample.sbt.

.. code-block:: XML

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
    <AttDef Type="Test"/>
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
