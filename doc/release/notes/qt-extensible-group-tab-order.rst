Tab order in extensible group items
------------------------------------

Extensible group items are displayed in a Qt table widget instance with
one row for each subgroup. Previously, the table widget would not
respond to tab key input (typically used for navigating between cells).
This has been corrected for most mainstream cases: double items,
integer items, string items, and reference items (dropdown box format).
Tab-ordering also works with the discrete and expression versions of
value items.

There are some cases that have not yet been implemented. The overall
tab order might not be consistently preserved between the table widget
and other items before and after it in the attribute editor. For optional
items, the checkbox is not reliably included in the tab order. The tab
behavior is also not defined for discrete items that include conditional
children.
