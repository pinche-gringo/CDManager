//$Id: LangDlg.cpp,v 1.1 2004/12/11 16:52:41 markus Exp $

//PROJECT     : CDManager
//SUBSYSTEM   : Language
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.1 $
//AUTHOR      : Markus Schwab
//CREATED     : 11.12.2004
//COPYRIGHT   : Copyright (C) 2004

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

#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/image.h>
#include <gtkmm/button.h>
#include <gtkmm/treeview.h>

#define CHECK 9
#define TRACELEVEL 9
#include <YGP/Check.h>
#include <YGP/Trace.h>

#include "Language.h"

#include "LangDlg.h"


//-----------------------------------------------------------------------------
/// Constructor
/// \param languages: The preselected languages; updated with the user input
/// \param maxLangs: Maximal number of languages which can be selected
//-----------------------------------------------------------------------------
LanguageDialog::LanguageDialog (std::string& languages, unsigned int maxLangs)
   : XGP::XDialog (OKCANCEL), pClient (new Gtk::VBox), languages (languages),
      maxLangs ( maxLangs), imgLang (new Gtk::Image),
     listLang (new Gtk::TreeView), model (Gtk::ListStore::create (colLang)) {
   set_title (_("Select languages"));

   Gtk::Label*  lblLang (new Gtk::Label (_("_Language: "), true));
   Gtk::Box*    boxLang (new Gtk::HBox);
   Gtk::Button* mainLang (new Gtk::Button);
   boxLang->pack_start (*manage (lblLang));
   boxLang->pack_start (*manage (imgLang));
   mainLang->add (*manage (boxLang));

   lblLang->show ();
   imgLang->show ();
   boxLang->show ();
   mainLang->show ();

   Gtk::TreeViewColumn* column (new Gtk::TreeViewColumn (_("Translations")));
   column->pack_start (colLang.flag, false);
   column->pack_start (colLang.name);
   listLang->append_column (*Gtk::manage (column));
   listLang->set_model (model);
   listLang->get_selection ()->set_mode (Gtk::SELECTION_EXTENDED);

   YGP::Tokenize langs (languages);
   std::string main;
   if (languages.size ()) {
      main = languages.getNextNode (',');
      imgLang = Languages::getFlag (main);
   }

   // Add language values
   if (model->children ().empty ())
      for (std::map<std::string, Language>::const_iterator l (Language::begin ());
	   l != Language::end (); ++l) {
	 Gtk::TreeModel::Row lang (*model->append ());
	 lang[colLang.id] = l->first;
	 lang[colLang.flag] = l->second.getFlag ();
	 lang[colLang.name] = l->second.getInternational ();
      }

   pClient->pack_start (*manage (mainLang), Gtk::PACK_EXPAND_PADDING, 5);
   pClient->pack_start (*manage (listLang), Gtk::PACK_EXPAND_WIDGET, 5);
   pClient->show ();

   get_vbox ()->pack_start (*manage (pClient), false, false, 5);
   show_all_children ();
   show ();
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
LanguageDialog::~LanguageDialog () {
}

//-----------------------------------------------------------------------------
/// Handling of the OK button; closes the dialog with commiting data
//-----------------------------------------------------------------------------
void LanguageDialog::okEvent () {
   TRACE9 ("LanguageDialog::okEvent ()");

#if 0
   Glib::RefPtr<Gtk::TreeSelection> selection (get_selection ());
   Gtk::TreeIter movieSel (selection->get_selected ()); Check3 (movieSel->parent ());

   Gtk::TreeRow selFlag (*(*list)->get_selection ()->get_selected ());

   std::string langs (getMovieAt (movieSel)->getLanguage ());
   std::string newLang (selFlag[colLang.id]);
   if (newLang.size ()) {
      if (langs.size ())
	 langs.append (1, ',');
      langs += newLang;
   } // end-if
   else {
      unsigned int pos (langs.rfind (','));
      if (pos == std::string::npos)
	 pos = 0;
      TRACE1 ("Removing from " << pos << " of " << langs);
      langs.replace (pos, langs.size () - pos, 0, '\0');
   }
   valueChanged (mOwnerObjects->get_path (movieSel).to_string (), langs, 1);
#endif
}
