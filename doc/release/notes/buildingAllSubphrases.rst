Descriptive Phrase Model Change in Subphrase Generation
--------------------------------------------------------

Previously, subphrases generation was done on demand as a performance optimization.  Unfortunately, this made certain potential functionality such as trinary visibility difficult to implement at best.  By forcing the Phrase Model to build all that it can in terms of its subphrases, it will now be possible to calculate and maintain a trinary visibility state.
