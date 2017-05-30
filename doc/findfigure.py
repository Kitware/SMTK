"""
  The findfigure Sphinx extension allows projects to
  specify a set of search paths that images may appear in.
  It is configured by two variables you can set in your
  project's conf.py file:

      findfigure_paths is a dictionary mapping builder
          names to a tuple of paths to search.

      findfigure_types is a dictionary mapping builder
          names to a tuple of figure filename extensions,
          in descending order of preference.

  See setup() below for more information.
"""

import os
import sys
from docutils import nodes, utils
from docutils.parsers.rst import directives, states
from docutils.parsers.rst.directives.images import Image, Figure
import sphinx.builders

rememberedApp = None


class FindImageDirective(Image):

    """A directive that finds images in a search path."""

    def run(self):
        """Process a new image."""
        env = self.state.document.settings.env
        reference = directives.uri(self.arguments[0])
        if not os.path.isabs(reference):
            # A relative path means we should search for the image
            # Find the builder-specific path-list to search:
            bname = rememberedApp.builder.format if rememberedApp != None else env.app.builder.name

            if bname in env.app.config.findfigure_paths:
                searchdirs = env.app.config.findfigure_paths[bname]
            elif '*' in env.app.config.findfigure_paths:
                searchdirs = env.app.config.findfigure_paths['*']
            else:
                searchdirs = (os.path.abspath('.'),)
            if reference.endswith('.*'):
                # Find the builder-specific list of extensions to try
                base, dummy = os.path.splitext(reference)
                if bname in env.app.config.findfigure_types:
                    searchexts = env.app.config.findfigure_types[bname]
                elif '*' in env.app.config.findfigure_types:
                    searchexts = env.app.config.findfigure_types['*']
                else:
                    searchexts = (
                        '.svg', '.pdf', '.png', '.jpeg', '.jpg', '.tiff', '.tif', '.gif')
            else:
                base = reference
                searchexts = ('',)
            # Now try finding the figure.
            foundit = False
            aref = base
            for ext in searchexts:
                for path in searchdirs:
                    try:
                        aref = os.path.join(path, base) + ext
                        # print '  TRY <%s>' % aref
                        status = os.stat(aref)  # Could check status bits here.
                        foundit = True
                        break
                    except:
                        foundit = False
                if foundit:
                    break
            if not foundit:
                # print 'MISSING FILE %s' % reference
                return []
            # print 'RESOLVED %s to %s' % (reference, aref)
            rewr = os.path.relpath(
                aref, os.path.join(env.srcdir, os.path.dirname(env.docname)))
            # NB: We must rewrite path relative to source directory
            #     because otherwise the output stage will be unable
            #     to find it.
            # print 'REWROTE %s to %s' % (aref, rewr)
            self.arguments[0] = rewr
        return Image.run(self)


class FindFigureDirective(Figure):

    """A directive that finds figure images in a search path."""

    def run(self):
        """Process a new figure."""
        env = self.state.document.settings.env
        reference = directives.uri(self.arguments[0])
        if not os.path.isabs(reference):
            # A relative path means we should search for the image
            # Find the builder-specific path-list to search:
            bname = rememberedApp.builder.format if rememberedApp != None else env.app.builder.name

            if bname in env.app.config.findfigure_paths:
                searchdirs = env.app.config.findfigure_paths[bname]
            elif '*' in env.app.config.findfigure_paths:
                searchdirs = env.app.config.findfigure_paths['*']
            else:
                searchdirs = (os.path.abspath('.'),)
            if reference.endswith('.*'):
                # Find the builder-specific list of extensions to try
                base, dummy = os.path.splitext(reference)
                if bname in env.app.config.findfigure_types:
                    searchexts = env.app.config.findfigure_types[bname]
                elif '*' in env.app.config.findfigure_types:
                    searchexts = env.app.config.findfigure_types['*']
                else:
                    searchexts = (
                        '.svg', '.pdf', '.png', '.jpeg', '.jpg', '.tiff', '.tif', '.gif')
            else:
                base = reference
                searchexts = ('',)
            # Now try finding the figure.
            foundit = False
            aref = base
            for ext in searchexts:
                for path in searchdirs:
                    try:
                        aref = os.path.join(path, base) + ext
                        # print '  TRY <%s>' % aref
                        status = os.stat(aref)  # Could check status bits here.
                        foundit = True
                        break
                    except:
                        foundit = False
                if foundit:
                    break
            if not foundit:
                # print 'MISSING FILE %s' % reference
                return []
            # print 'RESOLVED %s to %s' % (reference, aref)
            rewr = os.path.relpath(
                aref, os.path.join(env.srcdir, os.path.dirname(env.docname)))
            # NB: We must rewrite path relative to source directory
            #     because otherwise the output stage will be unable
            #     to find it.
            # print 'REWROTE %s to %s rel %s' % (aref, rewr,
            # os.path.abspath(os.path.dirname(env.docname)))
            self.arguments[0] = rewr
        return Figure.run(self)


def setup(app):
    """Read configuration for the module.

    This is mainly a list of paths to search and
    a dictionary of file extension preferences
    for each builder.

        findfigure_paths is a dictionary mapping builder
            names to a tuple of paths to search.

        findfigure_types is a dictionary mapping builder
            names to a tuple of figure filename extensions,
            in descending order of preference.

    Both settings above take the usual builder names
    (e.g., "html", "epub") as well as the special
    name "*", which is used as a fallback when the
    builder specified is not in the dictionary.

    Thus you may put figures for different builders into
    different search paths or you may simply use different
    files types for figures, but place them all in the same
    search paths (by only providing search paths for "*").

    By default the current directory is searched.
    The default figure extensions are different
    for each builder.

    An example is

    findfigure_paths = {
      '*': (
        '@CMAKE_CURRENT_SOURCE_DIR@',
        '@CMAKE_CURRENT_BINARY_DIR@')
    }
    findfigure_types = {
      'html': ('.svg','.png','.jpeg','.jpg', '.gif'),
      'epub': ('.svg','.png','.jpeg','.jpg', '.gif'),
      'latex':('.pdf','.png','.jpeg','.jpg')
    }, 'env')

    This example uses the same set of search paths for
    all builders but has the HTML and epub builders prefer
    SVG files over raster images while the LaTeX builder
    prefers PDF figures over raster images.
    """
    defaultpath = os.path.abspath('.')
    app.add_config_value('findfigure_paths',
                         {
                         '*': (defaultpath,)
                         }, 'env')
    app.add_config_value('findfigure_types',
                         {
                         'html': ('.svg', '.png', '.jpeg', '.jpg', '.tiff', '.tif', '.gif'),
                         'epub': ('.svg', '.png', '.jpeg', '.jpg', '.tiff', '.tif', '.gif'),
                         'latex': ('.pdf', '.png', '.jpeg', '.jpg', '.tiff', '.tif')
                         }, 'env')
    app.add_directive('findimage', FindImageDirective)
    app.add_directive('findfigure', FindFigureDirective)
    rememberedApp = app
