Now uses smtk::attribute::Definition::isRelevant method
------------------------------------------------------

Removed qtAttributeViewInternals::removeAdvancedDefs since this logic is now part of the Definition's isRelevant method.
Similarly, qtAttributeViewInternals::getCurrentDefs has been simplified by using Definition's isRelevant method.

qtAttributeViewInternals::getCurrentDefs removed attribute resource parameter.  It was no longer needed.
