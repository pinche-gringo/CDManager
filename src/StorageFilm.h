#ifndef STORAGEFILM_H
#define STORAGEFILM_H

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

#include <string>
#include <vector>

#include <YGP/Relation.h>

#include "Director.h"
#include "Film.h"

#include "Storage.h"

// Forward declarations
namespace YGP {
class StatusObject;
} // namespace YGP

/**Class to access the stored films
 */
class StorageFilm : public Storage {
  public:
    static void loadDirectors(std::vector<HDirector>& target, YGP::StatusObject& stat) {
        loadCelebrities(target, "Directors", stat);
    }
    static unsigned int loadFilms(std::map<unsigned int, std::vector<HFilm>>& aFilms, YGP::StatusObject& stat);
    static void saveFilm(const HFilm& film, unsigned int idDirector);
    static void deleteFilm(unsigned int idFilm);
    static void deleteDirector(unsigned int idDirector);
    static void deleteFilmNames(unsigned int idFilm);
    static void saveFilmName(const HFilm& film, const std::string& lang);

    static void loadNames(const std::vector<HDirector>& directors, const YGP::Relation1_N<HDirector, HFilm>& relFilms,
                          const std::string& lang);

  private:
    StorageFilm() = delete;
    StorageFilm(const StorageFilm& other) = delete;
    ~StorageFilm() override;

    const StorageFilm& operator=(const StorageFilm& other) = delete;
};

#endif
