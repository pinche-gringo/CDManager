if nls
    i18n = import('i18n')
    i18n.gettext(meson.project_name(),
                 languages: linguas,
                 args: ['--keyword=_', '--keyword=N_',
                        '--copyright-holder=Markus Schwab',
                        '--msgid-bugs-address=' + email])
endif
