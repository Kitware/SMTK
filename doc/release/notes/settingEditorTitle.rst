Added Ability to Set Attribute Editor Panel's Title
----------------------------------------------------

The Attribute Editor Panel name can now be configure by a smtk::view::Configuration.

If the Configuration is Top Level then the following Configuration Attributes can be used:

* AttributePanelTitle - defines the base name of the Panel.  If not specified it defaults to Attribute Editor.
* IncludeResourceNameInPanel - if specified and set to true, the Panel's title will include the name of the resource in ()

SimpleAttribute.sbt contains an example:

.. code-block:: xml

  <Views>
    <View Type="Attribute" Title="External Expression Test Pt - Source" TopLevel="true" DisableTopButtons="false"
      AttributePanelTitle="SMTK Test" IncludeResourceNameInPanel="t">
      <AttributeTypes>
        <Att Type="B-expressions"/>
      </AttributeTypes>
    </View>
  </Views>


Developer changes
~~~~~~~~~~~~~~~~~~

* pqSMTKAttributePanel::updateTitle now takes in a const smtk::view::ConfigurationPtr&
