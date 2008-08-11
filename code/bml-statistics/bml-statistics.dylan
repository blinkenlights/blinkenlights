module: bml-statistics

define class <bml-statistic> (<object>)
  constant slot width :: <integer>, required-init-keyword: width:;
  constant slot height :: <integer>, required-init-keyword: height:;
  constant slot bits :: <integer>, required-init-keyword: bits:;
  slot max-value :: <integer>;
  slot total-time :: <integer> = 0;
  slot burning-time :: <array>;
  slot total-burning :: <array>;
  constant slot power :: <table> = make(<table>);
  constant slot power-consumption :: <table> = make(<table>);
end;

define variable *stat* :: false-or(<bml-statistic>) = #f;
define variable *bits* :: <integer> = 4;

define variable $compute-watts
  = method(x :: <integer>)
	x * ceiling/(150, *stat*.max-value - 1)
/*  = method (x :: <integer>)
      if (x < 5)
	0
      else
	150
    end */
    end;

define method initialize (stat :: <bml-statistic>,
			  #next next-method,
			  #key, #all-keys)
  next-method();
  stat.max-value := 2 ^ stat.bits;
  stat.burning-time
    := make(<array>,
	    dimensions: list(stat.height, stat.width, stat.max-value),
	    fill: 0);
  stat.total-burning
    := make(<array>, dimensions: list(stat.max-value), fill: 0);
  *stat* := stat;
end;

define method xml-element-handler-attributes (symbol == #"blm",
                                   message :: <element>,
                                   key == #"bml");
  let bits = string-to-integer(attribute-value(attribute(message, "bits")));
  if (bits > 4)
    error("Only <= 4 bits supported!");
  end;
  *bits* := bits;
  unless (*stat*)
    let width = string-to-integer(attribute-value(attribute(message, "width")));
    let height = string-to-integer(attribute-value(attribute(message, "height")));
    make(<bml-statistic>, width: width, height: height, bits: bits)
  end;
end;

define variable *frame-count* :: <integer> = 0;

define method xml-element-handler (symbol == #"frame",
                                   message :: <element>,
                                   key == #"bml");
  *frame-count* := *frame-count* + 1;
  if (modulo(*frame-count*, 50) == 0)
    format-out("."); force-output(*standard-output*);
  end;
  let duration
    = string-to-integer(attribute-value(attribute(message, "duration")));
  *stat*.power[*stat*.total-time]
    := make(<array>, dimensions: list(*stat*.height));
  record-rows(message, duration);
  *stat*.total-time := *stat*.total-time + duration;
end;

define method xml-element-handler (symbol == #"param",
                                   message :: <element>,
                                   key == #"playlist");
  if (attribute(message.element-parent, "type"))
    format-out("type is not empty, but %=\n",
	       attribute-value(attribute(message.element-parent, "type")));
  elseif (attribute-value(attribute(message, "key")) = "movie")
    block()
      gather-stats(attribute-value(attribute(message, "value")))
    exception (e :: <error>)
      format-out("received error during gathering statistics of %=: %=\n",
		 attribute-value(attribute(message, "value")), e);
    end
  else
    format-out("Skipping unknown key %=\n",
	       attribute-value(attribute(message, "key")));
  end
end;

define function record-rows (message :: <element>, duration :: <integer>)
  let current-row :: <integer> = 0;
  for (row in node-children(message))
    if (instance?(row, <element>))
      let watts = 0.0d0;
      for (column in text(row), i from 0)
	let f = make(<string>);
	f := add!(f, column);
	let value = string-to-integer(f, base: 16);
	if (*bits* ~== *stat*.bits)
	  value := lookup-bit-mask(*bits*, value);
	end;
	*stat*.burning-time[current-row, i, value]
	  := *stat*.burning-time[current-row, i, value] + duration;
	*stat*.total-burning[value] := *stat*.total-burning[value] + duration;
	watts := watts + $compute-watts(value);
      end;
      *stat*.power[*stat*.total-time][current-row] := watts;
      let w = ceiling(watts);
      let vec = begin
		  let res = element(*stat*.power-consumption, w, default: #f);
		  if (res)
		    res
		  else
		    let res2 = make(<vector>, size: *stat*.height, fill: 0);
		    *stat*.power-consumption[w] := res2;
		    res2
		  end;
		end;
      let tot = vec[current-row];
      *stat*.power-consumption[w][current-row]
	:= tot + as(<float>, duration) / 1000;
      current-row := current-row + 1;
    end;
  end;
end;

define function print-totals(out :: <stream>, filename)
  format(out, "%s contained %d frames with a total runtime of %d msec,\n",
	 filename, *frame-count*, *stat*.total-time);
  let (grand, perc) = compute-totals();
  format(out, "total average pixel-on-time %d msec (%d%%)\n",
	 ceiling/(grand, 100), ceiling(perc));
  let kwh = compute-kwh();
  format(out, "total watts consumed: %= kWh\n", kwh);
  //format(out, "average overall powerdraw %= kWh\n", kwh);
end;

define function compute-kwh () => (kwh)
  let grand-total = 0.0d0;
  for (x in *stat*.total-burning,
       i from 0)
    if (x > 0)
      grand-total
	:= grand-total + $compute-watts(i) * (x / 1000.0d0) //ms -> s
    end;
  end;
  grand-total / (3600.0d0 * 1000.0d0); //s -> h; W -> kW
end;

define function compute-totals (#key out :: false-or(<stream>))
 => (grand-total, percentage)
  let grand-total = 0.0d0;
  let factor = 100.0d0 / (*stat*.max-value - 1);
  for (x in *stat*.total-burning, i from 0)
    if (x > 0)
      if (out)
	format(out, "%= pixels [%d ms] %d greyscale\n",
	       as(<float>, x) / *stat*.total-time, x, i);
      end;
      grand-total := grand-total + i * factor * x
    end;
  end;
  let normalized-grand = grand-total / (*stat*.height * *stat*.width);
  values(normalized-grand,
	 normalized-grand / *stat*.total-time)
end;

define function print-details (out :: <stream>)
  format(out, "Detailed report:\n");
  for (i from 0 below *stat*.height)
    for (j from 0 below *stat*.width)
      for (k from 0 below *stat*.max-value)
	let v = *stat*.burning-time[i, j, k];
	unless (v == 0)
	  format(out, "%d.%d greyscale %d time %d (%= percent)\n",
		 i, j, k, v, (100 * as(<float>, v)) / *stat*.total-time);
	end;
      end;
    end;
  end;
end;

define function print-average-bml (stream :: <stream>)
  format(stream, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
	   "<blm width=\"%d\" height=\"%d\" bits=\"%d\" channels=\"1\">\n"
	   "  <header>\n"
	   "    <title>Average frame</title>\n"
	   "  </header>\n"
	   "  <frame duration=\"%d\">\n",
	 *stat*.width, *stat*.height, *stat*.bits,
	 *stat*.total-time);
  for (i from 0 below *stat*.height)
    format(stream, "    <row>");
    for (j from 0 below *stat*.width)
      let t = 0.0d0;
      for (k from 0 below *stat*.max-value)
	t := t + *stat*.burning-time[i, j, k] * k;
      end;
      format(stream, "%s",
	     integer-to-string(ceiling(t / *stat*.total-time), base: 16));
    end;
    format(stream, "</row>\n");
  end;
  format(stream, "  </frame>\n");
  format(stream, "</blm>\n");
end;

define function print-watt-usage (out :: <stream>) => ()
  for (arr in sort(key-sequence(*stat*.power)))
    let total = 0.0d0;
    let max = 0.0d0;
    format(out, "%s\t", pf(ceiling/(arr, 1000)));
    for (level in *stat*.power[arr], i from 5)
      if (level > max) max := level end;
      total := total + level;
      format(out, "%s\t", pf(level));
    end;
    format(out, "%s\t", pf(max));
    format(out, "%s\t", pf(ceiling/(total, *stat*.height)));
    format(out, "%s\n", pf(total));
  end
end;

define variable maxim = 0.0d0;

define function print-total-watts (out :: <stream>) => ()
  //level kw time
  for (w in sort(key-sequence(*stat*.power-consumption)))
//    format-out("now in watt %d\n", w);
    format(out, "%s\t", pf(w));
    let total = 0.0d0;
    maxim := 0.0d0;
    for (i from 0 below *stat*.height)
//      format-out("now in %d\n", i);
      let time = *stat*.power-consumption[w][i];
      total := total + time;
      if (time > maxim)
//	format-out("time is greater!\n");
	maxim := time;
      end;
      format(out, "%s\t", pf(time));
    end;
    format(out, "%s\t%s\t%s\n",
	   pf(maxim),
	   pf(total / as(<float>, *stat*.height)),
	   pf(total));
  end;
end;

define function print-total-watts3d (out :: <stream>) => ()
  //level kw time
  let lasta = 0.0d0;
  let lastx = make(<vector>, size: *stat*.height, fill: 0.0d0);
  for (w in sort(key-sequence(*stat*.power-consumption)))
    let total = 0.0d0;
    maxim := 0.0d0;
    for (i from 0 below *stat*.height)
      let time = *stat*.power-consumption[w][i];
      if (time > 0)
	lastx[i] := lastx[i] + time;
	format(out, "%s\t%d\t%s\n", pf(w), i, pf(lastx[i]));
      end;
      total := total + time;
      if (maxim < time)
	maxim := time;
      end;
    end;
    lasta := total / as(<float>, *stat*.height) + lasta;
    format(out, "#%s\t%s\t%s\t%s\t%s\n", pf(w),
	   pf(maxim), pf(lasta), pf(total), pf(total / as(<float>, *stat*.height)));
  end;
end;

define function pf (float)
  regex-replace(regex-replace(format-to-string("%d", float), "d", "e"),
		"s", "e");
end;

define function lookup-bit-mask (bits :: <integer>, value :: <integer>)
  => (new-value :: <integer>)
  block(ret)
    if (value == 0)
      ret(0);
    end;
    if (bits == 1)
      ret(15);
    end;
    if (bits == 2)
      ret(5 * value)
    end;
    if (bits == 3)
      if (value < 4)
	ret(value * 2)
      else
	ret((value * 2) + 1)
      end;
    end;
  end;
end;