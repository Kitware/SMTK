New qtComponentAttributeView Replacing qtModelEntityAttributeView
-----------------------------------------------------------------
This new class expands the capabilities of the old class by now supporting Components of any Resources not just Model Resources.

Also there is no longer a need to specifier the filter type.  The View will look at the association rule associated with the Attribute Definition specified in the view configuration.  If more than one Definition is specified, then the first one is chosen.

As a result the new constraints to using this View are:
* The Attribute Definition(s) in the configuration must contain the association rule to be used to determine which Resource Components need to be attributed
* If more than one Attribute Definition is listed, then the first one's association rule will be used.

How to refer to qtComponentAttributeViews in a ViewConfiguration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Setting the "Type" to ComponentAttributeView will cause the creation of a qtComponentAttributeView.
**Note** The "ModelEntity" Type will still be supported and will result in the creation of a qtComponentAttributeView.

.. code-block:: xml

  <Views>
    <View Type="ComponentAttribute" Title="Surface Properties">
      <AttributeTypes>
        <Att Type="SurfaceProperty" />
      </AttributeTypes>
    </View>
  </Views>
