use ExtUtils::MakeMaker;

WriteMakefile(
    'NAME'		=> 'Capi',
    'VERSION_FROM'	=> 'Capi.pm',
    'PREREQ_PM'		=> { Time::HiRes => 0,
                             IO::File    => 0,
                             IO::Select  => 0,
                             Fcntl       => 0,
                             overload    => 0 },
    ($] >= 5.005 ?
      (ABSTRACT_FROM => 'Capi.pm',
       AUTHOR        => 'Tobias Engel <perlcapi@tobias.org>') : ()),
    'LIBS'		=> ['-lcapi20 '],
    'DEFINE'		=> '-D__restrict= -D__const=const -DDONT_INST_STD',
    'INC'		=> '',
    'OBJECT'		=> '$(O_FILES) c20lib/libc20.a'
);

