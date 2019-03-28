## Add second model as an option when specifying new projects

The project manager was updated to accept a second geometry
file when initializing new projects. In this implementation,
only the primary geometry is linked to the simulation
attributes resource when the project is initialized.
(Resource links can be added or removed by application code,
of course.)
