Improving qtAssociation2ColumnWidget's Operation Handler
--------------------------------------------------------

The original code would cause a large number of refreshes due to the fact it merely would see if the were any resource changes.  The new logic is a bit more intelligent and will only cause a refresh if any of the following conditions happen:

* A modified component is either the widget's attribute or can be associated with the widget's attribute
* A deleted component could have been associated with the widget's attribute
* A created component could be associated with the widget's attribute

The result should be a more responsive UI.
