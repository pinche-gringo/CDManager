#ifndef LANGDLG_H
#define LANGDLG_H

//$Id: LangDlg.h,v 1.1 2004/12/11 16:52:41 markus Exp $

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


#include <string>

#include <glibmm/ustring.h>

#include <gtkmm/liststore.h>
#include <gtkmm/treemodelcolumn.h>

#include <XGP/XDialog.h>

namespace Gtk {
   class Box;
   class Image;
   class TreeView;
}


/**Dialog allowing to select various languages
 */
class LanguageDialog : public XGP::XDialog {
 public:
   LanguageDialog (std::string& languages, unsigned int maxLangs);
   virtual ~LanguageDialog ();

   /// Method to display the dialog; cares about freeing it afterwards.
   /// This is the method of choice, when the dialog is created on the heap;
   /// don't just call new LanguageDialog (...);
   /// \param languages: The preselected languages; updated with the user input
   /// \param maxLangs: Maximal number of languages which can be selected
   static LanguageDialog* create (std::string& languages, unsigned int maxLangs) {
      LanguageDialog* dlg (new LanguageDialog (languages, maxLangs));
      dlg->signal_response ().connect (mem_fun (*dlg, &LanguageDialog::free));
      return dlg;
   }

 private:
   //Prohibited manager functions
   LanguageDialog (const LanguageDialog& other);
   const LanguageDialog& operator= (const LanguageDialog& other);

   virtual void okEvent ();

   Gtk::Box*    pClient;
   std::string& languages;
   unsigned int maxLangs;

   Gtk::Image*    imgLang;
   Gtk::TreeView* listLang;

   class LanguageColumns : public Gtk::TreeModel::ColumnRecord {
    public:
      LanguageColumns () {
	 add (flag); add (name); add (id); }

      Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > flag;
      Gtk::TreeModelColumn<Glib::ustring>              name;
      Gtk::TreeModelColumn<std::string>                id;
   };
   LanguageColumns colLang;
   Glib::RefPtr<Gtk::ListStore> model;
};

#endif
