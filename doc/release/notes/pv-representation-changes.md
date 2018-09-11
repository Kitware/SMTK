# ParaView Representation Changes

## User-facing changes

Highlighted and selected entities are no longer rendered as semi-transparent
overlays on top of the original geometry.
Instead, each visual block is rendered at most once.

## Developer-facing changes

Each requested render now rebuilds the list of renderable vtkDataObject
leaf-nodes in its multiblock inputs only when the input changes.
Also, the representation observes selection changes and updates a
timestamp used to determine whether block visibilities need to be
recomputed at the next render.
It is still up to the client application to force a re-render when the
selection changes.
