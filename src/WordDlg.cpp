//$Id

//PROJECT     : CDManager
//SUBSYSTEM   : WordDlg
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.1 $
//AUTHOR      : Markus Schwab
//CREATED     : 20.4.2005
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

#include <glibmm/ustring.h>

#include <gtkmm/label.h>
#include <gtkmm/entry.h>
#include <gtkmm/stock.h>
#include <gtkmm/table.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/scrolledwindow.h>

#include <YGP/Check.h>
#include <YGP/Trace.h>

#include "Words.h"

#include "WordDlg.h"


//-----------------------------------------------------------------------------
/// Default constructor
//-----------------------------------------------------------------------------
WordDialog::WordDialog ()
   : Gtk::Table (3, 2),
     names (Gtk::ListStore::create (colWords)),
     articles (Gtk::ListStore::create (colWords)),
     txtName (*manage (new Gtk::Entry)),
     txtArticle (*manage (new Gtk::Entry)),
     addName (*manage (new Gtk::Button (Gtk::Stock::ADD))),
     deleteName (*manage (new Gtk::Button (Gtk::Stock::DELETE))),
     addArticle (*manage (new Gtk::Button (Gtk::Stock::ADD))),
     deleteArticle (*manage (new Gtk::Button (Gtk::Stock::DELETE))),
     lstNames (*manage (new Gtk::TreeView (names))),
     lstArticles (*manage (new Gtk::TreeView (articles))) {
   Gtk::ScrolledWindow& scrlNames (*manage (new Gtk::ScrolledWindow));
   Gtk::ScrolledWindow& scrlArticles (*manage (new Gtk::ScrolledWindow));

   Gtk::HButtonBox& bboxNames (*manage (new Gtk::HButtonBox (Gtk::BUTTONBOX_END, 3)));
   Gtk::HButtonBox& bboxArticle (*manage (new Gtk::HButtonBox (Gtk::BUTTONBOX_END, 3)));

   lstNames.append_column (_("First names"), colWords.word);
   lstArticles.append_column (_("Articles"), colWords.word);

   Gtk::TreeView* views[] = { &lstNames, &lstArticles };
   for (unsigned int i (0); i < (sizeof (views) / sizeof (*views)); ++i) {
      views[i]->set_flags (Gtk::CAN_FOCUS);
      views[i]->set_border_width (1);
      views[i]->set_headers_visible (true);
      views[i]->set_enable_search (true);
      views[i]->set_search_column (colWords.word);

      Glib::RefPtr<Gtk::TreeSelection> listSelection (views[i]->get_selection ());
      listSelection->set_mode (Gtk::SELECTION_EXTENDED);
      listSelection->signal_changed ().connect
	 (bind (mem_fun (*this, &WordDialog::entrySelected), i));
   }
   scrlNames.set_policy (Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
   scrlNames.set_shadow_type (Gtk::SHADOW_ETCHED_IN);
   scrlNames.add (lstNames);

   scrlArticles.set_shadow_type (Gtk::SHADOW_ETCHED_IN);
   scrlArticles.set_policy (Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
   scrlArticles.add (lstArticles);

   txtName.set_max_length (32);
   txtName.set_activates_default (true);
   txtArticle.set_max_length (9);
   txtArticle.set_activates_default (true);

   addName.set_flags (Gtk::CAN_FOCUS);
   deleteName.set_flags (Gtk::CAN_FOCUS);
   addName.set_sensitive (false);
   deleteName.set_sensitive (false);
   bboxNames.pack_start (addName);
   bboxNames.pack_start (deleteName);

   addArticle.set_flags (Gtk::CAN_FOCUS);
   addArticle.set_sensitive (false);
   deleteArticle.set_sensitive (false);
   deleteArticle.set_flags (Gtk::CAN_FOCUS);
   bboxArticle.pack_start (addArticle);
   bboxArticle.pack_start (deleteArticle);

   set_row_spacings (3);
   set_col_spacings (5);
   attach (scrlNames, 0, 1, 0, 1, Gtk::EXPAND | Gtk::FILL, Gtk::EXPAND | Gtk::FILL, 5, 5);
   attach (scrlArticles , 1, 2, 0, 1, Gtk::EXPAND | Gtk::FILL, Gtk::FILL, 5, 5);
   attach (txtName, 0, 1, 1, 2, Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK, 5, 5);
   attach (txtArticle, 1, 2, 1, 2, Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK, 5, 5);
   attach (bboxNames, 0, 1, 2, 3, Gtk::FILL, Gtk::SHRINK, 5, 5);
   attach (bboxArticle, 1, 2, 2, 3, Gtk::FILL, Gtk::SHRINK, 5, 5);

   show_all_children ();

   addName.signal_clicked ().connect (bind (mem_fun (*this, &WordDialog::onAdd), 0));
   deleteName.signal_clicked ().connect (bind (mem_fun (*this, &WordDialog::onDelete), 0));
   addArticle.signal_clicked ().connect (bind (mem_fun (*this, &WordDialog::onAdd), 1));
   deleteArticle.signal_clicked ().connect (bind (mem_fun (*this, &WordDialog::onDelete), 1));

   txtName.signal_changed ().connect
      (bind (mem_fun (*this, &WordDialog::entryChanged), 0));
   txtArticle.signal_changed ().connect
      (bind (mem_fun (*this, &WordDialog::entryChanged), 1));

   // Fill listboxes
   TRACE3 ("WordDialog::WordDialog () - Words: " << Words::names.size ()
	   << "; Articles: " << Words::articles.size ());
   for (std::vector<Glib::ustring>::const_iterator w (Words::names.begin ());
	w != Words::names.end (); ++w)
      appendWord (names, *w);

   for (std::vector<Glib::ustring>::const_iterator a (Words::articles.begin ());
	a != Words::articles.end (); ++a)
      appendWord (articles, *a);

   names->set_sort_column (colWords.word, Gtk::SORT_ASCENDING);
   articles->set_sort_column (colWords.word, Gtk::SORT_ASCENDING);
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
WordDialog::~WordDialog () {
}


//-----------------------------------------------------------------------------
/// Callback after an entryfield has been  changed; the buttons are enabled
/// accordingly.
/// \param which: Flag, which field has been changed
//-----------------------------------------------------------------------------
void WordDialog::entryChanged (unsigned int which) {
   TRACE8 ("WordDialog::entryChanged (unsigned int) - " << which);
   Gtk::Entry* fields[] = { &txtName, &txtArticle };
   Gtk::Button* buttons[] = { &addName, &addArticle };
   std::vector<Glib::ustring>* vectors[] = { &Words::names, &Words::articles };
   Check1 (which < 2);
   Check3 ((sizeof (fields) / sizeof (*fields)) < which);
   Check3 ((sizeof (buttons) / sizeof (*buttons)) < which);
   Check3 ((sizeof (vectors) / sizeof (*vectors)) < which);

   bool unique (false);
   if (fields[which]->get_text_length ()
       && !Words::containsWord (*vectors[which], fields[which]->get_text ()))
      unique = true;
   buttons[which]->set_sensitive (unique);
}

//-----------------------------------------------------------------------------
/// Callback after the selection in a list has been changed; the
/// buttons are enabled accordingly.
/// \param which: Flag, which list has been changed
//-----------------------------------------------------------------------------
void WordDialog::entrySelected (unsigned int which) {
   TRACE8 ("WordDialog::entrySelected (unsigned int) - " << which);
   Gtk::TreeView* lists[] = { &lstNames, &lstArticles };
   Gtk::Button* buttons[] = { &deleteName, &deleteArticle };
   Check1 (which < 2);
   Check3 ((sizeof (lists) / sizeof (*lists)) < which);
   Check3 ((sizeof (buttons) / sizeof (*buttons)) < which);

   buttons[which]->set_sensitive
      (lists[which]->get_selection ()->get_selected_rows ().size ());
}

//-----------------------------------------------------------------------------
/// Appends a line to the passed model
/// \param model: Model to append line to
/// \param value: Value to append
//-----------------------------------------------------------------------------
Gtk::TreeModel::Row WordDialog::appendWord (Glib::RefPtr<Gtk::ListStore>& list,
					    const Glib::ustring& value) {
   TRACE8 ("WordDialog::appendWord (Glib::RefPtr<Gtk::ListStore>&, const Glib::ustring&) - " << value);
   Gtk::TreeModel::Row row (*list->append ());
   row[colWords.word] = value;
   return row;
}

//-----------------------------------------------------------------------------
/// Callback after adding a name
/// \param which: Flag, to which list a value should be added
//-----------------------------------------------------------------------------
void WordDialog::onAdd (unsigned int which) {
   Check1 (which < 2);
   Gtk::Entry* fields[] = { &txtName, &txtArticle };
   Gtk::Button* buttons[] = { &addName, &addArticle };
   Gtk::TreeView* lists[] = { &lstNames, &lstArticles };
   Glib::RefPtr<Gtk::ListStore> models[] = { names, articles };
   Check3 ((sizeof (fields) / sizeof (*fields)) < which);
   Check3 ((sizeof (buttons) / sizeof (*buttons)) < which);
   Check3 ((sizeof (models) / sizeof (*models)) < which);
   Check3 ((sizeof (lists) / sizeof (*lists)) < which);

   Check2 (fields[which]->get_text_length ());
   buttons[which]->set_sensitive (false);
   Gtk::TreeModel::Row row (appendWord (models[which], fields[which]->get_text ()));
   lists[which]->scroll_to_row (models[which]->get_path (row), 0.8);
   Glib::RefPtr<Gtk::TreeSelection> sel (lists[which]->get_selection ());
   sel->unselect_all ();
   sel->select (row);
}

//-----------------------------------------------------------------------------
/// Callback after deleting a name
/// \param which: Flag, from which list a value/values should be removed
//-----------------------------------------------------------------------------
void WordDialog::onDelete (unsigned int which) {
   TRACE8 ("WordDialog::onDelete (unsigned int) - " << which);
   Gtk::TreeView* lists[] = { &lstNames, &lstArticles };
   Glib::RefPtr<Gtk::ListStore> models[] = { names, articles };
   Check1 (which < 2);
   Check3 ((sizeof (lists) / sizeof (*lists)) < which);
   Check3 ((sizeof (models) / sizeof (*models)) < which);

   Glib::RefPtr<Gtk::TreeSelection> selection (lists[which]->get_selection ());
   while (selection->get_selected_rows ().size ()) {
      Gtk::TreeSelection::ListHandle_Path list (selection->get_selected_rows ());
      Check3 (list.size ());
      Gtk::TreeSelection::ListHandle_Path::iterator i (list.begin ());

      Gtk::TreeIter iter (models[which]->get_iter (*i)); Check3 (iter);
      models[which]->erase (iter);
   }
}

//-----------------------------------------------------------------------------
/// Commits the changes within the lists
//-----------------------------------------------------------------------------
void WordDialog::commit () {
   TRACE9 ("WordDialog::commit ()");
   Glib::RefPtr<Gtk::ListStore> models[] = { names, articles };
   std::vector<Glib::ustring>* vectors[] = { &Words::names, &Words::articles };
   Check3 ((sizeof (models) / sizeof (*models))
	   == (sizeof (vectors) / sizeof (*vectors)));

   Glib::ustring value;
   for (unsigned int i (0); i < (sizeof (models) / sizeof (*models)); ++i) {
      // Search for lines existing in the original, but not in the list and
      // delete them
      Gtk::TreeNodeChildren lines (models[i]->children ());
      for (unsigned int j (0); j < vectors[i]->size ();) {
	 value = vectors[i]->at (j);
	 unsigned int first (0);
	 unsigned int last (lines.size ());
	 unsigned int middle;
	 TRACE7 ("WordDialog::commit () - Searching for " << value);
	 Glib::ustring line;

	 while ((last - first) > 0 ) {
	    middle = first + ((last - first) >> 1);
	    line = (*lines[middle])[colWords.word];
	    TRACE9 ("WordDialog::commit () - Comparing " << value << " with " << line);
	    if (line < value)
	       first = middle + 1;
	    else
	       last = middle;
	 }
	 if ((first >= lines.size ())
	     || ((line = (*lines[first])[colWords.word]), (line != value))) {
	    TRACE3 ("WordDialog::commit () - Erasing " << vectors[i]->at (j));
	    vectors[i]->erase (vectors[i]->begin () + j);
	 }
	 else
	    ++j;
      }

      // Search for lines existing in the list, but not in the original and
      // add them
      for (Gtk::TreeModel::const_iterator l (models[i]->children ().begin ());
	   l != models[i]->children ().end (); ++l) {
	 value = (*l)[colWords.word];
	 std::vector<Glib::ustring>::iterator p
	    (upper_bound (vectors[i]->begin (), vectors[i]->end (), value));
	 if ((p == vectors[i]->begin ()) || (value != p[-1])) {
	    TRACE3 ("WordDialog::commit () - Adding " << value);
	    vectors[i]->insert (p, value);
	 }
      }
   }
}

//-----------------------------------------------------------------------------
/// Returns a window allowing to manipulate words and articles.
/// \returns Gtk::Widget: A table holding controls to manipulate the words
/// \remarks The returned window must be freed
//-----------------------------------------------------------------------------
Gtk::Widget* WordDialog::makeDialog () {
   TRACE9 ("Words::makeDialog ()");

   return new WordDialog ();
}

//-----------------------------------------------------------------------------
/// Commits the changes within the lists
/// \param dialog
//-----------------------------------------------------------------------------
void WordDialog::commitDialogData (Gtk::Widget* dialog) {
   Check3 (dialog);
   dynamic_cast<WordDialog*> (dialog)->commit ();
}
