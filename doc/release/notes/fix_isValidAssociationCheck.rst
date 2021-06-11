Fix isValid Check w/r to an Attribute's Associations
====================================
If an attribute had the following conditions:

 * Its association item set to have its NumberOfRequiredValues > 0
 * Its Resource had active categories
 * All of its items where validity set with the exception of its association item

Then its isValid() would mistakenly return true, due to the fact that its association item (which does not have categories set), would be excluded from the check.

Now the association item is forced to be by turning off category checking for that item.  By doing this, we are saying that if the attribute itself passes its category checks, then so does its association item.
