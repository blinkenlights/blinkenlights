#!/bin/sh

echo

test -f test-movie-parser.c || {
    echo "You must run this script in the tests directory."
    echo
    exit 1
}

if xmllint --version > /dev/null 2>&1; then
    movie_dtd="../data/bml.dtd"
fi

if test -x test-movie-parser; then
    echo "Testing the movie parser ..."
    for suffix in blm bml gif; do
        for movie in movies/*.$suffix; do
	    test -f $movie && ./test-movie-parser $movie
	done
    done
    echo
fi

if test -x test-movie-writer; then
    echo "Testing the movie writer ..."
    for suffix in blm bml gif; do
        for movie in movies/*.$suffix; do
            if test -f $movie; then
		for format in blm bml gif; do
		    if test $format = bml && test -n $movie_dtd; then
			./test-movie-writer $format $movie | \
			    xmllint --noout --dtdvalid $movie_dtd - && \
			    echo "  ... and validated against $movie_dtd"
		    else
			./test-movie-writer $format $movie > /dev/null
		    fi
		done
	    fi
	done
    done
    echo
fi

exit 0
