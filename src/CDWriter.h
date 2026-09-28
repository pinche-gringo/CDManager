#ifndef CDWRITER_H
#define CDWRITER_H

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

#include "Options.h"

#include <YGP/IVIOAppl.h>

/**Application to write records and films to HTML pages.
 *
 *The data to write to is read from stdin.
 */
class CDWriter : public YGP::IVIOApplication {
  public:
    CDWriter(const int argc, const char* argv[]) : IVIOApplication(argc, argv, lo) {}
    ~CDWriter() override;

  protected:
    bool handleOption(const char option) override;

    // Program-handling
    bool shallShowInfo() const override { return false; }
    int perform(int argc, const char* argv[]) override;
    const char* name() const override { return "CDWriter"; }
    const char* description() const override;

    // Help-handling
    void showHelp() const override;

  private:
    // Prohobited manager functions
    CDWriter() = delete;
    CDWriter(const CDWriter&) = delete;
    const CDWriter& operator=(const CDWriter&) = delete;

    int createFile(const std::string& name, const char* lang, std::ofstream& file) pre(!name.empty()) pre(lang != nullptr);
    static bool readHeaderFile(const char* file, const char* lang, std::string& target, const Glib::ustring& title)
        pre(file != nullptr) pre(lang != nullptr);

    static void writeHeader(const char* lang, const char* format, std::ostream& stream, bool upSorted = true,
                            const char* lead = "Films") pre(lang != nullptr) pre(format != nullptr) pre(lead != nullptr);

    static const longOptions lo[];

    Options opt;
};

#endif
