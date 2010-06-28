#ifndef IMDBPROGRESS_H
#define IMDBPROGRESS_H

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
#include <list>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/placeholders.hpp>

#include <glibmm/ustring.h>

#include <gtkmm/progressbar.h>


/**Class reading data from IMDb while showing its status in itself (a
 * progress bar)
 *
 * Available signals:
 *   - sigError: Emitted if an error occurs; the error-message is passed
 *   - sigAmbigous: Emitted if a search finds more than one movie;
                    Passes a list of matching entries
 *   - sigSuccess: Emitted if a (single) movie was found; Passes the
                   director, the movie (with year in parenthesises) and the genre
 * \remarks Don't destroy the object within the signal-callbacks
 */
class IMDbProgress : public Gtk::ProgressBar {
 public:
   typedef enum { POPULAR, EXACT, PARTIAL } match;
   typedef struct _IMDbSearchEntry {
      std::string url;
      Glib::ustring title;

      _IMDbSearchEntry (const std::string& url, const Glib::ustring& title)
         : url (url), title (title) { }
   } IMDbSearchEntry;
   typedef std::list<IMDbSearchEntry> IMDbSearchEntries;

   typedef struct _IMDbEntry {
      Glib::ustring director;
      Glib::ustring title;
      Glib::ustring genre;
      Glib::ustring summary;
      std::string image;

      _IMDbEntry (const Glib::ustring& director, const Glib::ustring& title, const Glib::ustring& genre,
		  const Glib::ustring& summary, const std::string& icon)
         : director (director), title (title), genre (genre), summary (summary), image (icon) { }
   } IMDbEntry;

   IMDbProgress ();
   IMDbProgress (const Glib::ustring& movie);
   virtual ~IMDbProgress ();

   void start (const Glib::ustring& idMovie);
   void loadIcon (const std::string& icon);
   void stop ();
   void disconnect ();

   typedef struct ConnectInfo ConnectInfo;

   sigc::signal<void, const Glib::ustring&> sigError;
   sigc::signal<void, const std::map<match, IMDbSearchEntries>&> sigAmbiguous;
   sigc::signal<void, const IMDbEntry&> sigSuccess;
   sigc::signal<void, const std::string&> sigIcon;

 protected:
   sigc::connection conPoll;
   sigc::connection conProgress;

 private:
   IMDbProgress (const IMDbProgress& other);
   const IMDbProgress& operator= (const IMDbProgress& other);

   bool reStart (const Glib::ustring& idMovie);

   bool poll ();
   bool indicateWait ();
   void error (const Glib::ustring& msg);

   static void extractSearch (IMDbSearchEntries& target, const Glib::ustring& src,
			      const char** texts, unsigned int offset);
   Glib::ustring extract (const char* section, const char* subpart,
			  const char* before, const char* after) const;
   static void convert (Glib::ustring& string);

   void connect ();
   void resolved (const boost::system::error_code& err,
		  boost::asio::ip::tcp::resolver::iterator iEndpoints);
   void connected (const boost::system::error_code& err,
		   boost::asio::ip::tcp::resolver::iterator iEndpoints);
   void sendRequest ();
   void requestWritten (const boost::system::error_code& err);
   void readStatus (const boost::system::error_code& err);
   void readHeaders (const boost::system::error_code& err);
   void readContent (const boost::system::error_code& err);
   void readMovie (Glib::ustring& msg);
   void readImage ();

   ConnectInfo* data;

   enum { NONE, TITLE, IMAGE } status;
};

#endif
