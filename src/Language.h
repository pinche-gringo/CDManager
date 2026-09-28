#ifndef LANGUAGE_H
#define LANGUAGE_H

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

#include <map>
#include <stdexcept>
#include <string>

#include <glibmm/ustring.h>

#ifdef USE_LANGUAGEPIXMAPS
#    include <gdkmm/pixbuf.h>
#endif

/**Class to manage languages
 */
struct Language {
  public:
    static void init();

    [[nodiscard]] static Glib::ustring findInternational(const std::string& lang);
    [[nodiscard]] static const Language& findLanguage(const std::string& lang);
    [[nodiscard]] static bool exists(const std::string& lang);
#ifdef USE_LANGUAGEPIXMAPS
    [[nodiscard]] static Glib::RefPtr<Gdk::Pixbuf> findFlag(const std::string& lang);
#endif

    [[nodiscard]] Glib::ustring getInternational() const { return nameInternational; }
#ifdef USE_LANGUAGEPIXMAPS
    [[nodiscard]] const Glib::RefPtr<Gdk::Pixbuf> getFlag() const { return flag; }
#endif

    Language() = default;
    Language(const Language& other) = default;
    Language(const Glib::ustring& internat
#ifdef USE_LANGUAGEPIXMAPS
             ,
             const Glib::RefPtr<Gdk::Pixbuf>& image
#endif
    );

    Language& operator=(const Language& other) = default;
    ~Language() = default;

    [[nodiscard]] static std::map<std::string, Language>::const_iterator begin() { return languages.begin(); }
    [[nodiscard]] static std::map<std::string, Language>::const_iterator end() { return languages.end(); }

  protected:
#ifdef USE_LANGUAGEPIXMAPS
    static Glib::RefPtr<Gdk::Pixbuf> loadFlag(const char* file) pre(file);
#endif

  private:
    Glib::ustring nameInternational;
#ifdef USE_LANGUAGEPIXMAPS
    Glib::RefPtr<Gdk::Pixbuf> flag;
#endif

    static std::map<std::string, Language> languages;
};

#endif
