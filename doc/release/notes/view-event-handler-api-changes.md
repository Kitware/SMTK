# PhraseModel::handle*() without needing a ComponentItem

smtk::view::PhraseModel and its subclasses stay up to date with Resources in
SMTK by observing completed Resource events and Operations. Previously, the
methods, handleCreated(), handleModified(), and handleExpunged() required a
ComponentItem in order to extract Components which had been created, modified,
and expunged, respectively. The need for a ComponentItem has been removed by
allowing these methods to accept a smtk::resource::PersistentObjectSet. The
intention is to enable subclasses of PhraseModel to observe events of their
own choosing which may not yield a ComponentItem as Operation::Result does.

## Developer changes

Developers must now extract the created, modified, and expunged Components in
overrides of PhraseModel::handleOperationEvent() when observing Operations.
