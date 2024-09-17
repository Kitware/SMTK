View Changes
============

Added view::Configuration::Component::attributeAsString method
--------------------------------------------------------------

Added a method that returns the string value of a component's attribute.
If the attribute doesn't exist an empty string is returned.

Expandable Multi-line String Items
----------------------------------

You can now indicate that you want a Multi-line StringItem to have an expanding size policy
for its height by using the new **ExpandInY** option.  Here is an example:

.. code-block:: xml

  <Views>
    <View Type="Instanced" Title="StringItemTest" Label="Simple String Item Test" TopLevel="true">
      <InstancedAttributes>
        <Att Type="A" Name="Test Attribute">
          <ItemViews>
            <View Item="s" ExpandInY="true"/>
          </ItemViews>
        </Att>
      </InstancedAttributes>
    </View>
  </Views>

Please see data/attribute/attribute_collection/StringItemExample.sbt for a complete example.
