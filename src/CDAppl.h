#ifndef CDAPPL_H
#define CDAPPL_H

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

#include <YGP/IVIOAppl.h>

#include "Options.h"

/**VIO-Application part of the CD-manager; cares about reading the INI-file and
 * processing the options
 */
class CDAppl : public YGP::IVIOApplication {
  public:
    CDAppl(const int argc, const char* argv[]) : IVIOApplication(argc, argv, lo) {}
    ~CDAppl() override = default;

  protected:
    void readINIFile(const char* pFile) override;
    bool handleOption(const char option) override;

    // Program-handling
    [[nodiscard]] bool shallShowInfo() const override { return false; }
    int perform(int argc, const char* argv[]) override;
    [[nodiscard]] const char* name() const override { return PACKAGE_NAME; }
    [[nodiscard]] const char* description() const override;

    // Help-handling
    void showHelp() const override;

    // Prohobited manager functions
    CDAppl() = delete;
    CDAppl(const CDAppl&) = delete;
    const CDAppl& operator=(const CDAppl&) = delete;

  private:
    Options options;

    static const longOptions lo[];
};

#endif
