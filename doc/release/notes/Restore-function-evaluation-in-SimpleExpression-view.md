Restore function evaluation feature in SimpleExpression view

The function evaluation feature in SimpleExpression view relies on VTK.
However, also do we want to be able to preview this view with only Qt
enabled. In order to do so this MR creates a subclass of qtSimpleExpressionView
and it would override the SimpleExpression view constructor at runtime.
That being said now users can preview it with only Qt enabled and do
function evaluation when both Qt and VTK are enabled.
