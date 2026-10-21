#
# Unit tests (meson test -C build)
#
boost_test_dep = dependency('boost', modules: ['unit_test_framework'], required: get_option('tests'))
if not boost_test_dep.found()
    subdir_done()
endif

test_args = ['-DBOOST_TEST_DYN_LINK', '-DTESTDATADIR="@0@/"'.format(meson.current_source_dir() / 'data'),
             '-DSRCDATADIR="@0@/"'.format(meson.project_source_root() / 'data')]
test_inc = include_directories('../src')
test_deps = [libcdmgr_dep, boost_test_dep]

tests = {
    'Words': [files('TestWords.cpp'), []],
    'Genres': [files('TestGenres.cpp'), []],
    'Database': [files('TestDB.cpp', '../src/DB.cpp',
                       db_backend == 'mysql' ? '../src/DBMySQL.cpp' : '../src/DBPostgres.cpp'), [db_dep]],
}
# The entity- and writer-tests use films and records
if with_films and with_records
    tests += {
        'Entities': [files('TestEntities.cpp'), [xgp_dep]],
        'Writer': [files('TestWriter.cpp', '../src/Writer.cpp'), [xgp_dep]],
    }
endif
if with_films
    tests += {'IMDbParser': [files('TestIMDbParser.cpp', '../src/IMDbParser.cpp'), [boost_dep]]}
endif

foreach name, test : tests
    test(name, executable('Test' + name, test[0], cpp_args: test_args, include_directories: test_inc,
                          dependencies: test_deps + test[1]))
endforeach
