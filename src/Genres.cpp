// PROJECT     : CDManager
// SUBSYSTEM   : libCDMgr
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 13.01.2005
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

#include <sys/stat.h>

#include <algorithm>
#include <ranges>
#include <string>
#include <string_view>

#include <glibmm/convert.h>

#include <YGP/INIFile.h>
#include <YGP/Trace.h>

#include <XGP/XAttribute.h>

#include "Genres.h"

//-----------------------------------------------------------------------------
/// Loads the genres from a data-file. The parameter \languages specifies the
/// language to use.
/// \param file: File to load the data from
/// \param records: Object, to load the record genres into
/// \param film: Object, to load the film genres into
/// \param languages: Colon-separated list of languages
//-----------------------------------------------------------------------------
void Genres::loadFromFile(const char* file, Genres& records, Genres& films, const char* languages) {
    std::string name(file);

    // Check every (non-empty) language-entry (while removing trailing specifiers)
    struct stat sfile;
    for (auto&& lang : std::string_view(languages) | std::views::split(':') |
                           std::views::filter([](auto&& part) { return !std::ranges::empty(part); })) {
        std::string extension(std::from_range, lang);
        std::string search;
        do {
            search = name + std::string(1, '.') + extension;

            TRACE9("Genres::loadFromFile (...) - Trying " << search);
            if (!::stat(search.c_str(), &sfile) && (sfile.st_mode & S_IFREG))
                break;

            size_t pos(extension.rfind('_'));
            if (pos == std::string::npos)
                pos = 0;
            extension.replace(pos, extension.length(), 0, '\0');
        }
        while (extension.size());

        if (extension.size()) {
            TRACE1("Genres::loadFromFile (...) - Using " << search);
            name = search;
            break;
        }
    } // end-while

    YGP::INIFile _inifile_(name.c_str());
    YGP::INIList<Glib::ustring, std::vector<Glib::ustring>> lstFilms("Films", films.genres);
    _inifile_.addSection(lstFilms);
    YGP::INIList<Glib::ustring, std::vector<Glib::ustring>> lstRecords("Records", records.genres);
    _inifile_.addSection(lstRecords);

    _inifile_.read();
}

//-----------------------------------------------------------------------------
/// Gets the id of the passed genre
/// \param Name of genre to convert to its associated number
/// \returns int ID of genre or -1
//-----------------------------------------------------------------------------
int Genres::getId(const Glib::ustring& genre) const {
    const auto g(std::ranges::find(genres, genre));
    return (g != genres.end()) ? (g - genres.begin()) : -1;
}
