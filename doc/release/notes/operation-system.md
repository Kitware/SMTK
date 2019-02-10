## Operation system changes

### Configuration

Operations now have a method named `configure()`.
This method may be invoked by a user interface when
the operation's associations are changed, item values are
changed, or new attributes are created (in the operation's
resource) in order for the operation to edit itself
for consistency and to provide context-sensitive default
values for items.

An example is provided in the oscillator-session's EditSource
operation that uses changes to associations to compute a default
center and radius for the source.
