Graph Arcs now support inverse relationships
--------------------------

Arcs may implement inverse relationships via the `Inverse` handler to seamlessly
add/remove arcs as coupled pairs.

Developer changes
~~~~~~~~~~~~~~~~~~

Arc(s) Required APIs:
 * insert(ToType&,bool)
 * erase(ToType&,bool)
 * begin()
 * end()


* Note, insert and erase take a second boolean argument that is used by the
  `Inverse` handler to break recursive insertion/removal of inverses.

The default behavior of operations for each type of arc type when inserted as the inverse are as follows.

.. list-table:: Default Arc Type Inter-op
   :widths: 10 10 30 30 20
   :header-rows: 1

   * - Self Arc Type
     - Inverse Arc Type
     - Assign
     - Insert
     - Erase
   * - Arc
     - Arc
     - Overwrite self, remove current inverse, insert new inverse.
     - Insert inverse if valid to insert self, insert self if inverse successfully inserted.
     - Unset self and erase inverse.
   * - Arc
     - Arcs
     - Overwrite self, remove current inverses, insert new inverse.
     - Insert inverse if valid to insert self, insert self if inverse successfully inserted.
     - Unset self and erase inverse.
   * - Arcs
     - Arc
     - Overwrite self, remove current inverses, insert new inverses.
     - Insert self, if successful insert inverse, if inverse failed remove self and report failure
     - Erase inverse and self.
   * - Arcs
     - Arcs
     - Overwrite self, remove current inverses, insert new inverses
     - Insert self, if successful insert inverse, if inverse failed remove self and report failure.
     - Erase inverse and self.
   * - OrderedArcs
     - Arc
     - Overwrite self, remove current inverses, insert new inverses.
     - Insert inverse, if successful insert self.
     - Erase inverse and self.
   * - OrderedArcs
     - Arcs
     - Overwrite self, remove current inverses, insert new inverses.
     - Insert inverse, if successful insert self.
     - Erase inverse and self.
   * - Arc or Arcs
     - OrderedArcs
     - Throw `std::logic_error` exception.
     - Throw `std::logic_error` exception.
     - Erase self and first matching inverse.
   * - OrderedArcs
     - OrderedArcs
     - Won't compile.
     - Won't compile.
     - Won't compile.

All behaviors can be overwritten by providing a specialization of
`smtk::graph::Inverse`.

User-facing changes
~~~~~~~~~~~~~~~~~~~

The user should not see any major changes. Management of arc types with an
inverse is now handled automatically so there may be some improvements to
arc consistency.
