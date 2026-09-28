// PROJECT     : CDManager
// SUBSYSTEM   : Records
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 24.01.2006
// COPYRIGHT   : Copyright (C) 2006, 2007, 2009 - 2011, 2026

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

#include <unistd.h>

#include <cstring>

#ifdef HAVE_ICONV
#    include <iconv.h>
#endif

#include <algorithm>
#include <fstream>
#include <memory>
#include <sstream>
#include <string_view>

#include <giomm/file.h>

#include <glibmm/convert.h>

#include <gtkmm/paned.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/statusbar.h>

#include <YGP/ANumeric.h>
#include <YGP/StatusObj.h>
#include <YGP/Trace.h>

#include <XGP/MessageDlg.h>
#include <XGP/XFileDlg.h>

#include "SaveCeleb.h"
#include "StorageRecord.h"

#include "PRecords.h"

//-----------------------------------------------------------------------------
/// Reads an IDv3 size object (4 bytes; only 7 bits used)
//-----------------------------------------------------------------------------
static unsigned int getID3Size(const char* buffer) { return (*buffer << 21) + (buffer[1] << 14) + (buffer[2] << 7) + buffer[3]; }

//-----------------------------------------------------------------------------
/// Constructor: Creates a widget handling records/songs
/// \param status Statusbar to display status-messages
/// \param menuSave Menu-entry to save the database
/// \param genres Genres to use in actor-list
//-----------------------------------------------------------------------------
PRecords::PRecords(Gtk::Statusbar& status, Glib::RefPtr<Gio::SimpleAction> menuSave, const Genres& genres)
    : NBPage(status, menuSave), records(genres), songs(genres), relRecords("records"), relSongs("songs") {
    TRACE9("PRecords::PRecords (Gtk::Statusbar&, Glib::RefPtr<Gio::SimpleAction>, const Genres&)");

    auto* cds(new Gtk::Paned(Gtk::Orientation::HORIZONTAL));
    auto* scrlRecords(Gtk::make_managed<Gtk::ScrolledWindow>());
    auto* scrlSongs(Gtk::make_managed<Gtk::ScrolledWindow>());

    scrlRecords->set_has_frame(true);
    scrlSongs->set_has_frame(true);
    scrlRecords->set_child(records);
    scrlSongs->set_child(songs);
    scrlRecords->set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);
    scrlSongs->set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);

    Glib::RefPtr<Gtk::TreeSelection> sel(songs.get_selection());
    sel->set_mode(Gtk::SelectionMode::MULTIPLE);
    sel->signal_changed().connect(sigc::mem_fun(*this, &PRecords::songSelected));
    songs.signalChanged.connect(sigc::mem_fun(*this, &PRecords::songChanged));

    records.signalOwnerChanged.connect(sigc::mem_fun(*this, &PRecords::interpretChanged));
    records.signalObjectChanged.connect(sigc::mem_fun(*this, &PRecords::recordChanged));

    sel = records.get_selection();
    sel->set_mode(Gtk::SelectionMode::MULTIPLE);
    sel->signal_changed().connect(sigc::mem_fun(*this, &PRecords::recordSelected));

    cds->set_start_child(*scrlRecords);
    cds->set_end_child(*scrlSongs);
    cds->set_position(400);

    widget = cds;
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
PRecords::~PRecords() { TRACE9("PRecords::~PRecords ()"); }

//-----------------------------------------------------------------------------
/// Loads the records from the database
///
/// According to the available information the pages of the notebook
/// are created.
//-----------------------------------------------------------------------------
void PRecords::loadData() {
    TRACE9("PRecords::loadData ()");
    try {
        YGP::StatusObject status;
        StorageRecord::loadInterprets(interprets, status);
        std::ranges::sort(interprets, &Interpret::compByName);

        std::map<unsigned int, std::vector<HRecord>> aRecords;
        const unsigned int cRecords(StorageRecord::loadRecords(aRecords, status));
        TRACE8("PRecords::loadData () - Found " << aRecords.size() << " records");

        for (const auto& artist : interprets) {
            Gtk::TreeModel::Row interpret(records.append(artist));

            if (auto iRec(aRecords.find(artist->getId())); iRec != aRecords.end()) {
                for (auto& record : iRec->second) {
                    records.append(record, interpret);
                    relRecords.relate(artist, record);
                }
                aRecords.erase(iRec);
            } // end-if artist has record
        } // end-for all artists
        records.expand_all();

        loaded = true;

        Glib::ustring msg(Glib::ustring::compose(Glib::locale_to_utf8(ngettext("Loaded %1 record", "Loaded %1 records", cRecords)),
                                                 Glib::ustring(YGP::ANumeric::toString(cRecords))));
        msg += Glib::ustring::compose(Glib::locale_to_utf8(ngettext(" from %1 artist", " from %1 artists", interprets.size())),
                                      Glib::ustring(YGP::ANumeric::toString(interprets.size())));
        showStatus(msg);

        if (status.getType() > YGP::StatusObject::UNDEFINED) {
            status.generalize(_("Warnings loading records"));
            XGP::MessageDlg::create(status);
        }
    }
    catch (const std::exception& err) {
        showError(Glib::ustring::compose(_("Can't query available records!\n\nReason: %1"), err.what()));
    }
}

//-----------------------------------------------------------------------------
/// Loads the songs for the passed record
/// \param record Handle to the record for which to load songs
//-----------------------------------------------------------------------------
void PRecords::loadSongs(const HRecord& record) {
    TRACE9("PRecords::loadSongs (const HRecord& record) - " << (record ? record->getName().c_str() : "Undefined"));

    try {
        std::vector<HSong> songs_;
        StorageRecord::loadSongs(record->getId(), songs_);
        TRACE5("PRecords::loadSongs (const HRecord& record) - Found songs: " << songs_.size());

        if (!songs_.empty())
            relSongs.relate(record, songs_);
        record->setSongsLoaded();
    }
    catch (const std::exception& err) {
        showError(Glib::ustring::compose(_("Can't query the songs for record %1!\n\nReason: %2"), record->getName(), err.what()));
    }
}

//-----------------------------------------------------------------------------
/// Adds a new interpret to the list
//-----------------------------------------------------------------------------
void PRecords::newInterpret() {
    addInterpret(std::make_shared<Interpret>());
}

//-----------------------------------------------------------------------------
/// Adds a new record to the first selected interpret
//-----------------------------------------------------------------------------
void PRecords::newRecord() {
    Glib::RefPtr<Gtk::TreeSelection> recordSel(records.get_selection());
    const std::vector<Gtk::TreePath> list(recordSel->get_selected_rows());
    contract_assert(!list.empty());
    Glib::RefPtr<Gtk::TreeStore> model(records.getModel());
    Gtk::TreeModel::iterator p(model->get_iter(list.front()));
    contract_assert(p);
    if (p->parent())
        p = p->parent();

    auto record(std::make_shared<Record>());
    record->setSongsLoaded();
    addRecord(p, record);
}

//-----------------------------------------------------------------------------
/// Adds a new song to the first selected record
//-----------------------------------------------------------------------------
void PRecords::newSong() {
    auto song(std::make_shared<Song>());
    addSong(song);
}

//-----------------------------------------------------------------------------
/// Callback after selecting a record
/// \param row Selected row
//-----------------------------------------------------------------------------
void PRecords::recordSelected() {
    TRACE9("PRecords::recordSelected ()");
    songs.clear();
    contract_assert(records.get_selection());
    const std::vector<Gtk::TreePath> list(records.get_selection()->get_selected_rows());
    TRACE9("PRecords::recordSelected () - Size: " << list.size());
    if (!list.empty()) {
        Gtk::TreeModel::iterator i(records.get_model()->get_iter(list.front()));
        contract_assert(i);

        if (i->parent()) {
            HRecord hRecord(records.getRecordAt(i));
            contract_assert(hRecord);
            if (hRecord->needsLoading() && hRecord->getId())
                loadSongs(hRecord);
            contract_assert(!hRecord->needsLoading());

            // Add related songs to the listbox
            if (relSongs.isRelated(hRecord))
                for (HSong song : relSongs.getObjects(hRecord))
                    songs.append(song);

            enableEdit(OBJECT_SELECTED);
        }
        else
            enableEdit(OWNER_SELECTED);
    }
    else
        enableEdit(NONE_SELECTED);
}

//-----------------------------------------------------------------------------
/// Callback after selecting a song
/// \param row Selected row
//-----------------------------------------------------------------------------
void PRecords::songSelected() {
    TRACE9("PRecords::songSelected ()");
    contract_assert(songs.get_selection());
    const std::vector<Gtk::TreePath> list(songs.get_selection()->get_selected_rows());
    TRACE9("PRecords::songSelected () - Size: " << list.size());
    apMenus[DELETE]->set_enabled(!list.empty());
}

//-----------------------------------------------------------------------------
/// Callback when a song is being changed
/// \param row Changed line
/// \param column Changed column
/// \param oldValue Old value of the changed entry
//-----------------------------------------------------------------------------
void PRecords::songChanged(const Gtk::TreeModel::iterator& row, unsigned int column, Glib::ustring& oldValue) {
    TRACE4("PRecords::songChanged (const Gtk::TreeModel::iterator&, unsigned int, Glib::ustring&) - " << column);

    const std::vector<Gtk::TreePath> list(records.get_selection()->get_selected_rows());
    TRACE9("PRecords::songChanged (const Gtk::TreeModel::iterator&, unsigned int, Glib::ustring&) - Selected: " << list.size());
    aUndo.push(Undo(Undo::CHANGED, SONG, column, songs.getEntryAt(row), list.front(), oldValue));

    apMenus[UNDO]->set_enabled();
    enableSave();
}

//-----------------------------------------------------------------------------
/// Callback when changing an interpret
/// \param row Changed line
/// \param column Changed column
/// \param oldValue Old value of the changed entry
//-----------------------------------------------------------------------------
void PRecords::interpretChanged(const Gtk::TreeModel::iterator& row, unsigned int column, Glib::ustring& oldValue) {
    TRACE9("PRecords::interpretChanged (const Gtk::TreeModel::iterator&, unsigned int, Glib::ustring&) - " << column);

    aUndo.push(Undo(Undo::CHANGED, INTERPRET, column, records.getCelebrityAt(row), records.getModel()->get_path(row), oldValue));

    apMenus[UNDO]->set_enabled();
    enableSave();
}

//-----------------------------------------------------------------------------
/// Callback when changing a record
/// \param row Changed line
/// \param column Changed column
/// \param oldValue Old value of the changed entry
//-----------------------------------------------------------------------------
void PRecords::recordChanged(const Gtk::TreeModel::iterator& row, unsigned int column, Glib::ustring& oldValue) {
    TRACE9("PRecords::recordChanged (const Gtk::TreeModel::iterator&, unsigned int, Glib::ustring&) - " << column);

    Gtk::TreePath path(records.getModel()->get_path(row));
    aUndo.push(Undo(Undo::CHANGED, RECORD, column, records.getObjectAt(row), path, oldValue));

    if (column == 2) { // If the record-genre was changed, copy it for songs
        HRecord rec(records.getRecordAt(row));
        TRACE9("PRecords::recordChanged (const HEntity& record) - " << (rec ? rec->getId() : -1UL) << '/'
                                                                    << (rec ? rec->getName().c_str() : "Undefined"));
        contract_assert(oldValue.size() == 1);

        if (relSongs.isRelated(rec)) {
            const unsigned int genre(rec->getGenre());

            Glib::RefPtr<Gtk::TreeModel> model(songs.get_model());
            if (const std::vector<Gtk::TreePath> list(songs.get_selection()->get_selected_rows()); !list.empty()) {
                for (const auto& selected : list)
                    songs.setGenre(model->get_iter(selected), genre);
            }
            else {
                Gtk::TreeModel::Children children(model->children());
                contract_assert(!children.empty());
                for (auto i(children.begin()); i != children.end(); ++i)
                    songs.setGenre(i, genre);
            }
        }
    }

    apMenus[UNDO]->set_enabled();
    enableSave();
}

//-----------------------------------------------------------------------------
/// Setting the page-specific menu
/// \param menuEdit Edit-menu to add the page-specific entries to
/// \param menuOther Additional top-level menus (not used)
/// \param grpAction Action-group to add the actions to
/// \param shortcuts Controller to add the keyboard shortcuts to
//-----------------------------------------------------------------------------
void PRecords::addMenu(Glib::RefPtr<Gio::Menu> menuEdit, Glib::RefPtr<Gio::Menu>, Glib::RefPtr<Gio::SimpleActionGroup> grpAction,
                       Glib::RefPtr<Gtk::ShortcutController> shortcuts) {
    TRACE7("PRecords::addMenu");

    Glib::RefPtr<Gio::Menu> sec(Gio::Menu::create());
    apMenus[UNDO] = grpAction->add_action("RUndo", sigc::mem_fun(*this, &PRecords::undo));
    addMenuEntry(sec, _("_Undo"), "page.RUndo", _("<ctl>Z"), shortcuts);
    menuEdit->append_section(sec);

    sec = Gio::Menu::create();
    apMenus[NEW1] = grpAction->add_action("NInterpret", sigc::mem_fun(*this, &PRecords::newInterpret));
    addMenuEntry(sec, _("New _interpret"), "page.NInterpret", _("<ctl>N"), shortcuts);
    apMenus[NEW2] = grpAction->add_action("NRecord", sigc::mem_fun(*this, &PRecords::newRecord));
    addMenuEntry(sec, _("_New record"), "page.NRecord", _("<ctl><alt>N"), shortcuts);
    apMenus[NEW3] = grpAction->add_action("NSong", sigc::mem_fun(*this, &PRecords::newSong));
    addMenuEntry(sec, _("New _song"), "page.NSong", _("<ctl><shft>N"), shortcuts);
    menuEdit->append_section(sec);

    sec = Gio::Menu::create();
    apMenus[DELETE] = grpAction->add_action("RDelete", sigc::mem_fun(*this, &PRecords::deleteSelection));
    addMenuEntry(sec, _("_Delete"), "page.RDelete", _("<ctl>Delete"), shortcuts);
    menuEdit->append_section(sec);

    sec = Gio::Menu::create();
    grpAction->add_action("Import", sigc::mem_fun(*this, &PRecords::importFromFileInfo));
    addMenuEntry(sec, _("_Import from file-info ..."), "page.Import", _("<ctl>I"), shortcuts);
    menuEdit->append_section(sec);

    apMenus[UNDO]->set_enabled(false);
    recordSelected();
}

//-----------------------------------------------------------------------------
/// Imports information from audio file (e.g. MP3-ID3 tag or OGG-commentheader)
//-----------------------------------------------------------------------------
void PRecords::importFromFileInfo() {
    XGP::FileDialog* dlg(XGP::FileDialog::create(_("Select file(s) to import"), Gtk::FileChooser::Action::OPEN,
                                                 XGP::FileDialog::MUST_EXIST | XGP::FileDialog::MULTIPLE));
    dlg->set_current_folder(Gio::File::create_for_path("/usr/local/Music/K/Käthecore/EKH-Sampler"));
    dlg->sigSelected.connect(sigc::mem_fun(*this, &PRecords::parseFileInfo));
}

//-----------------------------------------------------------------------------
/// Reads the ID3 information from a MP3 file
/// \param file Name of file to analzye
//-----------------------------------------------------------------------------
void PRecords::parseFileInfo(const std::string& file) {
    TRACE8("PRecords::parseFileInfo (const std::string&) - " << file);

    std::ifstream stream(file);
    Glib::ustring artist, record, song, genre;
    unsigned int track(0), year(0);
    if (!stream) {
        showError(Glib::ustring::compose(_("Can't open file `%1'!\n\nReason: %2"), Glib::ustring(file), Glib::ustring(strerror(errno))));
        return;
    }

    const std::string extension(file.substr(file.size() - 4));
    TRACE1("PRecords::parseFileInfo (const std::string&) - Type: " << extension);
    if (((extension == ".mp3") && parseID3Info(stream, artist, record, song, track, genre, year)) ||
        ((extension == ".ogg") && parseOGGCommentHeader(stream, artist, record, song, track, genre, year))) {
        TRACE8("PRecords::parseFileInfo (const std::string&) - " << artist << '/' << record << '/' << song << '/' << track);
        addEntry(artist, record, song, track, genre, year);
    }
}

//-----------------------------------------------------------------------------
/// Reads the ID3 information from a MP3 file
/// \param stream MP3-file to analyze
/// \param artist Found artist
/// \param record Found record name
/// \param song Found song
/// \param track Tracknumber
/// \param genre Genre
/// \param year Year of record
/// \returns bool: True, if ID3 info has been found
//-----------------------------------------------------------------------------
bool PRecords::parseID3Info(std::istream& stream, Glib::ustring& artist, Glib::ustring& record, Glib::ustring& song,
                            unsigned int& track, Glib::ustring& genre, unsigned int& year) {
    char buffer[512];
    stream.read(buffer, 4);

    // Check if an ID3v2 tag is present
    if (std::string_view(buffer, 3) != "ID3") {
        stream.seekg(-0x80, std::ios::end);
        std::string value;

        // If not: Check for ID3v1
        std::getline(stream, value, '\xff');
        TRACE8("PRecords::parseID3Info (std::istream&, 3x Glib::ustring&, unsigned&) - Found: " << value
                                                                                                << "; Length: " << value.size());
        if ((value.size() > 3) && value.starts_with("TAG")) {
            song = Glib::locale_to_utf8(stripString(value, 3, 29));
            artist = Glib::locale_to_utf8(stripString(value, 33, 29));
            record = Glib::locale_to_utf8(stripString(value, 63, 29));
            track = (value[0x7d] != 0x20) ? value[0x7e] : 0;
            return true;
        }
    }
    else {
        stream.read(buffer, 6);
        unsigned int size(getID3Size(buffer + 2));
        TRACE7("PRecords::parseID3Info (std::istream&, 3x Glib::ustring&, unsigned&) - ID3-tag " << size);

        // Skip extended header, if present
        if (buffer[1] & 0x40) {
            stream.read(buffer, 4);
            unsigned int extSize(getID3Size(buffer));
            TRACE9("PRecords::parseID3Info (std::istream&, 3x Glib::ustring&, unsigned&) - Extended header " << extSize);
            if (size < (extSize + 4))
                return false;

            stream.seekg(extSize - 4, std::ios::cur);
        }

        do {
            Glib::ustring* value(nullptr);
            // Read all frames
            stream.read(buffer, 10);
            unsigned int frameSize(getID3Size(buffer + 4));
            if ((size - frameSize) < 4)
                return false;
            size -= 10 + frameSize;

            const std::string_view frameID(buffer, 4);
            if (frameID == std::string_view("\0\0\0\0", 4))
                break;

            TRACE8("PRecords::parseID3Info (std::istream&, 3x Glib::ustring&, unsigned&) - ID3-frame " << frameID << '/'
                                                                                                      << frameSize);
            if (frameID == "TIT2")
                value = &song;
            else if (frameID == "TALB")
                value = &record;
            else if (frameID == "TPE1")
                value = &artist;
            else if (frameID == "TCON")
                value = &genre;
            else if (frameID == "TDRC") {
                contract_assert(frameSize < sizeof(buffer));
                stream.read(buffer, frameSize);
                year = strtoul(buffer + 1, nullptr, 10);
            }
            else if (frameID == "TRCK") {
                contract_assert(frameSize < sizeof(buffer));
                stream.read(buffer, frameSize);
                track = strtoul(buffer + 1, nullptr, 10);
            }
            else
                stream.seekg(frameSize, std::ios::cur);

            if (value) {
                unsigned int read(0);
                char type(stream.get());
                --frameSize;

#ifdef HAVE_ICONV
                // Encode to UTF-8
                const char* encoding("UTF16");
                iconv_t cd((iconv_t)(-1));

                switch (type) {
                case 0: // ISO-8859-1
                    encoding = "ISO-8859-1";
                    [[fallthrough]];

                case 1: // UTF-16
                case 2:
                    cd = iconv_open("UTF8", encoding);
                    break;

                default: // type == 3 is already UTF-8; ignore all other values
                    break;
                }
#endif

                do {
                    read = stream.readsome(buffer, (frameSize > sizeof(buffer)) ? sizeof(buffer) - 1 : frameSize);
                    TRACE8("PRecords::parseID3Info (std::istream&, 3x Glib::ustring&, unsigned&) - Read " << read);
                    TRACE8("PRecords::parseID3Info (std::istream&, 3x Glib::ustring&, unsigned&) - Read "
                           << std::string(buffer, read));
                    frameSize -= read;
                    TRACE8("PRecords::parseID3Info (std::istream&, 3x Glib::ustring&, unsigned&) - Left " << frameSize);

#ifdef HAVE_ICONV
                    if (cd != (iconv_t)(-1)) {
                        const auto converted(std::make_unique_for_overwrite<char[]>(read));
                        size_t inLeft(size);
                        while (inLeft) {
                            size_t outLeft(read);
                            char* curPos(converted.get());
                            size_t conv(iconv(cd, (char**)&buffer, &inLeft, &curPos, &outLeft));
                            TRACE8("PRecords::parseID3Info (std::istream&, 3x Glib::ustring&, unsigned&) - Converted " << conv);
                            if (conv)
                                value->append(converted.get(), conv);
                        }
                    }
                    else
                        value->append(std::string(buffer, read));
#else
                    value->append(buffer, read);
#endif
                }
                while (frameSize);

#ifdef HAVE_ICONV
                iconv_close(cd);
#endif
            }
            TRACE8("PRecords::parseID3Info (std::istream&, 3x Glib::ustring&, unsigned&) - ID3 left " << size);
        }
        while (size);
    }
    return true;
}

//-----------------------------------------------------------------------------
/// Reads the OGG comment header from an OGG vorbis encoded file
/// \param stream OGG-file to analyze
/// \param artist Found artist
/// \param record Found record name
/// \param song Found song
/// \param track Tracknumber
/// \param genre Genre
/// \param year Year of record
/// \returns bool: True, if comment header has been found
//-----------------------------------------------------------------------------
bool PRecords::parseOGGCommentHeader(std::istream& stream, Glib::ustring& artist, Glib::ustring& record, Glib::ustring& song,
                                     unsigned int& track, [[maybe_unused]] Glib::ustring& genre,
                                     [[maybe_unused]] unsigned int& year) {
    char buffer[512];
    stream.read(buffer, 4);
    if (std::string_view(buffer, 4) != "OggS")
        return false;

    stream.seekg(0x69, std::ios::cur);
    unsigned int len(0);
    stream.read(reinterpret_cast<char*>(&len), 4); // Read the vendorstring-length
    TRACE8("PRecords::parseOGGCommentHeader (std::istream&, 3x Glib::ustring&, unsigned&) - Length: " << len);
    stream.seekg(len, std::ios::cur);

    unsigned int cComments(0);
    stream.read(reinterpret_cast<char*>(&cComments), 4); // Read number of comments
    TRACE8("PRecords::parseOGGCommentHeader (std::istream&, 3x Glib::ustring&, unsigned&) - Comments: " << cComments);
    if (!cComments)
        return false;

    std::string key;
    Glib::ustring* value(nullptr);
    do {
        stream.read(reinterpret_cast<char*>(&len), 4); // Read the comment-length

        std::getline(stream, key, '=');
        len -= key.size() + 1;
        TRACE8("PRecords::parseOGGCommentHeader (std::stream&, 3x Glib::ustring&, unsigned&) - Key: " << key);

        if (key == "TITLE")
            value = &song;
        else if (key == "ALBUM")
            value = &record;
        else if (key == "ARTIST")
            value = &artist;
        else if (key == "TRACKNUMBER") {
            contract_assert(len < sizeof(buffer));
            stream.read(buffer, len);
            track = strtoul(buffer, nullptr, 10);
            value = nullptr;
            len = 0;
        }
        else
            value = nullptr;

        if (value) {
            unsigned int read(0);
            do {
                read = stream.readsome(buffer, (len > sizeof(buffer)) ? sizeof(buffer) - 1 : len);
                len -= read;
                buffer[read] = '\0';
                value->append(buffer);
            }
            while (len);
        }
        else
            stream.seekg(len, std::ios::cur);
    }
    while (--cComments); // end-do while comments
    return true;
}

//-----------------------------------------------------------------------------
/// Returns the specified substring, removed from trailing spaces
/// \param value String to manipulate
/// \param pos Starting pos inside the string
/// \param len Maximal length of string
/// \returns std::string Stripped value
//-----------------------------------------------------------------------------
std::string PRecords::stripString(const std::string& value, unsigned int pos, unsigned int len) {
    len += pos;
    while (len > pos) {
        if ((value[len] != ' ') && (value[len]))
            break;
        --len;
    }
    return (pos == len) ? " " : value.substr(pos, len - pos + 1);
}

//-----------------------------------------------------------------------------
/// Adds an interpret to the record listbox
/// \param interpret Handle to the new interpret
/// \returns Gtk::TreeModel::iterator Iterator to new added interpret
//-----------------------------------------------------------------------------
Gtk::TreeModel::iterator PRecords::addInterpret(const HInterpret& interpret) {
    interprets.push_back(interpret);

    Gtk::TreeModel::iterator i(records.append(interpret).get_iter());
    Gtk::TreePath path(records.getModel()->get_path(i));
    records.set_cursor(path, *records.get_column(0), true);

    aUndo.push(Undo(Undo::INSERT, INTERPRET, 0, interpret, path, ""));
    apMenus[UNDO]->set_enabled();
    enableSave();
    return i;
}

//-----------------------------------------------------------------------------
/// Adds a record to the record listbox
/// \param parent Iterator to the interpret of the record
/// \param record Handle to the new record
/// \returns Gtk::TreeModel::iterator Iterator to new added record
//-----------------------------------------------------------------------------
Gtk::TreeModel::iterator PRecords::addRecord(const Gtk::TreeModel::iterator& parent, HRecord& record) {
    Gtk::TreeModel::iterator i(records.append(record, *parent).get_iter());
    Glib::RefPtr<Gtk::TreeStore> model(records.getModel());
    records.expand_row(model->get_path(parent), false);
    Gtk::TreePath path(records.getModel()->get_path(i));
    records.set_cursor(path, *records.get_column(0), true);

    relRecords.relate(records.getInterpretAt(parent), record);

    aUndo.push(Undo(Undo::INSERT, RECORD, 0, record, path, ""));
    apMenus[UNDO]->set_enabled();
    enableSave();
    return i;
}

//-----------------------------------------------------------------------------
/// Adds a song to the song listbox
/// \param song Handle to the new song
/// \returns Gtk::TreeModel::iterator Iterator to the record of the song
//-----------------------------------------------------------------------------
Gtk::TreeModel::iterator PRecords::addSong(HSong& song) {
    Glib::RefPtr<Gtk::TreeSelection> recordSel(records.get_selection());
    const std::vector<Gtk::TreePath> list(recordSel->get_selected_rows());
    contract_assert(!list.empty());
    Gtk::TreeModel::iterator p(records.getModel()->get_iter(list.front()));
    contract_assert(p);

    HRecord record(records.getRecordAt(p));
    contract_assert(record);
    relSongs.relate(record, song);
    Gtk::TreeModel::iterator iterSong(songs.append(song));
    Gtk::TreePath pathSong(songs.getModel()->get_path(iterSong));
    songs.set_cursor(pathSong, *songs.get_column(0), true);

    aUndo.push(Undo(Undo::INSERT, SONG, 0, song, list.front(), ""));

    apMenus[UNDO]->set_enabled();
    enableSave();
    return p;
}

//-----------------------------------------------------------------------------
/// Saves the changed information
/// \throw std::exception: In case of error
//-----------------------------------------------------------------------------
void PRecords::saveData() {
    TRACE5("PRecords::saveData () - " << aUndo.size());

    std::vector<HEntity> aSaved;
    auto posSaved(aSaved.end());

    while (!aUndo.empty()) {
        Undo last(aUndo.top());
        TRACE7("PRecords::saveData () - What: " << last.what() << '/' << last.how());

        posSaved = std::ranges::lower_bound(aSaved, last.getEntity());
        if ((posSaved == aSaved.end()) || (*posSaved != last.getEntity())) {
            switch (last.what()) {
            case SONG: {
                HSong song(std::dynamic_pointer_cast<Song>(last.getEntity()));
                contract_assert(song);
                if (last.how() == Undo::DELETE) {
                    if (song->getId()) {
                        contract_assert(song->getId() == last.column());
                        StorageRecord::deleteSong(song->getId());
                    }

                    auto delRel(delRelation.find(last.getEntity()));
                    contract_assert(delRel != delRelation.end());
                    contract_assert(typeid(*delRel->second) == typeid(Record));
                    delRelation.erase(delRel);
                }
                else {
                    HRecord hRec(relSongs.getParent(song));
                    if (!hRec->getId()) {
                        contract_assert(std::ranges::find(aSaved, hRec) == aSaved.end());
                        contract_assert(!delRelation.contains(hRec));

                        HInterpret interpret(relRecords.getParent(hRec));
                        if (!interpret->getId()) {
                            contract_assert(std::ranges::find(aSaved, interpret) == aSaved.end());
                            contract_assert(!delRelation.contains(interpret));

                            SaveCelebrity::store(interpret, "Interprets", *getWindow());
                            aSaved.insert(std::ranges::lower_bound(aSaved, interpret), interpret);
                        }
                        StorageRecord::saveRecord(hRec, relRecords.getParent(hRec)->getId());
                        aSaved.insert(std::ranges::lower_bound(aSaved, hRec), hRec);
                        posSaved = std::ranges::lower_bound(aSaved, last.getEntity());
                    }
                    StorageRecord::saveSong(song, hRec->getId());
                }
                break;
            }

            case RECORD: {
                contract_assert(typeid(*last.getEntity()) == typeid(Record));
                HRecord rec(std::dynamic_pointer_cast<Record>(last.getEntity()));
                if (last.how() == Undo::DELETE) {
                    if (rec->getId()) {
                        contract_assert(rec->getId() == last.column());
                        StorageRecord::deleteRecord(rec->getId());
                    }

                    auto delRel(delRelation.find(last.getEntity()));
                    contract_assert(delRel != delRelation.end());
                    contract_assert(typeid(*delRel->second) == typeid(Interpret));
                    delRelation.erase(delRel);
                }
                else {
                    HInterpret interpret(relRecords.getParent(rec));
                    if (!interpret->getId()) {
                        contract_assert(std::ranges::find(aSaved, interpret) == aSaved.end());
                        contract_assert(!delRelation.contains(interpret));

                        SaveCelebrity::store(interpret, "Interprets", *getWindow());
                        aSaved.insert(std::ranges::lower_bound(aSaved, interpret), interpret);
                        posSaved = std::ranges::lower_bound(aSaved, last.getEntity());
                    }
                    StorageRecord::saveRecord(rec, interpret->getId());
                }
                break;
            }

            case INTERPRET: {
                contract_assert(typeid(*last.getEntity()) == typeid(Interpret));
                HInterpret interpret(std::dynamic_pointer_cast<Interpret>(last.getEntity()));
                if (last.how() == Undo::DELETE) {
                    if (interpret->getId()) {
                        contract_assert(interpret->getId() == last.column());
                        StorageRecord::deleteInterpret(interpret->getId());
                    }
                }
                else
                    SaveCelebrity::store(interpret, "Interprets", *getWindow());
                break;
            }

            default:
                contract_assert(false);
            } // end-switch

            aSaved.insert(posSaved, last.getEntity());
        }
        aUndo.pop();
    } // end-while
    contract_assert(apMenus[UNDO]);
    apMenus[UNDO]->set_enabled(false);

    contract_assert(delRelation.empty());
}

//-----------------------------------------------------------------------------
/// Removes the selected records or interprets from the listbox. Depending objects
/// are deleted too.
//-----------------------------------------------------------------------------
void PRecords::deleteSelection() {
    if (records.has_focus())
        deleteSelectedRecords();
    else if (songs.has_focus())
        deleteSelectedSongs();

    apMenus[UNDO]->set_enabled();
    enableSave();
}

//-----------------------------------------------------------------------------
/// Removes the selected records or artists from the listbox. Depending objects
/// (records or songs) are deleted too.
//-----------------------------------------------------------------------------
void PRecords::deleteSelectedRecords() {
    TRACE9("PRecords::deleteSelectedRecords ()");

    Glib::RefPtr<Gtk::TreeSelection> selection(records.get_selection());
    while (!selection->get_selected_rows().empty()) {
        const std::vector<Gtk::TreePath> list(selection->get_selected_rows());
        contract_assert(!list.empty());

        Gtk::TreeModel::iterator iter(records.get_model()->get_iter(list.front()));
        contract_assert(iter);
        if (iter->parent()) // A record is going to be deleted
            deleteRecord(iter);
        else { // An interpret is going to be deleted
            TRACE9("PRecords::deleteSelectedRecords () - Deleting " << iter->children().size() << " children");
            HInterpret interpret(records.getInterpretAt(iter));
            contract_assert(interpret);
            while (!iter->children().empty()) {
                Gtk::TreeModel::iterator child(iter->children().begin());
                HRecord hRecord(records.getRecordAt(child));
                if (hRecord->needsLoading() && hRecord->getId())
                    loadSongs(hRecord);
                deleteRecord(child);
            }
            Gtk::TreePath path(records.getModel()->get_path(iter));
            aUndo.push(Undo(Undo::DELETE, INTERPRET, interpret->getId(), interpret, path, ""));
            records.getModel()->erase(iter);
        }
    }
}

//-----------------------------------------------------------------------------
/// Deletes the passed record
/// \param record Iterator to record to delete
//-----------------------------------------------------------------------------
void PRecords::deleteRecord(const Gtk::TreeModel::iterator& record) {
    HRecord hRec(records.getRecordAt(record));
    TRACE9("PRecords::deleteRecord (const Gtk::TreeModel::iterator&) - Deleting record " << hRec->getName());
    contract_assert(relRecords.isRelated(hRec));
    HInterpret hInterpret(relRecords.getParent(hRec));
    contract_assert(hInterpret);

    // Remove related songs
    TRACE3("PRecords::deleteRecord (const Gtk::TreeModel::iterator&) - Remove Songs");
    if (relSongs.isRelated(hRec)) {
        for (const auto& song : relSongs.getObjects(hRec))
            deleteSong(song, hRec);
        relSongs.unrelateAll(hRec);
    }

    contract_assert(!delRelation.contains(hRec));

    Gtk::TreePath path(records.getModel()->get_path(records.getOwner(hInterpret)));
    aUndo.push(Undo(Undo::DELETE, RECORD, hRec->getId(), hRec, path, ""));
    delRelation[hRec] = hInterpret;
    relRecords.unrelate(hInterpret, hRec);

    records.getModel()->erase(record);
}

//-----------------------------------------------------------------------------
/// Removes the passed songs from the model (but not from the relations).
/// \param song Song to delete
/// \param record Record of song
//-----------------------------------------------------------------------------
void PRecords::deleteSong(const HSong& song, const HRecord& record) {
    TRACE9("PRecords::deleteSong (const HSong& song, const HRecord& record)");
    contract_assert(!delRelation.contains(song));

    Gtk::TreePath path(records.getModel()->get_path(records.getObject(record)));
    aUndo.push(Undo(Undo::DELETE, SONG, song->getId(), song, path, ""));
    delRelation[song] = record;
}

//-----------------------------------------------------------------------------
/// Removes the selected songs from the listbox.
//-----------------------------------------------------------------------------
void PRecords::deleteSelectedSongs() {
    TRACE9("PRecords::deleteSelectedSongs ()");

    Glib::RefPtr<Gtk::TreeSelection> selection(songs.get_selection());
    while (!selection->get_selected_rows().empty()) {
        const std::vector<Gtk::TreePath> list(selection->get_selected_rows());
        contract_assert(!list.empty());

        Gtk::TreeModel::iterator iter(songs.get_model()->get_iter(list.front()));
        contract_assert(iter);
        HSong song(songs.getSongAt(iter));
        contract_assert(song);
        contract_assert(relSongs.isRelated(song));
        HRecord record(relSongs.getParent(song));
        contract_assert(record);
        deleteSong(song, record);

        relSongs.unrelate(record, song);
        songs.getModel()->erase(iter);
    }
}

//-----------------------------------------------------------------------------
/// Exports the contents of the page to HTML
/// \param fd File-descriptor for exporting
//-----------------------------------------------------------------------------
void PRecords::export2HTML(unsigned int fd, const std::string&) {
    std::ranges::sort(interprets, &Interpret::compByName);

    // Write record-information
    for (const auto& interpret : interprets)
        if (relRecords.isRelated(interpret)) {
            std::stringstream output;
            output << 'I' << *interpret;

            const std::vector<HRecord>& aRecords(relRecords.getObjects(interpret));
            contract_assert(!aRecords.empty());
            for (const auto& record : aRecords)
                output << 'R' << *record;

            TRACE9("PRecorsd::export2HTML (unsigned int) - Writing: " << output.str());
            if (::write(fd, output.str().data(), output.str().size()) != static_cast<ssize_t>(output.str().size())) {
                showError(Glib::ustring::compose(_("Couldn't write data!\n\nReason: %1"), Glib::ustring(strerror(errno))),
                          _("Error exporting records to HTML!"));
                break;
            }
        }
}

//-----------------------------------------------------------------------------
/// Adds a song to the list (creating record/interpret/song, if necessary)
/// \param artist Name of artist
/// \param record Name of record
/// \param song Name of song
/// \param track Number of track
/// \param genre Genre
/// \param year Year of record
//-----------------------------------------------------------------------------
void PRecords::addEntry(const Glib::ustring& artist, const Glib::ustring& record, const Glib::ustring& song, unsigned int track,
                        Glib::ustring& genre, [[maybe_unused]] unsigned int year) {
    HInterpret interpret;
    Gtk::TreeModel::iterator i(records.getOwner(artist));
    if (i == records.getModel()->children().end()) {
        TRACE9("PRecords::addEntry (3x const Glib::ustring&, unsigned int) - Adding band " << artist);

        interpret = std::make_shared<Interpret>();
        interpret->setName(artist);
        i = addInterpret(interpret);
    }
    else
        interpret = records.getInterpretAt(i);

    HRecord rec;
    Gtk::TreeModel::iterator r(records.getObject(i, record));
    if (r == i->children().end()) {
        TRACE9("PRecords::addEntry (3x const Glib::ustring&, unsigned int) - Adding record " << record);
        rec = std::make_shared<Record>();
        rec->setSongsLoaded();
        rec->setName(record);
        addRecord(i, rec);
    }
    else {
        rec = records.getRecordAt(r);
        records.selectRow(r);
    }

    const int idGenre(songs.getGenre(genre));
    HSong hSong;
    Gtk::TreeModel::iterator s(songs.getSong(song));
    if (s == songs.getModel()->children().end()) {
        TRACE9("PRecords::addEntry (3x const Glib::ustring&, unsigned int) - Adding song " << hSong);
        hSong = std::make_shared<Song>();
        hSong->setName(song);
        if (track)
            hSong->setTrack(track);
        if (idGenre > 0)
            hSong->setGenre(idGenre);
        addSong(hSong);
    }
    else {
        hSong = songs.getSongAt(s);
        songs.scroll_to_row(songs.getModel()->get_path(s), 0.80);
        Glib::RefPtr<Gtk::TreeSelection> songSel(songs.get_selection());
        songSel->select(s);
        Gtk::TreeRow row(*s);
        if (track) {
            hSong->setTrack(track);
            songs.updateTrack(row, hSong->getTrack());
        }
        if (idGenre > 0) {
            hSong->setGenre(idGenre);
            songs.updateGenre(row, genre);
        }
    }
}

//-----------------------------------------------------------------------------
/// Undoes the changes on the page
//-----------------------------------------------------------------------------
void PRecords::undo() {
    TRACE1("PRecords::undo ()");
    contract_assert(!aUndo.empty());

    Undo last(aUndo.top());
    switch (last.what()) {
    case SONG:
        undoSong(last);
        break;

    case RECORD:
        undoRecord(last);
        break;

    case INTERPRET:
        undoInterpret(last);
        break;

    default:
        contract_assert(false);
    } // end-switch

    aUndo.pop();
    if (aUndo.empty()) {
        enableSave(false);
        apMenus[UNDO]->set_enabled(false);
    }
}

//-----------------------------------------------------------------------------
/// Undoes the last changes to a record
/// \param last Undo-information
//-----------------------------------------------------------------------------
void PRecords::undoRecord(const Undo& last) {
    TRACE6("PRecords::undoRecord (const Undo&)");

    Gtk::TreePath path(last.getPath());
    Gtk::TreeModel::iterator iter(records.getModel()->get_iter(path));
    contract_assert(iter->parent());

    contract_assert(typeid(*last.getEntity()) == typeid(Record));
    HRecord record(std::dynamic_pointer_cast<Record>(last.getEntity()));
    TRACE9("PRecords::undoRecord (const Undo&) - " << last.how() << ": " << record->getName());

    switch (last.how()) {
    case Undo::CHANGED:
        switch (last.column()) {
        case 0:
            record->setName(last.getValue());
            break;

        case 1:
            record->setYear(last.getValue());
            break;

        case 2:
            record->setGenre(static_cast<unsigned int>(last.getValue()[0]));
            break;

        default:
            contract_assert(false);
        } // end-switch
        break;

    case Undo::INSERT:
        contract_assert(iter->parent());
        contract_assert(relRecords.isRelated(record));
        relRecords.unrelate(records.getInterpretAt(iter->parent()), record);
        records.getModel()->erase(iter);
        iter = records.getModel()->children().end();
        break;

    case Undo::DELETE: {
        auto delRel(delRelation.find(last.getEntity()));
        contract_assert(typeid(*delRel->second) == typeid(Interpret));
        HInterpret interpret(std::dynamic_pointer_cast<Interpret>(delRel->second));
        Gtk::TreeRow rowInterpret(*records.getOwner(interpret));

        iter = records.append(record, rowInterpret).get_iter();
        path = records.getModel()->get_path(iter);

        relRecords.relate(interpret, record);
        delRelation.erase(delRel);
        break;
    }

    default:
        contract_assert(false);
    } // end-switch

    if (iter) {
        Gtk::TreeRow row(*iter);
        records.update(row);
    }
    records.set_cursor(path);
    records.scroll_to_row(path, 0.8);
}

//-----------------------------------------------------------------------------
/// Undoes the last changes to an interpret
/// \param last Undo-information
//-----------------------------------------------------------------------------
void PRecords::undoInterpret(const Undo& last) {
    TRACE6("PRecords::undoInterpret (const Undo&)");

    Gtk::TreePath path(last.getPath());
    Gtk::TreeModel::iterator iter(records.getModel()->get_iter(path));

    HInterpret interpret(std::dynamic_pointer_cast<Interpret>(last.getEntity()));
    contract_assert(interpret);
    TRACE9("PRecords::undoInterpret (const Undo&) - " << last.how() << ": " << interpret->getName());

    switch (last.how()) {
    case Undo::CHANGED:
        contract_assert(iter);
        contract_assert(!iter->parent());

        switch (last.column()) {
        case 0:
            interpret->setName(last.getValue());
            break;

        case 1:
            interpret->setLifespan(last.getValue());
            break;

        default:
            contract_assert(false);
        } // end-switch
        break;

    case Undo::INSERT:
        contract_assert(iter);
        contract_assert(!iter->parent());
        contract_assert(!relRecords.isRelated(interpret));
        records.getModel()->erase(iter);
        iter = records.getModel()->children().end();
        break;

    case Undo::DELETE:
        if (iter)
            contract_assert(!iter->parent());
        else
            iter = records.getModel()->children().end();
        iter = records.insert(interpret, iter).get_iter();
        path = records.getModel()->get_path(iter);
        break;

    default:
        contract_assert(false);
    } // end-switch

    if (iter) {
        Gtk::TreeRow row(*iter);
        records.update(row);
    }
    records.set_cursor(path);
    records.scroll_to_row(path, 0.8);
}

//-----------------------------------------------------------------------------
/// Undoes the last changes to a song
/// \param last Undo-information
//-----------------------------------------------------------------------------
void PRecords::undoSong(const Undo& last) {
    TRACE6("PRecords::undoSong (const Undo&) - " << last.column());

    Gtk::TreePath path(last.getPath());
    Gtk::TreeModel::iterator iter(records.getModel()->get_iter(path));
    Glib::RefPtr<Gtk::TreeSelection> sel(records.get_selection());
    sel->unselect_all();
    sel->select(iter);

    contract_assert(typeid(*last.getEntity()) == typeid(Song));
    HSong song(std::dynamic_pointer_cast<Song>(last.getEntity()));
    TRACE9("PRecords::undoSong (const Undo&) - " << last.how() << ": " << song->getName());
    iter = songs.getSong(song);
    contract_assert(iter);

    switch (last.how()) {
    case Undo::CHANGED: {
        switch (last.column()) {
        case 0:
            song->setTrack(last.getValue());
            break;

        case 1:
            song->setName(last.getValue());
            break;

        case 2:
            song->setDuration(last.getValue());
            break;

        case 3:
            contract_assert(last.getValue().size() == 1);
            song->setGenre(static_cast<unsigned int>(last.getValue()[0]));
            break;

        default:
            contract_assert(false);
        } // end-switch
        break;
    }

    case Undo::INSERT:
        relSongs.unrelate(relSongs.getParent(song), song);
        songs.getModel()->erase(iter);
        iter = songs.getModel()->children().end();
        break;

    case Undo::DELETE: {
        iter = songs.insert(song, iter);

        auto delRel(delRelation.find(last.getEntity()));
        contract_assert(typeid(*delRel->second) == typeid(Record));
        relSongs.relate(std::dynamic_pointer_cast<Record>(delRel->second), song);

        delRelation.erase(delRel);
        break;
    }

    default:
        contract_assert(false);
    } // end-switch

    if (iter) {
        Gtk::TreeRow row(*iter);
        songs.update(row);
        path = songs.getModel()->get_path(iter);
    }
    songs.set_cursor(path);
    songs.scroll_to_row(path, 0.8);
}

//-----------------------------------------------------------------------------
/// Sets the focus to the record-list
//-----------------------------------------------------------------------------
void PRecords::getFocus() { records.grab_focus(); }

//-----------------------------------------------------------------------------
/// Removes all information from the page
//-----------------------------------------------------------------------------
void PRecords::clear() {
    TRACE9("PRecords::clear ()");
    relSongs.unrelateAll();
    relRecords.unrelateAll();
    interprets.clear();

    songs.getModel()->clear();
    records.getModel()->clear();
    NBPage::clear();
}
