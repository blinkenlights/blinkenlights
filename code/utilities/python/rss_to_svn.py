#!/usr/bin/python

import feedparser
import urllib
import os

d = feedparser.parse("http://blinkenlights.net/stereoscope/movies/submissions/feed")

for entry in d['entries']:
	date = entry.updated_parsed
	dirname = "%d/%d/%d" % (date[0], date[1], date[2])
	filename = "%s/%s" % (dirname, os.path.basename(entry.id))

	if not os.path.isdir(dirname):
		os.makedirs(dirname)

	if not os.path.isfile(filename):
		urllib.urlretrieve (entry.id, filename)
		print filename

