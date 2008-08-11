module: bml-statistics

define argument-parser <bml-statistics-argument-parser> ()
  synopsis print-synopsis,
    usage: "bml-statistics [options]",
    description: "Gather statistics of a bml file.";
  option verbose?, "Verbose output, print detailed statistic",
    short: "v", long: "verbose";
  option average?, "Generate average bml",
    short: "a", long: "average";
  option output-to-file?, "Save statistics in a file",
    short: "o", long: "output-to-file";
  option data-file?, "Save power consumption in a data",
    short: "d", long: "data-file";
  option filename, "File to gather statistics from",
    kind: <parameter-option-parser>, long: "filename", short: "f";
  option playlist, "Playlist to gather statistics from",
    kind: <parameter-option-parser>, long: "playlist", short: "p";
end;

define function main()
  let parser = make(<bml-statistics-argument-parser>);
  unless(parse-arguments(parser, application-arguments()))
    print-synopsis(parser, *standard-output*);
    exit-application(0);
  end;
  let file = parser.filename;
  if (parser.playlist & file-exists?(parser.playlist))
    //XXX: currently, some parts are hard coded
    // (width, height, bits)
    file := parser.playlist;
    make(<bml-statistic>, width: 26, height: 20, bits: 4);
    gather-playlist-stats(parser.playlist);
  elseif (parser.filename & file-exists?(parser.filename))
    gather-stats(parser.filename);
  else
    print-synopsis(parser, *standard-output*);
    exit-application(0);
  end;

  let base = split(file, ".")[0];
  block ()
    let output
      = if (parser.output-to-file?)
	  let out = concatenate(base, ".stat");
	  make(<file-stream>,
	       locator: out,
	       direction: #"output",
	       if-exists: #"replace");
	else
	  *standard-output*;
	end;
    
    print-totals(output, file);
    if (parser.verbose?)
      print-details(output);
    end;
    close(output);
  exception (e :: <error>)
    format(*standard-error*, "%=", e);
  end;
    
  if (parser.average?)
    with-open-file (stream = concatenate(base, ".average.bml"),
		    direction: #"output",
		    if-exists: #"replace")
      print-average-bml(stream);
    end;
  end;
  if (parser.data-file?)
    with-open-file (stream = concatenate(base, ".data"),
		    direction: #"output",
		    if-exists: #"replace")
      print-watt-usage(stream);
    end;
    with-open-file (stream = concatenate(base, ".power.data"),
		    direction: #"output",
		    if-exists: #"replace")
      print-total-watts3d(stream);
    end;
  end;
end;

define function gather-stats (filename :: <string>)
  with-open-file (file-stream = filename, direction: #"input")
    let stream-parser = make(<xml-stream-parser>, stream: file-stream);
    parser-loop(stream-parser, #"bml");
  end;
end function;

define function gather-playlist-stats(playlist :: <string>)
  with-open-file (file-stream = playlist, direction: #"input")
    let stream-parser = make(<xml-stream-parser>, stream: file-stream);
    parser-loop(stream-parser, #"playlist");
  end;
end;

// Invoke our main() function.
main();
