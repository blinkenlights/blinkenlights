SUBDIRS = src . build test

jar: compile
	mkdir -p build/dist
	(cd build/classes; zip -R ../dist/blinken.jar '*.class')

javadoc:
	mkdir -p build/doc
	$(JDK)/bin/javadoc -d  build/doc src/java/de/blinkenlights/blinkenlet/*.java


compile:
	mkdir -p build/classes
	CLASSPATH=$(COMPILECP) $(JAVAC) $(shell find src/java -name "*.java")

all-local: jar

clean-local:
	rm -rf build/doc

really-clean: maintainer-clean
	-rm configure aclocal.m4 COPYING INSTALL install-sh ltmain.sh missing mkinstalldirs conftest* config.*
	find . -name Makefile.in | xargs rm
