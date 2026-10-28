# Substitutes @EMAIL@ and the links to the other languages (@DOC_<LANG>@) in the documentation
doc_conf = configuration_data({'EMAIL': email})
langs = ['en', 'es', 'de']
foreach lang : langs
    doc_conf.set('DOC_' + lang.to_upper(), 'CDManager.html.' + lang)
endforeach
foreach lang : langs
    configure_file(input: 'CDManager.html.' + lang, output: 'CDManager.html.' + lang,
                   configuration: doc_conf, install_dir: docdir)
endforeach
