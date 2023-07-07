Supporting Templates in Attribute XML Files
-------------------------------------------

Starting with Version 7 XML attribute template files, SMTK now supports Templates.  Templates are an extension to the existing ItemBlock concept.  The main difference between an ItemBlock and a Template is that a Template's contents can be parameterized.  When a Template is instantiated, these parameters can be assigned different values and will thereby change the information being copied.  a Template's parameter can also be given a default value.

**Note**  All parameters that do not have a default value must be given values when the Template is instanced.

Here is an example:

.. code-block:: xml

  <Templates>
    <Template Name="SimpleStingDefault">
      <Parameters>
        <Param Name="a">dog</Param>
      </Parameters>
      <Contents>
        <DefaultValue>{a}</DefaultValue>
      </Contents>
    </Template>
  </Templates>
  <Definitions>
    <AttDef Type="A">
      <ItemDefinitions>
        <String Name="s1">
          <Template Name="SimpleStingDefault">
            <Param Name="a">cat</Param>
          </Template>
        </String>
        <String Name="s2">
          <Template Name="SimpleStingDefault"/>
        </String>
      </ItemDefinitions>
    </AttDef>
  </Definitions>

See data/attribute/attribute_collection/TemplateTest.sbt and smtk/attribute/testing/cxx/unitTemplates.cxx for examples.  You can also read the discourse on the topic here: https://discourse.kitware.com/t/adding-parameterized-blocks-to-sbt-files/1013/4.
