//PROJECT     : CDManager
//SUBSYSTEM   : Language
//REFERENCES  :
//TODO        :
//BUGS        :
//AUTHOR      : Markus Schwab
//CREATED     : 11.12.2004
//COPYRIGHT   : Copyright (C) 2004, 2005, 2009 - 2011, 2026

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
      return Glib::make_refptr_for_instance<LanguageModel> (new LanguageModel (columns, withUndefined));
   }
   ~LanguageModel () { }

   /// Returns the line of the passed language
   Gtk::TreeModel::iterator getLine (const std::string& lang) {
      return children ()[lines[lang]].get_iter (); }

   // Deletes the line with the passed language
   void eraseLanguage (const std::string& lang) {
      TRACE9 ("LanguageModel::eraseLanguage (const std::string&) - " << lang
	      << '=' << lines[lang]);
      erase (getLine (lang));
   }
   void insertLanguage (const std::string& lang, const LanguageColumns& cols) {
      insertLanguage (lang, Language::findLanguage (lang), cols); }

protected:
   LanguageModel (const LanguageColumns& cols, bool withUndefined = false);

   void insertLanguage (const std::string& name, const Language& lang,
			const LanguageColumns& cols);

 private:
   LanguageModel () = delete;
   LanguageModel (const LanguageModel&) = delete;
   LanguageModel& operator= (const LanguageModel&) = delete;

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
   Gtk::TreeModel::iterator iRow;
   if (lines.find (id) == lines.end ()) {
      lines[id] = children ().size ();
      iRow = append ();
   }
   else
      iRow = insert (getLine (id));

   Gtk::TreeModel::Row row (*iRow);
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
   : XGP::XDialog (OKCANCEL),
     pClient (Gtk::make_managed<Gtk::Box> (Gtk::Orientation::VERTICAL, 5)),
     languages (languages), maxLangs (maxLangs),
     mainLang (nullptr),
     listLang (Gtk::make_managed<Gtk::TreeView> ()),
     modelMain (LanguageModel::create (colLang, true)),
     modelList (LanguageModel::create (colLang)) {
   TRACE9 ("LanguageDialog::LanguageDialog (std::string&, unsigned int) - Main: "
	   << languages << '(' << maxLangs << ')');

   set_title (_("Select languages"));

   if (showMainLang) {
      // Label of the main language
      Gtk::Label*  lblLang (Gtk::make_managed<Gtk::Label> (_("_Language: "), true));

      // Combobox to select the main language
      mainLang = Gtk::make_managed<Gtk::ComboBox> ();
      mainLang->pack_start (colLang.flag, false);
      mainLang->pack_start (colLang.name);
      mainLang->set_model (modelMain);
      lblLang->set_mnemonic_widget (*mainLang);

      mainLang->signal_changed ().connect (sigc::mem_fun (*this, &LanguageDialog::selectLanguage));

      pClient->append (*lblLang);
      pClient->append (*mainLang);
   }

   // Listbox to select further languages
   if (maxLangs > 1)
      listLang->get_selection ()->set_mode (Gtk::SelectionMode::MULTIPLE);
   Gtk::TreeViewColumn* column (Gtk::make_managed<Gtk::TreeViewColumn> (_("Translations")));
   column->pack_start (colLang.flag, false);
   column->pack_start (colLang.name);
   listLang->append_column (*column);
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

   listLang->set_expand (true);
   pClient->append (*listLang);
   pClient->set_margin (5);
   pClient->set_expand (true);

   get_content_area ()->append (*pClient);
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
      translations = modelList->get_iter (*i++)->get_value (colLang.id);
      for (unsigned int c (0); (i != list.end ()) && (++c < maxLangs); ++i) {
	 translations.append (1, ',');
	 translations += modelList->get_iter (*i)->get_value (colLang.id);
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
   Check3 (mainLang);
   TRACE9 ("LanguageDialog::selectLanguage () - " << main << "->"
	   << mainLang->get_active ()->get_value (colLang.id));

   if (main.size ())
      modelList->insertLanguage (main, colLang);
   main = mainLang->get_active ()->get_value (colLang.id);

   if (main.size ())
      modelList->eraseLanguage (main);
   listLang->set_sensitive (!main.empty ());
}
