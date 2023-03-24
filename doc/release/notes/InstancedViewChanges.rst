Adding ID Support to Instanced Views
------------------------------------

You can now refer to an Attribute in an Instanced View by its **ID**.  This will allow the View to refer to
the Attribute even if its Name is changed later on.

The View will add the **ID** View Configuration information if it doesn't already exist and will use it
in the future to find the Attribute.

If the **ID** is not specified, the view will continue require the **Name** configuration view attribute to be specified (along with the **Type** configuration view attribute if the named Attribute does not currently exists).
