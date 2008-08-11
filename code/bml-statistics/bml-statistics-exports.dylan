module: dylan-user

define library bml-statistics
  use common-dylan;
  use io;
  use xml-parser;
  use system, import: { file-system };
  use command-line-parser;
  use regular-expressions;
end library;

define module bml-statistics
  use common-dylan, exclude: { format-to-string };
  use format-out;
  use standard-io;
  use streams;
  use format;
  use xml-parser;
  use xml-stream-parser;
  use file-system;
  use simple-xml;
  use command-line-parser;
  use regular-expressions;
end module;
