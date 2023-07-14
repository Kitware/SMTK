ValueItem Validity now include enum applicability
-------------------------------------------------

isValid will now take into consideration if the enum it is set to is applicable with respects to the resource's set of active categories.

See attribute/testing/cxx/unitCategories.cxx for an example.
