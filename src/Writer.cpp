// PROJECT     : CDManager
// SUBSYSTEM   : Writer
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 27.11.2004
// COPYRIGHT   : Copyright (C) 2004 - 2007, 2009 - 2011, 2026

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

#include <format>
#include <ranges>
#include <string_view>

#include <YGP/Trace.h>

#include "CDType.h"

#include "Writer.h"

#if WITH_FILMS == 1
//-----------------------------------------------------------------------------
/// Substitution of column values
/// \param ctrl: Control character to subsitute
/// \returns std::string: Substituted string
//-----------------------------------------------------------------------------
std::string FilmWriter::getSubstitute(const char ctrl, bool) const {
    if (hFilm) {
        contract_assert(hDirector);

        switch (ctrl) {
        case 'n':
            return YGP::TableWriter::changeHTMLSpecialChars(hFilm->getName());

        case 'y':
            return hFilm->getYear().toString();

        case 'g':
            contract_assert(hFilm->getGenre() < genres.size());
            return YGP::TableWriter::changeHTMLSpecialChars(genres.getGenre(hFilm->getGenre()));

        case 'd':
            return YGP::TableWriter::changeHTMLSpecialChars(hDirector->getName());

        case 't':
            return YGP::TableWriter::changeHTMLSpecialChars(CDType::getInstance()[hFilm->getType()]);

        case 'l': {
            std::string output(addLanguageLinks(hFilm->getLanguage()));
            if (hFilm->getTitles().size()) {
                output += " &ndash; ";
                output += addLanguageLinks(hFilm->getTitles());
            }
            return output;
        }
        } // endswitch
    }
    else {
        contract_assert(hDirector);
        contract_assert(!hFilm);

        if (ctrl == 'n')
            return YGP::TableWriter::changeHTMLSpecialChars(hDirector->getName());
    }
    return std::string(1, ctrl);
}

//-----------------------------------------------------------------------------
/// Writes a film into the table
/// \param film: Film to write
/// \param director: Director of film
/// \param out: Stream to write to
//-----------------------------------------------------------------------------
void FilmWriter::writeFilm(const HFilm& film, const HDirector& director, std::ostream& out) {
    contract_assert(!hFilm);
    contract_assert(!hDirector);
    hFilm = film;
    hDirector = director;

    out << "<tr class=\"" << (oddLine ? "odd" : "even") << "\" title=\""
        << YGP::TableWriter::changeHTMLSpecialChars(hFilm->getDescription()) << "\">";
    oddLine = !oddLine;
    for (std::string value; (value = getNextNode()).size();)
        out << "<td>" << value << "</td>";
    out << "</tr>\n";

    hFilm.reset();
    hDirector.reset();
}

//-----------------------------------------------------------------------------
/// Writes a director into the table
/// \param director: Director to write
/// \param out: Stream to write to
//-----------------------------------------------------------------------------
void FilmWriter::writeDirector(const HDirector& director, std::ostream& out) {
    contract_assert(!hFilm);
    contract_assert(!hDirector);
    hDirector = director;
    out << "<tr><td>&nbsp;" << rowEnd << "\n"
        << "<tr><td colspan=\"5\" class=\"owner\">" << hDirector->getName() << rowEnd;
    hDirector.reset();

    oddLine = true;
}

//-----------------------------------------------------------------------------
/// Appends the links to the language-flags for the passed languages
/// \param languages: List of languages (comma-separated)
/// \returns HTML-text of links to languages
//-----------------------------------------------------------------------------
std::string FilmWriter::addLanguageLinks(const std::string& languages) {
    std::string output;

    // Empty parts (e.g. from ",,") are skipped, like boost::char_separator did
    for (auto lang : languages | std::views::split(',') | std::views::transform([](auto&& part) {
                         return std::string_view(part);
                     }) | std::views::filter([](std::string_view part) { return !part.empty(); }))
        output += std::format("<img src=\"images/{0}.png\" alt=\"{0} \">", lang);
    return output;
}
#endif

#if WITH_RECORDS == 1
//-----------------------------------------------------------------------------
/// Substitution of column values
/// \param ctrl: Control character to subsitute
/// \returns std::string: Substituted string
//-----------------------------------------------------------------------------
std::string RecordWriter::getSubstitute(const char ctrl, bool) const {
    if (hRecord) {
        contract_assert(hInterpret);

        switch (ctrl) {
        case 'n':
            return YGP::TableWriter::changeHTMLSpecialChars(hRecord->getName());

        case 'y':
            return hRecord->getYear().toString();

        case 'g':
            contract_assert(hRecord->getGenre() < genres.size());
            return YGP::TableWriter::changeHTMLSpecialChars(genres.getGenre(hRecord->getGenre()));

        case 'd':
            return YGP::TableWriter::changeHTMLSpecialChars(hInterpret->getName());
        } // endswitch
    }
    else {
        contract_assert(hInterpret);
        contract_assert(!hRecord);

        if (ctrl == 'n')
            return YGP::TableWriter::changeHTMLSpecialChars(hInterpret->getName());
        return "";
    }
    return std::string(1, ctrl);
}

//-----------------------------------------------------------------------------
/// Writes a record into the table
/// \param record: Record to write
/// \param interpret: Interpret of record
/// \param out: Stream to write to
//-----------------------------------------------------------------------------
void RecordWriter::writeRecord(const HRecord& record, const HInterpret& interpret, std::ostream& out) {
    contract_assert(!hRecord);
    contract_assert(!hInterpret);
    hRecord = record;
    hInterpret = interpret;

    out << "<tr class=\"" << (oddLine ? "odd" : "even") << "\">";
    oddLine = !oddLine;
    for (std::string value; (value = getNextNode()).size();)
        out << "<td>" << value << "</td>";
    out << "</tr>\n";

    hRecord.reset();
    hInterpret.reset();
}

//-----------------------------------------------------------------------------
/// Writes a interpret into the table
/// \param interpret: Interpret to write
/// \param out: Stream to write to
//-----------------------------------------------------------------------------
void RecordWriter::writeInterpret(const HInterpret& interpret, std::ostream& out) {
    contract_assert(!hRecord);
    contract_assert(!hInterpret);
    hInterpret = interpret;
    out << "<tr><td>&nbsp;" << rowEnd << "\n"
        << "<tr><td colspan=\"3\" class=\"owner\">" << hInterpret->getName() << rowEnd;
    hInterpret.reset();

    oddLine = true;
}
#endif
