# Badges for descriptive phrases

This work adds support for multiple arbitary, run-time-configurable badges
to descriptive phrases. There is also supporting work that refactors and
simplifies how PhraseModel subclasses are constructed and configured.

+ Add Badge, BadgeSet, and BadgeFactory classes.
+ Refactor PhraseModel registration in view::Manager into PhraseModelFactory
  using the new smtk::common::Factory template.
+ Refactor PhraseModel construction so that
    + constructors, not static `create` methods, are used; and
    + constructors take and use a view::Configuration object that they
      use to prepare themselves and objects they own (namely, BadgeSet
      and (indirectly) SubphraseGenerator).
+ Add a badge (ObjectIconBadge) showing icons based on the phrase's subject.
  This uses the view-manager's icon factory.
+ Add a badge (AssociationBadge) showing an exclamation mark when
  matching persistent objects are not associated to attributes with a
  given set of definitions.
