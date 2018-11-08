------------------------
SMTK's Simulation System
------------------------

Once you have
a model or mesh and
an attribute resource describing a simulation,
you are ready to create an input deck for the simulation.

One option is to have your simulation link directly to SMTK,
load the saved information from SMTK's native file formats,
and use SMTK to initialize the simulation.

Often this is not feasible, and so SMTK provides stubs
for writing a file in the format a simulation expects,
commonly known as an *input deck* [#f1]_.
We expect you will use python to write the input deck as
it is much simpler and easier to maintain a python script
than a large C++ code base to write what is usually a flat
text file.

The :smtk:`smtk::simulation::ExportSpec` class aggregates
all of the information you should need to write the input deck:

* an attribute resource holding simulation parameters
* an attribute resource holding locations of files involved in the export
* an object that provides methods for querying the analysis grid (be it a model or mesh).

Your python export script is expected to take a single argument (an
instance of ExportSpec) and write whatever files are needed to run the simulation.

.. [#f1] This term is historical, as many of the first simulations
         were run on computers that used punch cards for offline
         storage; a deck of punch cards holding simulation parameters
         was often kept as a way to reproduce results for a simulation
         run used in a report or journal article.
