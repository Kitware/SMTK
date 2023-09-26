Added Ability to Exclude Associations in Instance Views
-------------------------------------------------------

You can now prevent an Attribute's Associations from being displayed in an Instance View using **ExcludeAssocations**.
Note that this is not necessary if the Attribute's Definition does not have Associations specified

.. code-block:: xml

  <Views>
    <View Type="Instanced" Title="General">
      <InstancedAttributes>
        <Att Name="numerics-att" Type="numerics" ExcludeAssocations="true"/>
      </InstancedAttributes>
    </View>
  </Views>
