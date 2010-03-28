#ifndef IMPORTIMDB_H
#define IMPORTIMDB_H

//$Id$

// This file is part of CDManager.
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
#include <boost/asio/streambuf.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/placeholders.hpp>

#include <gtkmm/table.h>

#include <YGP/Check.h>
#include <XGP/XDialog.h>


namespace Gtk {
   class Label;
   class Table;
   class ProgressBar;
}

/**Dialog allowing to import information from a movie from IMDb.com
 *
 * After entering an movie identification (a number or the URL to
 * IMDb.com) the matching page on IMDb.com is read and the relevant
 * information is filtered out and displayed for confirmation.
 */
class ImportFromIMDb : public XGP::XDialog {
 public:
   ImportFromIMDb ();
   virtual ~ImportFromIMDb ();

   /// Creates the dialog (and set it as child of the parent)
   /// \remarks Cares also about freeing the dialog
   static ImportFromIMDb* create () {
      ImportFromIMDb* dlg (new ImportFromIMDb);
      dlg->signal_response ().connect (mem_fun (*dlg, &ImportFromIMDb::free));
      return dlg;
   }

   /// Signal emitted, when the loaed movie-information is confirmed
   sigc::signal<bool, const Glib::ustring&, const Glib::ustring&, const Glib::ustring&> sigLoaded;

 protected:
   Gtk::Table* client;            ///< Pointer to the client information area
   Gtk::Entry* txtID;                ///< Textfield, where user enters the ID
   Gtk::Label* lblDirector;                ///< Label displaying the director
   Gtk::Label* lblMovie;          ///< Label displaying the movie (with year)
   Gtk::Label* lblGenre;         ///< Label displaying the genre of the movie

   Glib::ustring contentIMDb;                ///< Page received from IMDb.com

   void okEvent ();

 private:
   volatile enum { QUERY, LOADING, CONFIRM } status;

   // Prohibited manager functions
   ImportFromIMDb (const ImportFromIMDb& other);
   const ImportFromIMDb& operator= (const ImportFromIMDb& other);

   void inputChanged ();
   bool indicateWait (Gtk::ProgressBar* progress);
   void showError (const Glib::ustring& msg);

   Glib::ustring extract (const char* section, const char* subpart,
			  const char* before, const char* after) const;

   void connectToIMDb ();
   void resolved (const boost::system::error_code& err,
		  boost::asio::ip::tcp::resolver::iterator iEndpoints);
   void connected (const boost::system::error_code& err,
		  boost::asio::ip::tcp::resolver::iterator iEndpoints);
   void written (const boost::system::error_code& err);
   void readStatus (const boost::system::error_code& err);
   void readHeaders (const boost::system::error_code& err);
   void readContent (const boost::system::error_code& err);

   boost::asio::io_service svcIO;
   boost::asio::ip::tcp::socket sockIO;
   boost::asio::streambuf buf;
};

#endif
