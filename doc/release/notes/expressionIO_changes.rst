Changing Expression Format
==========================
JSON will now store a ValueItem's Expression in ComponentItem format using the key "ExpressionReference" instead of 2 keys called "Expression" and "ExpressionName".  This no only simplifies things format wise but will also support expressions stored in different resources.

**Note** The older format is still supported so this change is backward compatible.
**Note** The XML format is still using the older style.
