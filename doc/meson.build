# Substitutes @EMAIL@ in the documentation
doc_conf = configuration_data({'EMAIL': email})
foreach lang : ['en', 'es', 'de']
    configure_file(input: 'CDManager.html.' + lang, output: 'CDManager.html.' + lang,
                   configuration: doc_conf, install_dir: docdir)
endforeach
