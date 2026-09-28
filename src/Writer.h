#ifndef WRITER_H
#define WRITER_H

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

#include <string>

#if WITH_FILMS == 1
#    include "Director.h"
#    include "Film.h"
#endif
#if WITH_RECORDS == 1
#    include "Interpret.h"
#    include "Record.h"
#endif

#include "Genres.h"
#include "Options.h"

#include <YGP/TableWriter.h>

#if WITH_FILMS == 1
/**Class to write films as HTML-tables
 */
class FilmWriter : public YGP::TableWriter {
  public:
    FilmWriter(const std::string& format, Genres& genres)
        : YGP::TableWriter(format, TBLW_HTML_PARAMS), genres(genres) {}
    ~FilmWriter() override = default;

    void writeFilm(const HFilm& film, const HDirector& director, std::ostream& out);
    void writeDirector(const HDirector& director, std::ostream& out);

  protected:
    std::string getSubstitute(const char ctrl, bool extend = false) const override;

  private:
    FilmWriter(const FilmWriter& other) = delete;
    const FilmWriter& operator=(const FilmWriter& other) = delete;

    static std::string addLanguageLinks(const std::string& languages);

    HFilm hFilm;
    HDirector hDirector;

    bool oddLine{true};
    Genres& genres;
};
#endif

#if WITH_RECORDS == 1
/**Class to write records as HTML-tables
 */
class RecordWriter : public YGP::TableWriter {
  public:
    RecordWriter(const std::string& format, Genres& genres)
        : YGP::TableWriter(format, TBLW_HTML_PARAMS), genres(genres) {}
    ~RecordWriter() override = default;

    void writeRecord(const HRecord& record, const HInterpret& interpret, std::ostream& out);
    void writeInterpret(const HInterpret& interpret, std::ostream& out);

  protected:
    std::string getSubstitute(const char ctrl, bool extend = false) const override;

  private:
    RecordWriter(const RecordWriter& other) = delete;
    const RecordWriter& operator=(const RecordWriter& other) = delete;

    HRecord hRecord;
    HInterpret hInterpret;

    bool oddLine{true};
    Genres& genres;
};
#endif

#endif
