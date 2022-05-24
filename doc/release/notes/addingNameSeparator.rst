Allowing Designers to Set the Default Name Separator for Attribute Resources
----------------------------------------------------------------------------

Added an API to set, get and reset the Default Name Separator used by the Attribute Resource when creating unique names
for a new Attribute.  Also added support in Version 5 XML Attribute Files as well as in JSON representations to save and
retrieve the separator.

New C++ API Changes for smtk::attribute::Resource
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: c++

  /**\brief Get the separator used for new Attributes whose names are not unique
   */
  const std::string &defaultNameSeparator();
  /**\brief Reset the separator used for new Attributes whose names are not unique to to the default which is '-'.
   */
  void resetDefaultNameSeparator();
  /**\brief Set the separator used for new Attributes whose names are not unique
   */
  bool setDefaultNameSeparator(const std::string& separator);

XML Formatting Additions
~~~~~~~~~~~~~~~~~~~~~~~~

This example sets the Name Separator to **::**

.. code-block:: xml

<SMTK_AttributeResource Version="5" ID="504c3ea1-0aa4-459f-8267-2ba973d786ad" NameSeparator="::">
</SMTK_AttributeResource>
