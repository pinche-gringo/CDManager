project('CDManager', 'cpp',
        version: '1.2.00',
        license: 'GPL-3.0-or-later',
        meson_version: '>= 1.4.0',
        default_options: ['cpp_std=c++26', 'warning_level=1', 'buildtype=debugoptimized'])

cxx = meson.get_compiler('cpp')
fs = import('fs')

version = meson.project_version()
release = '.'.join([version.split('.')[0], version.split('.')[1]])  # 1.2.00 -> 1.2

prefix = get_option('prefix')
datadir = get_option('datadir')
pkgdatadir = datadir / meson.project_name()
docdir = datadir / 'doc' / meson.project_name()
localedir = get_option('localedir')

#
# C++26 features used throughout the sources: reflection and contracts
#
foreach arg : ['-freflection', '-fcontracts']
    if not cxx.has_multi_arguments('-std=c++26', arg)
        error('The compiler doesn\'t support @0@ (C++26); GCC >= 16 is needed'.format(arg))
    endif
endforeach
add_project_arguments('-freflection', '-fcontracts', language: 'cpp')
# Links the contract violation handler (libstdc++exp) if contracts are evaluated
add_project_link_arguments('-fcontracts', language: 'cpp')

# Contracts (pre/post/contract_assert) are only enforced if checks are enabled
checks = get_option('checks')
if checks >= 0
    add_project_arguments('-DCHECK=@0@'.format(checks), '-fcontract-evaluation-semantic=enforce', language: 'cpp')
else
    add_project_arguments('-fcontract-evaluation-semantic=ignore', language: 'cpp')
endif
if get_option('trace') >= 0
    add_project_arguments('-DTRACELEVEL=@0@'.format(get_option('trace')), language: 'cpp')
endif

#
# Dependencies
#
glibmm_dep = dependency('glibmm-2.68', version: '>= 2.68.0')
ygp_dep = dependency('YGP', version: '>= 0.9.42')
gtkmm_dep = dependency('gtkmm-4.0', version: '>= 4.0.0')
gthread_dep = dependency('gthread-2.0', version: '> 2.2.0')
xgp_dep = dependency('XGP', version: '>= 0.9.50')
boost_dep = dependency('boost', modules: ['json'])
soup_dep = dependency('libsoup-3.0', version: '>= 3.0.0')
if not cxx.has_header('sys/shm.h')
    error('Needed header sys/shm.h not found!')
endif

#
# Database backend: MySQL (libmysqlclient) or PostgreSQL (libpq)
#
db_backend = get_option('database')
db_dep = disabler()
if db_backend in ['auto', 'mysql']
    db_dep = dependency('mysqlclient', required: false)
    if not db_dep.found()
        db_dep = dependency('libmariadb', required: false)
    endif
    if not db_dep.found()
        # Fall back to the classical installation locations
        mysql_lib = cxx.find_library('mysqlclient', required: false,
                                     dirs: ['/usr/lib64/mysql', '/usr/lib/mysql', '/usr/local/lib/mysql',
                                            '/usr/local/mysql/lib', '/opt/mysql/lib'])
        foreach incdir : ['/usr/include/mysql', '/usr/local/include/mysql', '/usr/local/mysql/include/mysql',
                          '/opt/mysql/include/mysql']
            if mysql_lib.found() and not db_dep.found() and fs.exists(incdir / 'mysql.h')
                db_dep = declare_dependency(dependencies: mysql_lib, compile_args: '-I' + incdir)
            endif
        endforeach
    endif
    if db_dep.found()
        db_backend = 'mysql'
    elif db_backend == 'mysql'
        error('MySQL requested, but its library/headers weren\'t found')
    endif
endif
if db_backend in ['auto', 'postgresql']
    db_dep = dependency('libpq', required: false)
    if not db_dep.found()
        pg_config = find_program('pg_config', required: false)
        if pg_config.found()
            pq_incdir = run_command(pg_config, '--includedir', check: true).stdout().strip()
            pq_libdir = run_command(pg_config, '--libdir', check: true).stdout().strip()
            db_dep = declare_dependency(dependencies: cxx.find_library('pq', dirs: pq_libdir),
                                        compile_args: '-I' + pq_incdir)
        endif
    endif
    if db_dep.found() and cxx.has_function('PQconnectdb', prefix: '#include <libpq-fe.h>', dependencies: db_dep)
        db_backend = 'postgresql'
    elif db_backend == 'postgresql'
        error('PostgreSQL requested, but libpq wasn\'t found')
    else
        error('No supported database found! Install the development files of either MySQL (libmysqlclient) or PostgreSQL (libpq)')
    endif
endif

#
# Features
#
with_films = get_option('films')
with_actors = get_option('actors')
if with_actors and not with_films
    warning('Handling actors needs films enabled! Enabling both ...')
    with_films = true
endif
with_records = get_option('records')
if not (with_actors or with_films or with_records)
    error('Disabling all features makes the program quite useless, innit?')
endif

#
# Translations
#
linguas = fs.read('po' / 'LINGUAS').strip().split()
nls = cxx.has_function('gettext', prefix: '#include <libintl.h>', required: get_option('nls'))

#
# Configuration header
#
conf = configuration_data()
conf.set_quoted('PACKAGE', meson.project_name())
conf.set_quoted('PACKAGE_NAME', meson.project_name())
conf.set_quoted('PACKAGE_VERSION', version)
conf.set_quoted('PACKAGE_BUGREPORT', 'g17m0@users.sourceforge.net')
conf.set_quoted('VERSION', version)
conf.set_quoted('PRG_RELEASE', release)
conf.set_quoted('LOCALEDIR', prefix / localedir)
conf.set_quoted('DOCUDIR', prefix / docdir + '/')
conf.set_quoted('DATADIR', prefix / pkgdatadir + '/')
conf.set_quoted('CATALOGS', ' '.join(linguas))
conf.set('ENABLE_NLS', nls ? 1 : false)
conf.set('HAVE_GETTEXT', nls ? 1 : false)
conf.set('HAVE_ICONV', cxx.has_function('iconv_open', prefix: '#include <iconv.h>') ? 1 : false)
conf.set('HAVE_LC_MESSAGES', cxx.has_header_symbol('locale.h', 'LC_MESSAGES') ? 1 : false)
conf.set_quoted(db_backend == 'mysql' ? 'HAVE_LIBMYSQL' : 'HAVE_LIBPQ', '1')
conf.set('WITH_ACTORS', with_actors.to_int())
conf.set('WITH_FILMS', with_films.to_int())
conf.set('WITH_RECORDS', with_records.to_int())
configure_file(input: 'cdmgr-cfg.h.in', output: 'cdmgr-cfg.h', configuration: conf)

config_inc = include_directories('.')

subdir('src')
subdir('po')
subdir('data')
subdir('doc')
subdir('SQL')

summary({'Database backend': db_backend,
         'Films': with_films,
         'Records': with_records,
         'Actors': with_actors,
         'Translations': nls ? ' '.join(linguas) : 'disabled',
         'Trace level': get_option('trace'),
         'Contracts': checks >= 0 ? 'enforced (check level @0@)'.format(checks) : 'ignored'},
        section: 'Configuration')
