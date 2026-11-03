#
# Shared library with the entities (libCDMgr)
#
lib_sources = files('Celebrity.cpp', 'CDType.cpp', 'Words.cpp', 'Genres.cpp', 'DB.cpp')
if with_films
    lib_sources += files('Film.cpp')
endif
if with_records
    lib_sources += files('Record.cpp')
endif

lib_deps = [glibmm_dep, ygp_dep]

libcdmgr = shared_library('CDMgr', lib_sources,
                          version: '0.0.0',
                          include_directories: config_inc,
                          dependencies: lib_deps,
                          install: true)
libcdmgr_dep = declare_dependency(link_with: libcdmgr, include_directories: config_inc, dependencies: lib_deps)

#
# The database backends; loaded at runtime by DBFactory
#
db_modules = []
foreach name, backend : db_backends
    db_modules += shared_module('cdmgr-' + name, backend[1],
                  name_prefix: '',
                  dependencies: [libcdmgr_dep, backend[0]],
                  gnu_symbol_visibility: 'hidden',
                  install_rpath: prefix / get_option('libdir'),
                  install: true,
                  install_dir: pkglibdir)
endforeach

#
# The GUI application
#
cdmanager_sources = files('NBPage.cpp', 'Storage.cpp', 'SaveCeleb.cpp', 'Settings.cpp', 'LangDlg.cpp', 'OOList.cpp',
                          'LangImg.cpp', 'WordDlg.cpp', 'Language.cpp', 'DBFactory.cpp', 'Statistics.cpp',
                          'CDManager.cpp', 'CDAppl.cpp')
if with_films
    cdmanager_sources += files('FilmList.cpp', 'StorageFilm.cpp', 'IMDbProgress.cpp', 'IMDbParser.cpp', 'ImportIMDb.cpp',
                               'PFilms.cpp', 'FilmData.cpp')
endif
if with_records
    cdmanager_sources += files('RecordList.cpp', 'SongList.cpp', 'StorageRecord.cpp', 'PRecords.cpp')
endif
if with_actors
    cdmanager_sources += files('StorageActor.cpp', 'ActorList.cpp', 'RelateFilm.cpp', 'PActors.cpp')
endif

# The icons are compiled in with #embed
icons = files('IconAuthor.gdkp', 'IconProgram.gdkp')

executable('CDManager', cdmanager_sources,
           cpp_args: '-DUSE_LANGUAGEPIXMAPS',
           extra_files: icons,
           dependencies: [libcdmgr_dep, gtkmm_dep, gthread_dep, xgp_dep, boost_dep, soup_dep, dl_dep],
           install_rpath: prefix / get_option('libdir'),
           install: true)

#
# The HTML exporter (started by CDManager)
#
executable('CDWriter', 'CDWriter.cpp', 'Writer.cpp', 'Language.cpp',
           dependencies: libcdmgr_dep,
           install_rpath: prefix / get_option('libdir'),
           install: true)

# Running from the build directory (meson devenv -C build) loads the modules built here
devenv = environment()
devenv.set('CDMGR_MODULEDIR', meson.current_build_dir())
meson.add_devenv(devenv)
