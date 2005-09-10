//$Id: CDManagerEI.cpp,v 1.2 2005/09/10 21:36:55 markus Exp $

//PROJECT     : CDManager
//SUBSYSTEM   : CDManagerEI
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.2 $
//AUTHOR      : Markus Schwab
//CREATED     : 30.8.2005
//COPYRIGHT   : Copyright (C) 2005

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#include <cdmgr-cfg.h>

#include <cerrno>
#include <cstdlib>

#include <fstream>
#include <sstream>

#include <YGP/File.h>
#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/Tokenize.h>
#include <YGP/Process.h>

#include <gtkmm/messagedialog.h>

#include "Words.h"

#include "CDManager.h"


//-----------------------------------------------------------------------------
/// Exports the stored information to HTML documents
//-----------------------------------------------------------------------------
void CDManager::exportToHTML () {
   std::string dir (opt.getDirOutput ());
   if (dir.size () && (dir[dir.size () - 1] != YGP::File::DIRSEPARATOR)) {
      dir += YGP::File::DIRSEPARATOR;
      opt.setDirOutput (dir);
   }

   try {
      exportData ();
   }
   catch (Glib::ustring& e) {
      Gtk::MessageDialog dlg (e, Gtk::MESSAGE_ERROR);
      dlg.run ();
   }
}

//-----------------------------------------------------------------------------
/// Loads the data and exports the stored information to HTML documents
//-----------------------------------------------------------------------------
void CDManager::exportData () throw (Glib::ustring) {
   TRACE9 ("CDManager::exportData ()");
   if (!(loadedPages & 1))
      loadRecords ();

   if (!(loadedPages & 2))
      loadMovies ();

   std::sort (directors.begin (), directors.end (), &Director::compByName);
   std::sort (artists.begin (), artists.end (), &Interpret::compByName);

   YGP::Tokenize langs (LANGUAGES);
   std::string lang;
   std::string tmpLang (Movie::currLang);
   const char* envLang (getenv ("LANGUAGE"));
   std::string oldLang;
   if (envLang)
      oldLang = envLang;

   while ((lang = langs.getNextNode (' ')).size ()) {
      TRACE1 ("CDManager::exportData () - Lang: " << lang);
      Glib::ustring stat (_("Exporting (language %1) ..."));
      stat.replace (stat.find ("%1"), 2, Language::findInternational (lang));
      status.push (stat);

      Glib::RefPtr<Glib::MainContext> ctx (Glib::MainContext::get_default ());
      while (ctx->iteration (false));                      // Update statusbar

      if (!loadedLangs[lang])
	 loadMovies (lang);

      setenv ("LANGUAGE", lang.c_str (), true);

      std::ostringstream memKey;
      memKey << Words::getMemoryKey () << std::ends;

      int pipes[2];
      const char* args[] = { "CDWriter", "--outputDir", opt.getDirOutput ().c_str (),
			     "--recHeader", opt.getRHeader ().c_str (),
			     "--recFooter", opt.getRFooter ().c_str (),
			     "--movieHeader", opt.getMHeader ().c_str (),
			     "--movieFooter", opt.getMFooter ().c_str (),
			     lang.c_str (), memKey.str ().c_str (), NULL };
      pid_t pid (-1);
      try {
	 pipe (pipes);
	 pid = YGP::Process::execIOConnected ("CDWriter", args, pipes);
      }
      catch (std::string& e) {
	 Gtk::MessageDialog dlg (Glib::locale_to_utf8 (e), Gtk::MESSAGE_ERROR);
	 dlg.set_title (_("Export Error!"));
	 dlg.run ();
	 continue;
      }

      // Write movie-information
      Movie::currLang = lang;
      for (std::vector<HDirector>::const_iterator i (directors.begin ());
	   i != directors.end (); ++i)
	 if (relMovies.isRelated (*i)) {
	    std::stringstream output;
	    output << 'D' << **i;

	    std::vector<HMovie>& dirMovies (relMovies.getObjects (*i));
	    Check3 (dirMovies.size ());
	    for (std::vector<HMovie>::const_iterator m (dirMovies.begin ());
		 m != dirMovies.end (); ++m)
	       output << "M" << **m;

	    TRACE9 ("CDManager::export () - Writing: " << output.str ());
	    ::write (pipes[1], output.str ().data (), output.str ().size ());
	 }
      Movie::currLang = tmpLang;

      // Write record-information
      for (std::vector<HInterpret>::const_iterator i (artists.begin ());
	   i != artists.end (); ++i)
	 if (relRecords.isRelated (*i)) {
	    std::stringstream output;
	    output << 'I' << **i;

	    std::vector<HRecord>& records (relRecords.getObjects (*i));
	    Check3 (records.size ());
	    for (std::vector<HRecord>::const_iterator r (records.begin ());
		 r != records.end (); ++r)
	       output << "R" << **r;

	    TRACE9 ("CDManager::export () - Writing: " << output.str ());
	    ::write (pipes[1], output.str ().data (), output.str ().size ());
	 }
      close (pipes[1]);

      char output[128] = "";
      std::string allOut;
      int cRead;
      while ((cRead = ::read (pipes[0], output, sizeof (output))) != -1) {
	 allOut.append (output, cRead);
	 if (!cRead)
	    break;
      }
      if (allOut.size ()) {
	 Gtk::MessageDialog dlg (Glib::locale_to_utf8 (output), Gtk::MESSAGE_INFO);
	 dlg.set_title (_("Export Warning!"));
	 dlg.run ();
      }

      close (pipes[0]);
      status.pop ();

      Check3 (pid != -1);
      YGP::Process::waitForProcess (pid);
   }
   setenv ("LANGUAGE", oldLang.c_str (), true);
}

//-----------------------------------------------------------------------------
/// Reads the ID3 information from a MP3 file
/// \param file: Name of file to analzye
//-----------------------------------------------------------------------------
void CDManager::parseFileInfo (const std::string& file) {
   TRACE9 ("CDManager::parseFileInfo (const std::string&) - " << file);
   Check2 (file.size ());

   std::ifstream stream (file.c_str ());
   Glib::ustring artist, record, song;
   unsigned int track (0);
   if (!stream) {
      Glib::ustring err (_("Can't open file `%1'!\n\nReason: %2"));
      err.replace (err.find ("%1"), 2, file);
      err.replace (err.find ("%2"), 2, strerror (errno));
      Gtk::MessageDialog (err).run ();
      return;
   }

   TRACE1 ("CDManager::parseFileInfo (const std::string&) - Type: " << file.substr (file.size () - 4));
   if (((file.substr (file.size () - 4) == ".mp3")
	&& parseMP3Info (stream, artist, record, song, track))
       || ((file.substr (file.size () - 4) == ".ogg")
	   && parseOGGCommentHeader (stream, artist, record, song, track))) {
      TRACE9 ("CDManager::parseFileInfo (const std::string&) - " << artist
	      << '/' << record << '/' << song << '/' << track);

      HInterpret group;
      Gtk::TreeIter i (records.getOwner (artist));
      if (i == records.getModel ()->children ().end ()) {
	 TRACE9 ("CDManager::parseFileInfo (const std::string&) - Adding band " << artist);
	 group.define ();
	 group->setName (artist);
	 i = addArtist (group);
      }
      else
	 group = records.getInterpretAt (i);

      HRecord rec;
      Gtk::TreeIter r (records.getObject (i, record));
      if (r == i->children ().end ()) {
	 TRACE9 ("CDManager::parseFileInfo (const std::string&) - Adding rec " << record);
	 rec.define ();
	 rec->setSongsLoaded ();
	 rec->setName (record);
	 addRecord (i, rec);
      }
      else {
	 rec = records.getRecordAt (r);
	 records.selectRow (r);
      }

      HSong hSong;
      Gtk::TreeIter s (songs.getSong (song));
      if (s == songs.getModel ()->children ().end ()) {
	 TRACE9 ("CDManager::parseFileInfo (const std::string&) - Adding song " << song);
	 hSong.define ();
	 hSong->setName (song);
	 if (track)
	    hSong->setTrack (track);
	 addSong (hSong);
      }
      else {
	 hSong = songs.getEntryAt (s);
	 songs.scroll_to_row (songs.getModel ()->get_path (s), 0.80);
	 Glib::RefPtr<Gtk::TreeSelection> songSel (songs.get_selection ());
	 songSel->select (s);
	 if (track) {
	    hSong->setTrack (track);
	    Gtk::TreeRow row (*s);
	    songs.updateTrack (row, hSong->getTrack ());
	 }
      }
   }
}

//-----------------------------------------------------------------------------
/// Reads the ID3 information from a MP3 file
/// \param stream: MP3-file to analyze
/// \param artist: Found artist
/// \param record: Found record name
/// \param song: Found song
/// \param track: Tracknumber
/// \returns bool: True, if ID3 info has been found
//-----------------------------------------------------------------------------
bool CDManager::parseMP3Info (std::istream& stream, Glib::ustring& artist,
			      Glib::ustring& record, Glib::ustring& song,
			      unsigned int& track) {
   stream.seekg (-0x80, std::ios::end);
   std::string value;

   std::getline (stream, value, '\xff');
   TRACE9 ("CDManager::parseMP3Info (std::istream&, 3x Glib::ustring&, unsigned&) - Found: "
	   << value << "; Length: " << value.size ());
   if ((value.size () > 3) && (value[0] == 'T') && (value[1] == 'A') && (value[2] == 'G')) {
      song = Glib::locale_to_utf8 (stripString (value, 3, 29));
      artist = Glib::locale_to_utf8 (stripString (value, 33, 29));
      record = Glib::locale_to_utf8 (stripString (value, 63, 29));
      track = (value[0x7d] != 0x20) ? value[0x7e] : 0;
      return true;
   }
   return false;
}

//-----------------------------------------------------------------------------
/// Reads the OGG comment header from an OGG vorbis encoded file
/// \param stream: OGG-file to analyze
/// \param artist: Found artist
/// \param record: Found record name
/// \param song: Found song
/// \param track: Tracknumber
/// \returns bool: True, if comment header has been found
//-----------------------------------------------------------------------------
bool CDManager::parseOGGCommentHeader (std::istream& stream, Glib::ustring& artist,
				       Glib::ustring& record, Glib::ustring& song,
				       unsigned int& track) {
   char buffer[512];
   stream.read (buffer, 4);
   if ((*buffer != 'O') && (buffer[1] != 'g') && (buffer[2] != 'g') && (buffer[3] != 'S'))
      return false;

   stream.seekg (0x69, std::ios::cur);
   unsigned int len (0);
   stream.read ((char*)&len, 4);                // Read the vendorstring-length
   TRACE9 ("CDManager::parseOGGCommentHeader (std::istream&, 3x Glib::ustring&, unsigned&) - Length: " << len);
   stream.seekg (len, std::ios::cur);

   unsigned int cComments (0);
   stream.read ((char*)&cComments, 4);               // Read number of comments
   TRACE9 ("CDManager::parseOGGCommentHeader (std::istream&, 3x Glib::ustring&, unsigned&) - Comments: " << cComments);
   if (!cComments)
      return false;

   std::string key;
   Glib::ustring *value (NULL);
   do {
      stream.read ((char*)&len, 4);                  // Read the comment-length

      std::getline (stream, key, '=');
      len -= key.size () + 1;
      TRACE9 ("CDManager::parseOGGCommentHeader (std::stream&, 3x Glib::ustring&, unsigned&) - Key: " << key);

      if (key == "TITLE")
	 value = &song;
      else if (key == "ALBUM")
	 value = &record;
      else if (key == "ARTIST")
	 value = &artist;
      else if (key == "TRACKNUMBER") {
	 Check2 (len < sizeof (buffer));
	 stream.read (buffer, len);
	 track = strtoul (buffer, NULL, 10);
	 value = NULL;
	 len = 0;
      }
      else
	 value = NULL;

      if (value) {
	 unsigned int read (0);
	 do {
	    read = stream.readsome (buffer, (len > sizeof (buffer)) ? sizeof (buffer) - 1 : len);
	    len -= read;
	    buffer[read] = '\0';
	    value->append (buffer);
 	 } while (len);
      }
      else
	 stream.seekg (len, std::ios::cur);
   } while (--cComments);  // end-do while comments
   return true;
}

//-----------------------------------------------------------------------------
/// Returns the specified substring, removed from trailing spaces
/// \param value: String to manipulate
/// \param pos: Starting pos inside the string
/// \param len: Maximal length of string
/// \returns std::string: Stripped value
//-----------------------------------------------------------------------------
std::string CDManager::stripString (const std::string& value, unsigned int pos, unsigned int len) {
   len += pos;
   while (len > pos) {
      if ((value[len] != ' ') && (value[len]))
         break;
      --len;
   }
   return (pos == len) ? " " : value.substr (pos, len - pos + 1);
}

