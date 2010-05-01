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
   IMDbProgress ();
   IMDbProgress (const Glib::ustring& movie);
   virtual ~IMDbProgress ();

   void start (const Glib::ustring& idMovie);
   void stop ();
   void disconnect ();

   typedef struct ConnectInfo ConnectInfo;

   sigc::signal<void, const Glib::ustring&> sigError;
   sigc::signal<void, const std::map<Glib::ustring, Glib::ustring>&> sigAmbiguous;
   sigc::signal<void, const Glib::ustring&, const Glib::ustring&, const Glib::ustring&> sigSuccess;

 protected:
   sigc::connection conPoll;
   sigc::connection conProgress;

 private:
   IMDbProgress (const IMDbProgress& other);
   const IMDbProgress& operator= (const IMDbProgress& other);

   static bool isNumber (const Glib::ustring& nr);

   bool reStart (const Glib::ustring& idMovie);

   bool poll ();
   bool indicateWait ();
   void error (const Glib::ustring& msg);

   static void extractSearch (std::map<Glib::ustring, Glib::ustring>& target,
			      const Glib::ustring& src, const Glib::ustring& text);
   Glib::ustring extract (const char* section, const char* subpart,
			  const char* before, const char* after) const;
   static void convert (Glib::ustring& string);

   void connectToIMDb ();
   void resolved (const boost::system::error_code& err,
		  boost::asio::ip::tcp::resolver::iterator iEndpoints);
   void connected (const boost::system::error_code& err,
		   boost::asio::ip::tcp::resolver::iterator iEndpoints);
   void sendRequest (const Glib::ustring& movie);
   void requestWritten (const boost::system::error_code& err);
   void readStatus (const boost::system::error_code& err);
   void readHeaders (const boost::system::error_code& err);
   void readContent (const boost::system::error_code& err);

   ConnectInfo* data;
};

#endif
