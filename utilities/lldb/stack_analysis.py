"""
This module is meant to be used outside lldb to analyze the
stack traces dumped to file every hardware watchpoint by the
useCountTrace command.

For the purposes of this documentation, we'll assume you are
dumping to /tmp/trace.txt.

## Simple plots during a run

To get some statistics on /tmp/trace.txt while the program is running:

   grep  -B1 -- "^----" /tmp/trace.txt  | grep -v -- "--" | sed -e 's/frame #//g' -e 's/:.*//g' > /tmp/stackdepth
   grep -- "^----" /tmp/trace.txt | sed -e 's/.* //g' > /tmp/usecount
   (cd /tmp; gnuplot)

and then in gnuplot, run

   plot 'usecount' w l, 'stackdepth' w l

## Run post-processing

To load the stack trace data:

    from stack_analysis import analyzer
    stack = analyzer('/tmp/trace.txt')

    # Get raw stack frame information
    if len(stack.traces) > 0:
      ff = stack.traces[0]

    # Print a stack frame
    stack.print_trace(0)

    # Compute the portion of two stack traces that
    # is shared by both of them.
    depth = stack.in_common(0, 1)

    # Find the range of stack traces containing the
    # given text in at least one frame:
    frameLo, frameHi, countAtFrameLo, countAtFrameHi = \
        stack.trace_range('smtk::resource::Manager::add')

    # Get the common stack depth between consecutive traces for the
    # entire dataset of all backtraces:
    depths = stack.consecutive_common_frames()

    # Print out ranges of traces that share a common backtrace
    stack.print_common()
"""

import itertools


class analyzer:

    """Read and analyze a collection of backtraces."""

    def __init__(self, filename):
        ff = open(filename, 'r')
        tl = ff.readlines()
        ff.close()
        self.tl = tl
        traces = [(ii, int(tl[ii][4:].strip()))
                  for ii in range(len(tl)) if tl[ii][0:4] == '----']
        self.traces = traces
        for ii in range(len(traces)):
            traces[ii] = (
                traces[ii][0], traces[ii][1],
                traces[ii + 1][0] - traces[ii][0] - 3
                if ii < len(traces) - 1
                else len(tl) - traces[ii][0] - 3)
        print('Read %d lines, %d traces' % (len(tl), len(traces)))

    def trace_lcd(self, fa, fb):
        """Compute the "least common divisor" between two stack traces."""
        la, ca, sa = self.traces[fa]
        lb, cb, sb = self.traces[fb]
        maxDepth = min(sa, sb)
        for ii in range(maxDepth):
            entrya = self.tl[la + sa + 2 - ii]
            entryb = self.tl[lb + sb + 2 - ii]
            ia = entrya.find(':')
            ib = entryb.find(':')
            if entrya[ia:] == entryb[ib:]:
                continue
            maxDepth = ii
            break
        return maxDepth

    def trace_range(self, txt):
        """Return the range of backtraces containing the given text.

        This returns a tuple holding the range of backtraces as well
        as the starting and ending use-counter (in that order).
        """
        # First, find the min/max log-line matches
        lmin = min([ii for ii in range(len(self.tl)) if txt in self.tl[ii]])
        lmax = max([ii for ii in range(len(self.tl)) if txt in self.tl[ii]])
        # Now find the traces that hold those log lines
        fmin = max(
            [ii for ii in range(len(self.traces)) if self.traces[ii][0] < lmin])
        fmax = min([ii for ii in range(len(self.traces)) if (
            self.traces[ii][0] + self.traces[ii][2] + 2) >= lmax])
        return (fmin, fmax, self.traces[fmin][1], self.traces[fmax][1])

    def trace(self, tracenum):
        """ Fetch the backtrace log entries given its index."""
        la, ca, sa = self.traces[tracenum]
        return self.tl[(la + 1):(la + sa + 2)]

    def print_trace(self, tracenum):
        """Print a backtrace given its index."""
        la, ca, sa = self.traces[tracenum]
        print(''.join(self.tl[(la + 1):(la + sa + 2)]).strip())
        print('   use-count: %d' % ca)

    def consecutive_common_frames(self):
        return [self.trace_lcd(ii - 1, ii) for ii in range(1, len(self.traces) - 1)]

    def print_common(self):
        """Print stack frames common across consecutive backtraces."""
        csd = self.consecutive_common_frames()

        match = []
        ff = 0
        root = ff
        rootdepth = csd[0]
        la, ca, sa = self.traces[ff]
        entrya = self.tl[la + sa + 2 - rootdepth + 1]
        ia = entrya.find(':')
        print('New root at ', ff, entrya[ia:-1])

        for depth in csd:
            ff += 1
            lb, cb, sb = self.traces[ff]
            entryb = self.tl[lb + sb + 2 - depth + 1]
            ib = entryb.find(':')
            if rootdepth != depth or entrya[ia:] != entryb[ib:]:
                print('    Delta ', self.traces[
                      root][1], self.traces[ff - 1][1])
                print('New root at ', ff, entryb[ib:-1])
                rootdepth = depth
                root = ff
                la, ca, sa, entrya, ia = lb, cb, sb, entryb, ib
            match.append(root)
        print('    Delta ', self.traces[root][1], self.traces[ff - 1][1])
