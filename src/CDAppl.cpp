// PROJECT     : CDManager
// SUBSYSTEM   : Application
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 22.12.2004
// COPYRIGHT   : Copyright (C) 2004 - 2006, 2009 - 2011, 2026

// This file is part of CDManager
//
// CDManager is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CDManager is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CDManager.  If not, see <http://www.gnu.org/licenses/>.

#include <cdmgr-cfg.h>

#include <charconv>
#include <format>
#include <print>
#include <string_view>

#include <glibmm/convert.h>

#include <gtkmm/application.h>

#include <YGP/INIFile.h>

#if WITH_FILMS == 1
#    include "Film.h"
#endif
#include "CDManager.h"
#include "DBFactory.h"

#include "CDAppl.h"

const YGP::IVIOApplication::longOptions CDAppl::lo[] = {
    {IVIOAPPL_HELP_OPTION}, {"user", 'u'},    {"password", 'p'}, {"database", 'd'}, {"host", 'H'},
    {"port", 'P'},          {"backend", 'b'}, {"file", 'f'},     {"version", 'V'},  {nullptr, '\0'}};

//-----------------------------------------------------------------------------
/// Displays the help
//-----------------------------------------------------------------------------
void CDAppl::showHelp() const {
    std::cout << _("Utility to manage CDs\n\nUsage: ") << PACKAGE << _(" [OPTIONS]\n\n") << "  -u, --user ....... "
              << _("[USER] User for database login\n") << "  -p, --password ... " << _("[PWD] Password for database login\n")
              << "  -d, --database ... " << _("[NAME] Name of the database (default: CDMedia)\n") << "  -H, --host ....... "
              << _("[HOST] Host or socket (path) of the database server (default: local host)\n") << "  -P, --port ....... "
              << _("[PORT] Port of the database server (default: the one of the backend)\n") << "  -b, --backend .... "
              << _("[DB] Database backend: auto (default), mysql or postgresql\n") << "  -f, --file ....... "
              << _("[FILE] Use file as INI file\n") << "  -V, --version .... " << _("Output version information and exit\n")
              << "  -h, -?, --help ... " << _("Displays this help and exit\n\n")
              << _("The INI file can have the following entries:\n\n")
              << ("  [Database]\n"
                  "  User=user\n"
                  "  Password=pwd\n"
                  "  Database=CDMedia\n"
                  "  Host=localhost\n"
                  "  Port=5432\n"
                  "  Backend=auto\n\n"
                  "  [Export]\n"
                  "  FilmHead=Films.head\n"
                  "  FilmFoot=Films.foot\n"
                  "  RecordHead=Records.head\n"
                  "  RecordFoot=Records.foot\n"
                  "  OutputDir=/var/www/cds/\n"
#if WITH_FILMS == 1
                  "  \n  [Films]\n"
                  "  Language=de\n"
#endif
                 );
}

//-----------------------------------------------------------------------------
/// Checks the validity of the passed option
/// \param option: Actual option
/// \returns \c bool: Status; false: Invalid option/option-value Require: option not '\0'
//-----------------------------------------------------------------------------
bool CDAppl::handleOption(const char option) {
    contract_assert(option != '\0');

    switch (option) {
    case 'u':
        if (checkOptionValue())
            options.user = getOptionValue();
        else
            std::cerr << PACKAGE << _("-warning: No user specified! Ignoring option `u'\n");
        break;

    case 'p':
        if (checkOptionValue())
            options.password = getOptionValue();
        else
            std::cerr << PACKAGE << _("-warning: No password specified! Ignoring option `p'\n");
        break;

    case 'd':
        if (checkOptionValue())
            options.database = getOptionValue();
        else
            std::cerr << PACKAGE << _("-warning: No database specified! Ignoring option `d'\n");
        break;

    case 'H':
        if (checkOptionValue())
            options.host = getOptionValue();
        else
            std::cerr << PACKAGE << _("-warning: No host specified! Ignoring option `H'\n");
        break;

    case 'P': {
        const char* value(checkOptionValue() ? getOptionValue() : nullptr);
        if (!value || !parsePort(value, options.port))
            std::cerr << PACKAGE << _("-warning: No valid port specified! Ignoring option `P'\n");
        break;
    }

    case 'b':
        if (checkOptionValue() && DBFactory::isValid(checkOptionValue()))
            options.backend = getOptionValue();
        else
            std::cerr << PACKAGE << _("-warning: No valid backend (auto, mysql or postgresql) specified! Ignoring option `b'\n");
        break;

    case 'f': {
        const char* pFile(getOptionValue());
        if (pFile)
            readINIFile(pFile);
        else
            std::cerr << PACKAGE << _("-warning: No file specified! Ignoring option `f'\n");
        break;
    }

    case 'V':
        std::cout << description() << '\n';
        exit(0);
        break;

    default:
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
/// Converts the passed text to a port number
/// \param value Text to convert
/// \param port Set to the port, if the value is valid
/// \returns bool True, if the value is a valid port
//-----------------------------------------------------------------------------
bool CDAppl::parsePort(std::string_view value, unsigned int& port) {
    unsigned int number(0);
    const auto [end, error](std::from_chars(value.data(), value.data() + value.size(), number));
    if ((error != std::errc()) || (end != value.data() + value.size()) || !number || (number > MAX_PORT))
        return false;
    port = number;
    return true;
}

//-----------------------------------------------------------------------------
/// Reads the options of the INI-file
/// \param pFile: Pointer to filename
/// \param Requieres : pFile not NULL
//-----------------------------------------------------------------------------
void CDAppl::readINIFile(const char* pFile) {
    try {
        INIFILE(pFile);
        // DB
        INISECTION(Database);
        INIATTR2(Database, std::string, options.user, User);
        INIATTR2(Database, std::string, options.password, Password);
        INIATTR2(Database, std::string, options.database, Database);
        INIATTR2(Database, std::string, options.host, Host);
        INIATTR2(Database, unsigned int, options.port, Port);
        INIATTR2(Database, std::string, options.backend, Backend);

        // Export-otions
        INIOBJ(options, Export);

#if WITH_FILMS == 1
        // Language in which to show the films
        INISECTION(Films);
        INIATTR2(Films, std::string, Film::currLang, Language);
#endif

        INIFILE_READ();
    }
    catch (YGP::FileError&) {
    }
    catch (std::exception& error) {
        std::print(std::cerr, "{}-warning: Error reading INI-file `{}'! {}\n", name(), pFile, error.what());
    }
    options.pINIFile = pFile;
}

//-----------------------------------------------------------------------------
/// Performs the job of the application
/// \param int: Number of parameters (without options)
/// \param const char*: Array with pointer to arguments
/// \returns \c int: Status
//-----------------------------------------------------------------------------
int CDAppl::perform(int, const char**) {
    // Check the values of the INI file (the command line ones are already checked)
    if (!DBFactory::isValid(options.backend)) {
        std::cerr << PACKAGE << _("-warning: Invalid database backend! Detecting it automatically ...\n");
        options.backend = DBFactory::AUTO;
    }
    if (options.port > MAX_PORT) {
        std::cerr << PACKAGE << _("-warning: Invalid port! Using the default ...\n");
        options.port = 0;
    }

    try {
        if (!options.password.empty())
            options.password = Glib::locale_to_utf8(options.password);
    }
    catch (Glib::ConvertError&) {
        options.password.clear();
        std::cerr << PACKAGE << _("-warning: Can't convert password to UTF-8! Ignoring ...\n");
    }

    try {
        if (!options.user.empty())
            options.user = Glib::locale_to_utf8(options.getUser());
    }
    catch (Glib::ConvertError&) {
        options.user.clear();
        std::cerr << PACKAGE << _("-warning: Can't convert username to UTF-8! Ignoring ...\n");
    }

    // The options are already handled; so don't pass them to GTK
    Glib::RefPtr<Gtk::Application> app(
        Gtk::Application::create("net.sourceforge.CDManager", Gio::Application::Flags::NON_UNIQUE));
    return app->make_window_and_run<CDManager>(0, nullptr, options);
}

//-----------------------------------------------------------------------------
/// Returns a short description of the program (not the help!)
/// \returns const char*: Pointer to a short description
//-----------------------------------------------------------------------------
const char* CDAppl::description() const {
    static const std::string version(std::format("{} V{} - {} {} - {}\n\n{}; e-mail: {}\n{}", PACKAGE, VERSION, _("Compiled on"),
                                                 __DATE__, __TIME__, _("Copyright (C) 2004 - 2026 Markus Schwab"),
                                                 PACKAGE_BUGREPORT,
                                                 _("Distributed under the terms of the GNU General Public License")));
    return version.c_str();
}

//-----------------------------------------------------------------------------
/// Entrypoint of application
/// \param argc: Number of parameters
/// \param argv: Array with pointer to parameter
/// \returns \c int: Status
//-----------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    YGP::IVIOApplication::initI18n(PACKAGE, LOCALEDIR);

    CDAppl appl(argc, const_cast<const char**>(argv));
    return appl.run();
}
