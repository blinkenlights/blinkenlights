module: bml-statistics

define generic xml-element-handler (name :: <symbol>, element :: false-or(<element>), user-data);

define generic xml-element-handler-attributes (name :: <symbol>, element :: false-or(<element>), user-data);

define class <no-handler-exception> (<error>)
end;

define method xml-element-handler (name :: <symbol>, element :: false-or(<element>), user-data)
  signal(make(<no-handler-exception>))
end;

define method xml-element-handler-attributes (name :: <symbol>, element :: false-or(<element>), user-data);
  signal(make(<no-handler-exception>))
end;
  


define method parser-loop (parser :: <xml-stream-parser>,
                           user-data) => ()
  let current-element = #f;
  monitor(parser,
          #"start-element",
          method (event-name, event-attributes)
            //format-out("START %= [%=]\n", event-name, event-attributes);
            let element = make(<element>, name: event-name);
            for (attribute in event-attributes)
              add-attribute(element, attribute);
            end;
            if (current-element)
              add-element(current-element, element);
            end;
            current-element := element;
	    block()
	      xml-element-handler-attributes (as(<symbol>, event-name),
					      current-element,
					      user-data);
	    exception (e :: <no-handler-exception>)
	    end;
          end);

  monitor(parser,
          #"end-element",
          method (event-name)
            //format-out("END %= [%=]\n", event-name, current-element);
            block()
              xml-element-handler(as(<symbol>, event-name),
				  current-element,
				  user-data);
              current-element :=
		if (current-element & current-element.element-parent)
		  current-element.element-parent
		end;
            exception(e :: <no-handler-exception>)
              if (current-element & current-element.element-parent)
                current-element := current-element.element-parent;
              end;
            end;
          end);
  
  monitor(parser,
          #"characters",
          method (chars)
            //format-out("CHARS %=\n", chars);
            if (current-element & ~ every?(method(x) x = '\n' | x = ' ' end, chars))
              current-element.node-children
                := concatenate(current-element.node-children,
                               vector(make(<char-string>,
                                           text: chars)));
            end;
          end);
  parse(parser);
end;
