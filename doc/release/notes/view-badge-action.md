+ The badge action method API has changed to include a BadgeAction
  reference with each call and requires a boolean return value.
  You will need to update your Badge's action() method to return
  true only when you are able to cast the BadgeAction passed to
  your badge to a supported type. You should always support
  BadgeActionToggle.
+ This API change was made so that other actions and bulk actions
  could be supported. In particular, the qtResourceBrowser now
  invokes BadgeActionToggle on both mouse click and keypress (space
  bar) events, providing bulk phrase actuation.
+ In general, Since keypresses may occur without the mouse over any
  badge, some user interface components may pass an action to all
  badges in a BadgeSet (in order) until one responds by returning true.
