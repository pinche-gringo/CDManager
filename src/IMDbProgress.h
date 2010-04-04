#ifndef IMDBPROGRESS_H
#define IMDBPROGRESS_H

//$Id$

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


#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/placeholders.hpp>

#include <glibmm/ustring.h>

#include <gtkmm/progressbar.h>


/**Class reading data from IMDb while showing its status in itself (a
 * progress bar)
 */
class IMDbProgress : public Gtk::ProgressBar {
 public:
   IMDbProgress (const Glib::ustring& movie);
   virtual ~IMDbProgress ();

   typedef struct ConnectInfo ConnectInfo;

   sigc::signal<void, const Glib::ustring&> sigError;
   sigc::signal<void, const Glib::ustring&, const Glib::ustring&, const Glib::ustring&> sigSuccess;


 private:
   IMDbProgress (const IMDbProgress& other);
   const IMDbProgress& operator= (const IMDbProgress& other);

   bool poll ();
   bool indicateWait ();
   void error (const Glib::ustring& msg);

   Glib::ustring extract (const char* section, const char* subpart,
			  const char* before, const char* after) const;

   void connectToIMDb ();
   void resolved (const boost::system::error_code& err,
		  boost::asio::ip::tcp::resolver::iterator iEndpoints);
   void connected (const boost::system::error_code& err,
		   boost::asio::ip::tcp::resolver::iterator iEndpoints);
   void requestWritten (const boost::system::error_code& err);
   void readStatus (const boost::system::error_code& err);
   void readHeaders (const boost::system::error_code& err);
   void readContent (const boost::system::error_code& err);

   ConnectInfo& data;
};

#endif
