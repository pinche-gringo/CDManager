// PROJECT     : CDManager
// SUBSYSTEM   : CDWriter
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 07.01.2005
// COPYRIGHT   : Copyright (C) 2005 - 2007, 2009 - 2011, 2026

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

#define DONT_CONVERT
#include <cdmgr-cfg.h>

#include <cerrno>
#include <cstdlib>
#include <cstring>

#include <algorithm>
#include <array>
#include <format>
#include <fstream>
#include <iostream>
#include <memory>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include <glibmm/convert.h>
#include <glibmm/ustring.h>

#include <YGP/ADate.h>
#include <YGP/ATStamp.h>
#include <YGP/Exception.h>
#include <YGP/File.h>
#include <YGP/Relation.h>
#include <YGP/Trace.h>

#include "DB.h"
#include "Genres.h"
#include "Language.h"
#include "Words.h"
#include "Writer.h"

#if WITH_FILMS == 1
#    include "Director.h"
#    include "Film.h"
#endif

#if WITH_RECORDS == 1
#    include "Interpret.h"
#    include "Record.h"
#endif

#include "CDWriter.h"

const YGP::IVIOApplication::longOptions CDWriter::lo[] = {{IVIOAPPL_HELP_OPTION}, {"version", 'V'},
#if WITH_RECORDS == 1
                                                          {"recHeader", 'r'},     {"recFooter", 'R'},
#endif
#if WITH_FILMS == 1
                                                          {"filmHeader", 'm'},    {"filmFooter", 'M'},
#endif
                                                          {"outputDir", 'd'},     {nullptr, '\0'}};

namespace {

//-----------------------------------------------------------------------------
/// Replaces every occurrence of %1 in the passed text
/// \param text: Text to change
/// \param value: Value to substitute %1 with
/// \remarks Unlike Glib::ustring::compose the text may contain other %-signs
//-----------------------------------------------------------------------------
void replacePlaceholder(std::string& text, const std::string& value) {
    std::string::size_type pos(0);
    while ((pos = text.find("%1", pos)) != std::string::npos)
        text.replace(pos, 2, value);
}

} // namespace

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
CDWriter::~CDWriter() { TRACE9("CDWriter::~CDWriter ()"); }

//-----------------------------------------------------------------------------
/// Displays the help
//-----------------------------------------------------------------------------
void CDWriter::showHelp() const {
    std::cout << _("Utitily to write HTML documents from data received\n\nUsage: ") << name()
              << _(" [OPTIONS] [LANGUAGE-ID] [MEMORY-ID]\n\n") << "  -d, --outputDir ..... " << _("Directory to export data to\n")
#if WITH_RECORDS == 1
              << "  -r, --recHeader ..... " << _("File to use as header for records\n") << "  -R, --recFooter ..... "
              << _("File to use as footer for records\n")
#endif
#if WITH_FILMS == 1
              << "  -m, --filmHeader ... " << _("File to use as header for films\n") << "  -M, --filmFooter ... "
              << _("File to use as footer for films\n")
#endif
              << "  -V, --version ....... " << _("Output version information and exit\n") << "  -h, -?, --help ...... "
              << _("Displays this help and exit\n\n");
}

//-----------------------------------------------------------------------------
/// Checks the validity of the passed option
/// \param option: Actual option
/// \returns \c bool: Status; false: Invalid option/option-value Require :
///     option not '\0�'
//-----------------------------------------------------------------------------
bool CDWriter::handleOption(const char option) {
    contract_assert(option != '\0');

    switch (option) {
    case 'd':
        if (const char* pDir(getOptionValue()); pDir)
            opt.setDirOutput(pDir);
        else
            std::cerr << name() << _("-warning: No directory specified! Ignoring option `d'\n");
        break;

#if WITH_FILMS == 1
    case 'm':
    case 'M':
        if (const char* pFile(getOptionValue()); pFile)
            (option == 'm') ? opt.setMHeader(pFile) : opt.setMFooter(pFile);
        else
            std::cerr << name()
                      << Glib::ustring::compose(_("-warning: No file specified! Ignoring option `%1'\n"),
                                                Glib::ustring(1, option));
        break;

#endif
#if WITH_RECORDS == 1
    case 'R':
    case 'r':
        if (const char* pFile(getOptionValue()); pFile)
            (option == 'r') ? opt.setRHeader(pFile) : opt.setRFooter(pFile);
        else
            std::cerr << name()
                      << Glib::ustring::compose(_("-warning: No file specified! Ignoring option `%1'\n"),
                                                Glib::ustring(1, option));
        break;
#endif

    case 'V':
        std::cout << description() << '\n';
        std::exit(0);
        break;

    default:
        return false;
    }
    return true;
}

//-----------------------------------------------------------------------------
/// Writes the header for the table
/// \param lang: Language code; Used for links to documents
/// \param format: Format of header; A sequence of the following characters
///     - d: Director
///     - n: Name of film
///     - y: Year the film was made
///     - g: Genre of film
///     - m: Media containing the film
///     - l: Language(s)
///     - [: Start of <div>-tag
///     - ]: End of <div>-tag
///     - !: With HMTL column-separaters "</td><td>"
///     - =: With HTML space "&nbsp;"
/// \param upSorted: Flag, if first column is sorted upwards
/// \param stream: Stream to write to
/// \param lead: String with which to start filenames
/// \remarks Other characters are just copied
//-----------------------------------------------------------------------------
void CDWriter::writeHeader(const char* lang, const char* format, std::ostream& stream, bool upSorted, const char* lead) {
    TRACE9("CDWriter::writeHeader (2x const char*, std::ostream&, bool, const char*) - " << lang << "; " << format << "; "
                                                                                         << lead);

    static constexpr std::string_view formats("adnygml-![]=");
    static constexpr std::array<const char*, formats.size()> docs {"",      "",      "-Name", "-Year", "-Genre", "-Media",
                                                                   "-Lang", nullptr, nullptr, nullptr, nullptr,  nullptr};
    static const auto titles = std::to_array<std::string>({_("Interpret"), _("Director"), _("Name"), _("Year"), _("Genre"),
                                                           _("Media"), _("Language(s)"), " | ", "</td><td>",
                                                           "<div class=\"header\">", "</div>", "&nbsp;"});
    static_assert(std::tuple_size_v<decltype(titles)> == formats.size());

    for (; *format; ++format) {
        if (auto pos(formats.find(*format)); pos != std::string_view::npos) {
            contract_assert(titles[pos].size());

            if (docs[pos]) {
                stream << "<a href=\"" << lead << docs[pos];
                if (upSorted) {
                    stream << "down";
                    upSorted = false;
                }
                stream << ".html." << lang << "\">" << titles[pos] << "</a>";
            }
            else
                stream << titles[pos];
        }
        else
            stream << *format;
    }
    stream << '\n';
}

//-----------------------------------------------------------------------------
/// Performs the job of the application
/// \param int: Number of parameters (without options)
/// \param const char*: Array with pointer to arguments
/// \returns \c int: Status
//-----------------------------------------------------------------------------
int CDWriter::perform(int argc, const char** argv) {
    TRACE9("CDWriter::perform (int, const char**) - " << argc);
    if (argc != 2) {
        std::cerr << name()
                  << _("-error: Need language id and memory-key as parameters\n"
                       "(This program is designed to be called by the CDManager-application)\n");
        return -1;
    }

    Genres filmGenres, recGenres;

    try {
        if (!std::atoi(argv[1]))
            throw std::invalid_argument(_("Invalid memory-key (0)!"));

        Words::access(std::atoi(argv[1]));
        TRACE9("Words: " << Words::getMemoryKey() << ": " << Words::cArticles() << '/' << Words::cNames());

        Genres::loadFromFile(DATADIR "Genres.dat", recGenres, filmGenres, *argv);
    }
    catch (std::invalid_argument& e) {
        std::cerr << name() << Glib::ustring::compose(_("-error: Can't access reserved words!\n\nReason: %1"), e.what()).raw()
                  << '\n';
        return -2;
    }
    catch (std::exception& e) {
        std::cerr << name() << Glib::ustring::compose(_("Can't read datafile containing the genres!\n\nReason: %1"), e.what()).raw()
                  << '\n';
        return -3;
    }

#if WITH_FILMS == 1
    Film::currLang = *argv;
    Glib::ustring transTitleFilm(_("Films (by %1)"));
#endif
#if WITH_RECORDS == 1
    Glib::ustring transTitleRecord(_("Records (by %1)"));
#endif

    struct HTMLData {
        std::string name;
        Glib::ustring& source;
        std::string target {};
    } htmlData[] = {
#if WITH_FILMS == 1
        {opt.getMHeader(), transTitleFilm},
        {opt.getMFooter(), transTitleFilm}
#    if WITH_RECORDS == 1
        ,
#    endif
#endif
#if WITH_RECORDS == 1
        {opt.getRHeader(), transTitleRecord},
        {opt.getRFooter(), transTitleRecord}
#endif
    };

    for (auto& [file, source, target] : htmlData) {
        if (file.size() && (file[0] != YGP::File::DIRSEPARATOR))
            file = DATADIR + file;

        if (!readHeaderFile(file.c_str(), argv[0], target, source)) {
            std::string error(
                Glib::ustring::compose(_("Error reading header file `%1'!\n\nReason: %2"), file, strerror(errno)).raw());

            TRACE1("CDWriter::perform (int, const char**) - Error reading HTML-header/footer:\n\t" << error);
            std::cerr << name() << ": " << error << '\n';
            return -4;
        }
    } // end-for

#if WITH_FILMS == 1
    std::ofstream fileFilm;
    if (createFile(opt.getDirOutput() + "Films.html", argv[0], fileFilm))
        return -5;

    // Writing the title for films
    std::string titleFilm(htmlData[0].target);
    replacePlaceholder(titleFilm, _("Director"));
    fileFilm << titleFilm;

    writeHeader(argv[0], "[d-n-y-g-m-l]", fileFilm);

    std::string filmFormat("%n|%y|%g|%t|%l");
    FilmWriter filmWriter(filmFormat, filmGenres);
    filmWriter.printStart(fileFilm, "");

    HFilm film;
    HDirector director;
    std::vector<HFilm> films;
    std::vector<HDirector> directors;
    YGP::Relation1_N<HDirector, HFilm> relFilms("films");
    std::string usedLanguages;
#endif

#if WITH_RECORDS == 1
    std::ofstream fileRec;
    if (createFile(opt.getDirOutput() + "Records.html", argv[0], fileRec))
        return -5;

    // Writing the title for records
    std::string titleRec(htmlData[WITH_FILMS << 1].target);
    replacePlaceholder(titleRec, _("Interpret"));
    fileRec << titleRec;

    writeHeader(argv[0], "[a-n-y-g]", fileRec, true, "Records");

    std::string recordFormat("%n|%y|%g");
    RecordWriter recWriter(recordFormat, recGenres);
    recWriter.printStart(fileRec, "");

    HRecord record;
    HInterpret artist;
    std::vector<HRecord> records;
    std::vector<HInterpret> artists;
    YGP::Relation1_N<HInterpret, HRecord> relRecords("records");
#endif

    // Read input from stdin; both films and records can be handled
    char type;
    std::cin >> type;
    while (!std::cin.eof()) {
        try {
            TRACE4("CDWriter::perform (int, char**) - Type: " << type);
            switch (type) {
#if WITH_FILMS == 1
            case 'D':
                director = std::make_shared<Director>();
                std::cin >> *director;
                TRACE9("CDWriter::perform (int, char**) - Director: " << director->getName());
                directors.push_back(director);
                break;

            case 'M': {
                contract_assert(director);
                film = std::make_shared<Film>();
                std::cin >> *film;
                TRACE9("CDWriter::perform (int, char**) - Film: " << film->getName());
                if (!relFilms.isRelated(director))
                    filmWriter.writeDirector(director, fileFilm);

                relFilms.relate(director, film);
                filmWriter.writeFilm(film, director, fileFilm);
                films.push_back(film);

                // Empty parts are skipped, like boost::char_separator did
                for (auto lang : film->getLanguage() | std::views::split(',') |
                                     std::views::transform([](auto&& part) { return std::string_view(part); }) |
                                     std::views::filter([](std::string_view part) { return !part.empty(); })) {
                    if (usedLanguages.find(lang) == std::string::npos) {
                        usedLanguages += ',';
                        usedLanguages += lang;
                    }
                }
                break;
            }
#endif

#if WITH_RECORDS == 1
            case 'I':
                artist = std::make_shared<Interpret>();
                std::cin >> *artist;
                TRACE9("CDWriter::perform (int, char**) - Artist: " << artist->getName());
                artists.push_back(artist);
                break;

            case 'R':
                contract_assert(artist);
                record = std::make_shared<Record>();
                std::cin >> *record;
                TRACE9("CDWriter::perform (int, char**) - Record: " << record->getName());
                if (!relRecords.isRelated(artist))
                    recWriter.writeInterpret(artist, fileRec);

                relRecords.relate(artist, record);
                recWriter.writeRecord(record, artist, fileRec);
                records.push_back(record);
                break;
#endif

            default:
                contract_assert(false);
                return -1;
            }
        }
        catch (std::exception& error) {
            static constexpr std::string_view types("DMIR");
            static constexpr std::array<const char*, types.size()> what {N_("Director"), N_("Film"), N_("Interpret"),
                                                                         N_("Record")};
            const auto i(types.find(type));
            Glib::ustring entity(_((i != std::string_view::npos) ? what[i] : N_("unknown entity")));

            std::cerr << name() << Glib::ustring::compose(_("-error: Can't read %1: %2"), entity, error.what()) << '\n';
            break;
        }
        catch (...) {
            std::cerr << name() << _("-error: Unknown error!\n");
        }

        std::cin >> type;
    } // end-while not eof

#if WITH_FILMS == 1
    filmWriter.printEnd(fileFilm);
    fileFilm << htmlData[1].target;
    fileFilm.close();

    // Write reverse file
    if (createFile(opt.getDirOutput() + "Filmsdown.html", argv[0], fileFilm))
        return -5;
    fileFilm << titleFilm;
    writeHeader(argv[0], "[d-n-y-g-m-l]", fileFilm, false);

    filmWriter.printStart(fileFilm, "");
    for (const auto& dir : directors | std::views::reverse)
        if (relFilms.isRelated(dir)) {
            filmWriter.writeDirector(dir, fileFilm);

            const std::vector<HFilm>& dirFilms(relFilms.getObjects(dir));
            contract_assert(dirFilms.size());
            for (const auto& m : dirFilms)
                filmWriter.writeFilm(m, dir, fileFilm);
        }
    filmWriter.printEnd(fileFilm);
    fileFilm << htmlData[1].target;
    fileFilm.close();
#endif

#if WITH_RECORDS == 1
    recWriter.printEnd(fileRec);
    fileRec.flush();
    fileRec << htmlData[(WITH_FILMS << 1) + 1].target;
    fileRec.close();

    // Write reverse file
    if (createFile(opt.getDirOutput() + "Recordsdown.html", argv[0], fileRec))
        return -5;
    fileRec << titleRec;
    writeHeader(argv[0], "[a-n-y-g]", fileRec, false, "Records");

    recWriter.printStart(fileRec, "");
    for (const auto& interpret : artists | std::views::reverse)
        if (relRecords.isRelated(interpret)) {
            recWriter.writeInterpret(interpret, fileRec);

            const std::vector<HRecord>& dirRecords(relRecords.getObjects(interpret));
            contract_assert(dirRecords.size());
            for (const auto& m : dirRecords)
                recWriter.writeRecord(m, interpret, fileRec);
        }
    recWriter.printEnd(fileRec);
    fileRec << htmlData[(WITH_FILMS << 1) + 1].target;
    fileRec.close();
#endif

    struct Output {
        const char* title;
        const char* file;
        const char* filedown;
        const char* format;
        const char* sorted;
#if WITH_FILMS == 1
        bool (*cmpFilm)(const HFilm&, const HFilm&) {nullptr};
#endif
#if WITH_RECORDS == 1
        bool (*cmpRecord)(const HRecord&, const HRecord&) {nullptr};
#endif
        const char* lead;
        unsigned int type;
    };
    const Output aOutputs[] = {
#if WITH_FILMS == 1
        // Entries for films
        {.title = "[n]|[d]|[y]|[g]|[m]|[l]", .file = "Films-Name.html", .filedown = "Films-Namedown.html",
         .format = "%n|%d|%y|%g|%t|%l", .sorted = N_("Name"), .cmpFilm = &Film::compByName, .lead = "Films", .type = 0},
        {.title = "[y]|[n]|[d]|[g]|[m]|[l]", .file = "Films-Year.html", .filedown = "Films-Yeardown.html",
         .format = "%y|%n|%d|%g|%t|%l", .sorted = N_("Year"), .cmpFilm = &Film::compByYear, .lead = "Films", .type = 0},
        {.title = "[g]|[n]|[d]|[y]|[m]|[l]", .file = "Films-Genre.html", .filedown = "Films-Genredown.html",
         .format = "%g|%n|%d|%y|%t|%l", .sorted = N_("Genre"), .cmpFilm = &Film::compByGenre, .lead = "Films", .type = 0},
        {.title = "[m]|[n]|[d]|[y]|[g]|[l]", .file = "Films-Media.html", .filedown = "Films-Mediadown.html",
         .format = "%t|%n|%d|%y|%g|%l", .sorted = N_("Media"), .cmpFilm = &Film::compByMedia, .lead = "Films", .type = 0}
#    if WITH_RECORDS == 1
        ,
#    endif
#endif
#if WITH_RECORDS == 1
        // Entries for records
        {.title = "[n]|[a]|[y]|[g]", .file = "Records-Name.html", .filedown = "Records-Namedown.html",
         .format = "%n|%d|%y|%g", .sorted = N_("Name"), .cmpRecord = &Record::compByName, .lead = "Records", .type = 1},
        {.title = "[y]|[n]|[a]|[g]", .file = "Records-Year.html", .filedown = "Records-Yeardown.html",
         .format = "%y|%n|%d|%g", .sorted = N_("Year"), .cmpRecord = &Record::compByYear, .lead = "Records", .type = 1},
        {.title = "[g]|[n]|[a]|[y]", .file = "Records-Genre.html", .filedown = "Records-Genredown.html",
         .format = "%g|%n|%d|%y", .sorted = N_("Genre"), .cmpRecord = &Record::compByGenre, .lead = "Records", .type = 1}
#endif
    };

    std::ofstream fileOut;
    std::string strTitle;
    // This combines writing films and records
    for (const auto& output : aOutputs) {
        if (createFile(opt.getDirOutput() + output.file, argv[0], fileOut))
            return -5;
        strTitle = htmlData[output.type << 1].target;
        replacePlaceholder(strTitle, _(output.sorted));

        fileOut << strTitle;

        std::stringstream header;
        writeHeader(argv[0], output.title, header, true, output.lead);

#if WITH_RECORDS == 1
        if (output.type) {
            std::ranges::sort(records, output.cmpRecord);

            recordFormat = output.format;
            RecordWriter writer(recordFormat, recGenres);
            writer.printStart(fileOut, header.str());

            HInterpret interpret;
            for (const auto& m : records) {
                interpret = relRecords.getParent(m);
                contract_assert(interpret);
                writer.writeRecord(m, interpret, fileOut);
            }
            writer.printEnd(fileOut);
        }
        else
#endif
#if WITH_FILMS == 1
        {
            std::ranges::sort(films, output.cmpFilm);

            filmFormat = output.format;
            FilmWriter writer(filmFormat, filmGenres);
            writer.printStart(fileOut, header.str());

            HDirector director;
            for (const auto& m : films) {
                director = relFilms.getParent(m);
                contract_assert(director);
                writer.writeFilm(m, director, fileOut);
            }
            writer.printEnd(fileOut);
        }
#endif

        fileOut << htmlData[(output.type << 1) + 1].target;
        fileOut.close();

        if (createFile(opt.getDirOutput() + output.filedown, argv[0], fileOut))
            return -5;
        fileOut << strTitle;

        std::stringstream rheader;
        writeHeader(argv[0], output.title, rheader, false, output.lead);

#if WITH_RECORDS == 1
        if (output.type) {
            RecordWriter writer(recordFormat, recGenres);
            writer.printStart(fileOut, rheader.str());

            for (const auto& m : records | std::views::reverse) {
                HInterpret interpret(relRecords.getParent(m));
                contract_assert(interpret);
                writer.writeRecord(m, interpret, fileOut);
            }
            writer.printEnd(fileOut);
        }
        else
#endif
#if WITH_FILMS == 1
        {
            FilmWriter writer(filmFormat, filmGenres);
            writer.printStart(fileOut, rheader.str());

            for (const auto& m : films | std::views::reverse) {
                HDirector director(relFilms.getParent(m));
                contract_assert(director);
                writer.writeFilm(m, director, fileOut);
            }
            writer.printEnd(fileOut);
        }
#endif

        fileOut << htmlData[(output.type << 1) + 1].target;
        fileOut.close();
    }

#if WITH_FILMS == 1
    // Export films by language
    if (createFile(opt.getDirOutput() + "Films-Lang.html", argv[0], fileOut))
        return -5;
    titleFilm = htmlData[0].target;
    replacePlaceholder(titleFilm, _("Language"));
    fileOut << titleFilm;

    writeHeader(argv[0], "[n-d-y-g-m]", fileOut, false);

    const std::ranges::subrange languages(Language::begin(), Language::end());
    fileOut << "<div class=\"header\">|";
    for (const auto& [id, language] : languages)
        if (usedLanguages.find(id) != std::string::npos)
            fileOut << " <a href=\"#" << id << "\"><img src=\"images/" << id << ".png\" alt=\"" << id << " \">&nbsp;"
                    << language.getInternational() << "</a> |";
    fileOut << "</div>";

    filmFormat = "%l|%n|%d||%y|%g|%t";
    FilmWriter langWriter(filmFormat, filmGenres);
    std::ranges::sort(films, &Film::compByName);

    langWriter.printStart(fileOut, "");

    for (const auto& [id, language] : languages) {
        if (usedLanguages.find(id) != std::string::npos) {
            fileOut << "<tr><td colspan=\"6\"><div class=\"header\"><a name=\"" << id << "\">\n<br></a><h2>"
                    << language.getInternational() << "</h2></div></td></tr>";

            fileOut << "<tr><td>";
            writeHeader(argv[0], "[=]![n]![d]![y]![g]![m]", fileOut, false);
            fileOut << "</td></tr>";

            for (const auto& m : films)
                if ((m->getLanguage().find(id) != std::string::npos) || (m->getTitles().find(id) != std::string::npos)) {
                    HDirector director(relFilms.getParent(m));
                    contract_assert(director);
                    langWriter.writeFilm(m, director, fileOut);
                }
        }
    }

    langWriter.printEnd(fileOut);
    fileOut << htmlData[1].target;
    fileOut.close();
#endif

    Words::destroy();
    return 0;
}

//-----------------------------------------------------------------------------
/// Returns a short description of the program (not the help!)
/// \returns const char*: Pointer to a short description
//-----------------------------------------------------------------------------
const char* CDWriter::description() const {
    static const std::string version(
        std::format("{} V" VERSION " - {} " __DATE__ " - " __TIME__ "\n\n{}", name(), _("Compiled on"),
                    _("Copyright (C) 2005 - 2007, 2009, 2011 Markus Schwab; e-mail: g17m0@users.sourceforge.net"
                      "\nDistributed under the terms of the GNU General "
                      "Public License")));
    return version.c_str();
}

//-----------------------------------------------------------------------------
/// Creates a file. Errors are reported to std::cerr
/// \param filename: Name of file to create
/// \param lang: Language-id
/// \param file: Created stream
/// \returns int: Error code
//-----------------------------------------------------------------------------
int CDWriter::createFile(const std::string& filename, const char* lang, std::ofstream& file) {
    TRACE9("CDWriter::createFile (const std::string&, const char*, std::ofstream&) - " << filename);

    const std::string utf8file(std::format("{}.{}", filename, lang));
    file.open(utf8file);
    if (!file) {
        const int error(errno);
        Glib::ustring msg(Glib::ustring::compose(_("Can't create file `%1'!\n\nReason: %2."), utf8file, strerror(error)));
        std::cerr << name() << _("-error: ") << msg;
        return error;
    }
    return 0;
}

//-----------------------------------------------------------------------------
/// Reads the contents of the passed file into the passed variable and performs
/// some substitutions within.
///
/// The substitutions are:
///   - @TITLE@ with the passed title
///   - @TIMESTAMPE@ with the current date/time
///   - @DATE@ with current date
///   - @YEAR@ with the current year
/// \param file: File to read from
/// \param lang: ID of language
/// \param target: Variable receiving the input
/// \param title: Value to substitute @TITLE@ with
/// \returns bool: True, if successful
/// \remarks The argument target is overwritten anyway
//-----------------------------------------------------------------------------
bool CDWriter::readHeaderFile(const char* file, const char* lang, std::string& target, const Glib::ustring& title) {
    TRACE9("CDWriter::readHeaderFile (const char*) - " << file << " (" << lang << "): " << title);

    target = std::format("{}.{}", file, lang);
    std::ifstream input(target);
    if (!input) {
        input.clear();
        input.open(file);
        if (!input)
            return false;
    }
    target.clear();

    std::array<char, 512> buffer;
    // Read as long as there is data or an error occurs
    while (input.read(buffer.data(), buffer.size()), input.gcount())
        target.append(buffer.data(), input.gcount());

    std::string::size_type i;
    while ((i = target.find("@TITLE@")) != std::string::npos)
        target.replace(i, 7, title);

    while ((i = target.find("@TIMESTAMP@")) != std::string::npos)
        target.replace(i, 11, YGP::ATimestamp::now().toString());

    while ((i = target.find("@DATE@")) != std::string::npos)
        target.replace(i, 6, YGP::ADate::today().toString());

    while ((i = target.find("@YEAR@")) != std::string::npos)
        target.replace(i, 6, YGP::ADate::today().toString("%Y"));
    return true;
}

//-----------------------------------------------------------------------------
/// Entrypoint of application
/// \param argc: Number of parameters
/// \param argv: Array with pointer to parameter
/// \returns \c int: Status
//-----------------------------------------------------------------------------
int main(int argc, const char* argv[]) {
    CDWriter::initI18n(PACKAGE, LOCALEDIR);
    CDWriter appl(argc, argv);

    Language::init();
    return appl.run();
}
