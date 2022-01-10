Adding Property Support in Attribute XML Files
----------------------------------------------

You can now set Properties on the Attribute Resource and on an Attribute via an XML file.

Property Section for the Attribute Resource
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
This is an optional section describing a set of properties  that should be
added to the Attribute Resource.  The Property section is defined by a XML
**Properties** node which is composed of a set of children **Property** nodes as shown below:

.. code-block:: xml

  <Properties>
    <Property Name="pi" Type="Int"> 42 </Property>
    <Property Name="pd" Type="double"> 3.141 </Property>
    <Property Name="ps" Type="STRING">Test string</Property>
    <Property Name="pb" Type="bool"> YES </Property>
  </Properties>

You can also look at data/attribute/attribute_collection/propertiesExample.rst and smtk/attribute/testing/cxx/unitXmlReaderProperties.cxx for a sample XML file and test.

The following table shows the XML
Attributes that can be included in <Property> Element.

.. list-table:: XML Attributes for <Property> Element
   :widths: 10 40
   :header-rows: 1

   * - XML Attribute
     - Description

   * - Name
     - String value representing the name of the property to be set.

   * - Type
     - String value representing the type of the property to be set. **Note** that the value is case insensitive.


The values that the **Type** attribute can be set to are:

* int for an integer property
* double for a double property
* string for a string property
* bool for a boolean property

The node's value is the value of the property being set.

Supported Values for Boolean Properties
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The following are supported values for true:

* t
* true
* yes
* 1

The following are supported values for false:

* f
* false
* no
* 0

**Note** that boolean values are case insensitive and any surrounding white-space will be ignored.

Properties and Include Files
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
If you include a Attribute XML file that also assigns Resource Properties, the include file's Properties are assigned first.  Meaning that the file suing the include file can override the Properties set by the include file.

**Note** - the ability to unset a Property is currently not supported.

**Note** - Properties are currently not saved if you write out an Attribute Resource that contains properties in XML format.
