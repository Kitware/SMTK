Added Definition::isRevelvant Method
------------------------------------

This method will return true if the Definition is considered relevant. If includeCategories is true then
the attribute resource's active categories must pass the Definition's category check.
If includeReadAccess is true then the Definition's advance level < the read access level provided.

Expanded attribute's Signal Operation to include category changes
-----------------------------------------------------------------
The Signal operation now includes a resource item called **categoriesModified** both as a parameter and as a result.
If set, it indicates the attribute resources whose categories have been modified.

Change to Attribute::isRelevant Method
--------------------------------------------------
The parameter readAccessLevel has been changed from int to unsigned int to match the signature of Item::isRelevant.
