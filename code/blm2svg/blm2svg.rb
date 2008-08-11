# convert BLM to SVG file

magic = gets;

if ( magic !~ /# BlinkenLights Movie \d+x\d+/ )
	print "Invalid Blinkenlights Movie\n";
	exit 1;
end

re = /(\d+):(\d+)/ # match a time hh:mm
result = /(\d+)x(\d+)/.match(magic)

size_x = result[1].to_i;
size_y = result[2].to_i;

matrix = Array:new;

frame_index = -1;

while ( _line = gets)
	if ( _line =~ /^#/ or _line =~ /^$/ )
		next;
	end;

	if ( _line =~ /^@\d+$/ )
		frame_index = frame_index + 1;
		matrix[frame_index] = Array:new;
		line_index = 0;
		next;
	end

	matrix[frame_index][line_index] = _line.split(//);;
	line_index = line_index + 1;
end

# output SVG

# prolog

print "<?xml version=\"1.0\"?>\n"
print "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.0//EN\" \"http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd\">\n"
print "<svg width=\"100%\" height=\"100%\" viewBox=\"0 0 18 14\">\n"

print "\t<defs>\n";
print "\t\t<g id=\"pixel\">\n";
print "\t\t\t<rect x=\"0.15\" y=\"0.3\" width=\"0.7\" height=\"1\" rx=\"0.1\" ry=\"0.1\" style=\"fill: black; stroke: none\"/>\n";
print "\t\t</g>\n"
print "\t</defs>\n"


line = 0;
while line < size_y
	column = 0;
	while column < size_x
		pixel = matrix[0][line][column];
		if ( pixel == "1" )
			printf "\t<use xlink:href=\"\#pixel\" x=\"%d\" y=\"%f\"/>\n", column, (Float(line) * 1.75);
		end
		column = column + 1;
	end
	line = line + 1;
end

print "</svg>\n"
