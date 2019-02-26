#Track attribute resource id, associations in xml I/O

In order for an attribute resource to be reliably written/read to/from
XML, its ID is now stored in its generated .sbi file. Additionally, an
attribute's associations are stored in XML with enough information to
recreate their underlying links.
