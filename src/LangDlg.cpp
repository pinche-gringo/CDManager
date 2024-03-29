//PROJECT     : CDManager
//SUBSYSTEM   : Language
//REFERENCES  :
//TODO        :
//BUGS        :
//AUTHOR      : Markus Schwab
//CREATED     : 11.12.2004
//COPYRIGHT   : Copyright (C) 2004, 2005, 2009 - 2011

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

#include <map>

#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/button.h>
#include <gtkmm/combobox.h>
#include <gtkmm/treeview.h>

#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <boost/tokenizer.hpp>

#include "Language.h"

#include "LangDlg.h"


/**Model for language lists
 */
class LanguageModel : public Gtk::ListStore {
 public:
   static Glib::RefPtr<LanguageModel> create (const LanguageColumns& columns,
					      bool withUndefined = false) {
      return Glib::RefPtr<LanguageModel> (new LanguageModel (columns, withUndefined));
   }
   ~LanguageModel () { }

   /// Returns the line of the passed language
   Gtk::TreeIter getLine (const std::string& lang) {
      return children ()[lines[lang]]; }

   // Deletes the line with the passed language
   void eraseLanguage (const std::string& lang) {
      TRACE9 ("LanguageModel::eraseLanguage (const std::string&) - " << lang
	      << '=' << lines[lang]);
      erase (children ()[lines[lang]]);
   }
   void insertLanguage (const std::string& lang, const LanguageColumns& cols) {
      insertLanguage (lang, Language::findLanguage (lang), cols); }

protected:
   LanguageModel (const LanguageColumns& cols, bool withUndefined = false);

   void insertLanguage (const std::string& name, const Language& lang,
			const LanguageColumns& cols);

 private:
   LanguageModel ();
   LanguageModel (const LanguageModel&);
   LanguageModel& operator= (const LanguageModel&);

   std::map<std::string, unsigned int> lines;
};


//-----------------------------------------------------------------------------
/// Constructor
/// \param cols: Columns of the model
/// \param withUndefined: Flag, if undefined line should be displayed
//-----------------------------------------------------------------------------
LanguageModel::LanguageModel (const LanguageColumns& cols, bool withUndefined)
   : Gtk::ListStore (cols) {
   // Add language values
   if (withUndefined) {
      Gtk::TreeModel::Row lang (*append ());
      lang[cols.id] = "";
      lang[cols.name] = _("None");
   }

   for (std::map<std::string, Language>::const_iterator l (Language::begin ());
	l != Language::end (); ++l)
      insertLanguage (l->first, l->second, cols);
}

//-----------------------------------------------------------------------------
/// Inserts a line into the model
/// \param id: ID of language
/// \param lang: Language description
/// \param cols: Columns of the model
//-----------------------------------------------------------------------------
void LanguageModel::insertLanguage (const std::string& id, const Language& lang,
				    const LanguageColumns& cols) {
   TRACE9 ("LanguageModel::insertLanguage (const std::string&, const Language&, const Columns&) - "
	   << id << '=' << ((lines.find (id) == lines.end ()) ? children ().size () : lines[id]));
   Gtk::TreeModel::Row row;
   if (lines.find (id) == lines.end ()) {
      lines[id] = children ().size ();
      row = *append ();
   }
   else
      row = *insert (children ()[lines[id]]);

   row[cols.id] = id;
   row[cols.flag] = lang.getFlag ();
   row[cols.name] = lang.getInternational ();
}


//-----------------------------------------------------------------------------
/// Constructor
/// \param languages: The preselected languages; updated with the user input
/// \param maxLangs: Maximal number of languages which can be selected
/// \param showMainLang: Flag, if there is one main language
//-----------------------------------------------------------------------------
LanguageDialog::LanguageDialog (std::string& languages, unsigned int maxLangs,
				bool showMainLang)
   : XGP::XDialog (OKCANCEL), pClient (new Gtk::VBox), languages (languages),
      maxLangs (maxLangs),
     mainLang (new Gtk::ComboBox),
     listLang (new Gtk::TreeView),
     modelMain (LanguageModel::create (colLang, true)),
     modelList (LanguageModel::create (colLang)) {
   TRACE9 ("LanguageDialog::LanguageDialog (std::string&, unsigned int) - Main: "
	   << languages << '(' << maxLangs << ')');

   set_title (_("Select languages"));

   if (showMainLang) {
      // Label of the main language
      Gtk::Label*  lblLang (new Gtk::Label (_("_Language: "), true));

      // Combobox to select the main language
      mainLang->pack_start (colLang.flag, false);
      mainLang->pack_start (colLang.name);
      mainLang->set_model (modelMain);

      mainLang->signal_changed ().connect (mem_fun (*this, &LanguageDialog::selectLanguage));

      pClient->pack_start (*manage (lblLang), Gtk::PACK_EXPAND_PADDING, 5);
      pClient->pack_start (*manage (mainLang), Gtk::PACK_EXPAND_PADDING, 5);
   }

   // Listbox to select further languages
   if (maxLangs > 1)
      listLang->get_selection ()->set_mode (Gtk::SELECTION_MULTIPLE);
   Gtk::TreeViewColumn* column (new Gtk::TreeViewColumn (_("Translations")));
   column->pack_start (colLang.flag, false);
   column->pack_start (colLang.name);
   listLang->append_column (*Gtk::manage (column));
   listLang->set_model (modelList);

   std::string tmp;
   if (languages.size ()) {
      boost::tokenizer<boost::char_separator<char> > langs (languages, boost::char_separator<char> (","));
      boost::tokenizer<boost::char_separator<char> >::iterator i (langs.begin ());
      if (showMainLang) {
	 tmp = *i;
	 ++i;
	 TRACE9 ("LanguageDialog::LanguageDialog (std::string&, unsigned int) - Main: " << main);
      }

      std::string translation;
      Glib::RefPtr<Gtk::TreeSelection> sel (listLang->get_selection ());
      for (; i != langs.end (); ++i)
	 sel->select (modelList->getLine (*i));
   }
   else
      if (showMainLang)
	 listLang->set_sensitive (false);

   if (showMainLang) {
      mainLang->set_active (modelMain->getLine (tmp));
      main = tmp;
   }

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

   std::string translations;
   std::vector<Gtk::TreePath> list (listLang->get_selection ()->get_selected_rows ());
   if (list.size ()) {
      std::vector<Gtk::TreePath>::const_iterator i (list.begin ());
      translations = (*modelList->get_iter (*i++))[colLang.id];
      for (unsigned int c (0); (i != list.end ()) && (++c < maxLangs); ++i) {
	 translations.append (1, ',');
	 translations += (*modelList->get_iter (*i))[colLang.id];
      }

      if (main.size ())
	 main += ',';
      main += translations;
   }

   TRACE3 ("LanguageDialog::okEvent () - " << main);
   languages = main;
}

//-----------------------------------------------------------------------------
/// Sets the main language (and removes it as translation)
/// \param lang: Language to set
//-----------------------------------------------------------------------------
void LanguageDialog::selectLanguage () {
   TRACE9 ("LanguageDialog::selectLanguage () - " << main << "->"
	   << (std::string)(*mainLang->get_active ())[colLang.id]);

   if (main.size ())
      modelList->insertLanguage (main, colLang);
   main = (*mainLang->get_active ())[colLang.id];

   if (main.size ())
      modelList->eraseLanguage (main);
   listLang->set_sensitive (main.size ());
}
