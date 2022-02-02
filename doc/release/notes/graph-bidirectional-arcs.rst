Graph Arcs now support bidirectional relationships
--------------------------

Arcs may be tagged as bidirectional with another type of Arc. Arcs can use the
BidirectionalArcWrapper class to handle seamlessly adding/removing arcs as coupled
pairs in conjunction with this tag.

Developer changes
~~~~~~~~~~~~~~~~~~

The stored ToType of an arc is now InverseArcType<InverseArc>::ArcWrapper

*** static assert FromType == InverseArc::ToType && ToType == InverseArc::FromType in Inverse<!void>

Arc(s) API:
 insert(pos, value) -> std::pair<iterator,bool>
 insert(value) -> std::pair<iterator,bool>
 erase(value) // erase first in ordered
 operator=(xx) // assign from iteratable container, Arc assign from scalar

Arc 	Arc 	One-to-one in both directions
 * insert:
   * no insert, it == end on non-null content
   * succeed on null/new content
 * assign:
   * overwrite existing content and remove inverse if not same

Arc 	Arcs 	One-to-many + many-to-one
 * insert Arc:
   * mirror Arc:Arc
 * insert Arcs:
   * If Arc.to() == Arcs.from() || Arc.to().expired() Ok
   * Else no insert, it == end
 * assign Arc:
   * Inserts into Arcs
 * assign Arcs:
   * remove old, Iterator and overwrite

Arcs 	Arcs 	One-to-many in both directions
 * insert
   * return if not inserted
 * assign
   * remove old, iterator and insert new

OrderedArcs 	Arc 	One-to-ordered + one-to-one
 * insert Arc:
   * Not allowed
 * insert OrderedArc:
   * Okay if Arc.to() == OrderedArc.from() || new
 * assign Arc
   * Not allowed
 * assign Arcs
   * remove old, Iterate and overwrite

OrderedArcs 	Arcs 	One-to-ordered + one-to-many
 * insert Arcs
   * Not allowed
 * insert Ordered Arcs
   * iterate and inserts Arcs
 * assign Arcs
   * Not allowed
 * assign
   * remove old, iterator and insert new

OrderedArcs 	OrderedArcs 	One-to-ordered in both directions
 Requires implementation by user
 * insert:
   * Not allowed
 * assign:
   * Not allowed

ArcTraits
  bool ordered;
  bool iteratable;


User-facing changes
~~~~~~~~~~~~~~~~~~~

Describe changes to new and existing users of SMTK applications here.
Do not assume users know obscure terminology or are already familiar with SMTK.
Feel free to include screenshots to clarify user interface changes.
