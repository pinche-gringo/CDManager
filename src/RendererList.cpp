//$Id: RendererList.cpp,v 1.1 2004/11/06 18:38:02 markus Rel $

/* gtkmm example Copyright (C) 2002 gtkmm development team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

// Everything in this file is combined from the GTKMM exampe for custom
// cellrenderers


#include <algorithm>
#include <memory>

#include <gdk/gdk.h>
#include <gtk/gtkmain.h>

#include <gtkmm/box.h>
#include <gtkmm/entry.h>
#include <gtkmm/arrow.h>
#include <gtkmm/frame.h>
#include <gtkmm/button.h>
#include <gtkmm/editable.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/celleditable.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtkentry.h> /* see XXX below */

#include "RendererList.h"


class PopupEntry : public Gtk::EventBox, public Gtk::CellEditable
{
public:
  explicit PopupEntry(const Glib::ustring& path);
  virtual ~PopupEntry();

  Glib::ustring get_path() const;

  void set_text(const Glib::ustring& text);
  Glib::ustring get_text() const;

  void select_region(int start_pos, int end_pos);

  bool get_editing_canceled() const;

  static int get_button_width();

  typedef sigc::signal<void> type_signal_arrow_clicked;
  type_signal_arrow_clicked& signal_arrow_clicked();

protected:
  virtual bool on_key_press_event(GdkEventKey* event);
  virtual void start_editing_vfunc(GdkEvent* event);

private:
  typedef PopupEntry Self;

  void on_entry_activate();
  bool on_entry_key_press_event(GdkEventKey* event);
  void on_button_clicked();

  Glib::ustring path_;
  Gtk::Button*  button_;
  Gtk::Entry*   entry_;
  bool          editing_canceled_;

  type_signal_arrow_clicked signal_arrow_clicked_;
};


struct PopupColumns : public Gtk::TreeModel::ColumnRecord
{
  Gtk::TreeModelColumn<Glib::ustring> item;
  PopupColumns() { add(item); }
};

const PopupColumns& popup_columns()
{
  static const PopupColumns columns;
  return columns;
}


PopupEntry::PopupEntry(const Glib::ustring& path)
:
  Glib::ObjectBase  (typeid(PopupEntry)),
  Gtk::EventBox     (),
  Gtk::CellEditable (),
  path_             (path),
  button_           (0),
  entry_            (0),
  editing_canceled_ (false)
{
  Gtk::HBox *const hbox = new Gtk::HBox(false, 0);
  add(*Gtk::manage(hbox));

  entry_ = new Gtk::Entry();
  hbox->pack_start(*Gtk::manage(entry_), Gtk::PACK_EXPAND_WIDGET);
  entry_->set_has_frame(false);
  entry_->gobj()->is_cell_renderer = true; // XXX

  button_ = new Gtk::Button();
  hbox->pack_start(*Gtk::manage(button_), Gtk::PACK_SHRINK);
  button_->add(*Gtk::manage(new Gtk::Arrow(Gtk::ARROW_DOWN, Gtk::SHADOW_OUT)));

  set_flags(Gtk::CAN_FOCUS);
  add_events(Gdk::KEY_PRESS_MASK | Gdk::KEY_RELEASE_MASK);

  show_all_children();
}

PopupEntry::~PopupEntry()
{}

Glib::ustring PopupEntry::get_path() const
{
  return path_;
}

void PopupEntry::set_text(const Glib::ustring& text)
{
  entry_->set_text(text);
}

Glib::ustring PopupEntry::get_text() const
{
  return entry_->get_text();
}

void PopupEntry::select_region(int start_pos, int end_pos)
{
  entry_->select_region(start_pos, end_pos);
}

bool PopupEntry::get_editing_canceled() const
{
  return editing_canceled_;
}

// static
int PopupEntry::get_button_width()
{
  Gtk::Window window (Gtk::WINDOW_POPUP);

  Gtk::Button *const button = new Gtk::Button();
  window.add(*Gtk::manage(button));

  button->add(*Gtk::manage(new Gtk::Arrow(Gtk::ARROW_DOWN, Gtk::SHADOW_OUT)));

  // Urgh.  Hackish :/
  window.move(-500, -500);
  window.show_all();

  Gtk::Requisition requisition;
  window.size_request(requisition);

  return requisition.width;
}

PopupEntry::type_signal_arrow_clicked& PopupEntry::signal_arrow_clicked()
{
  return signal_arrow_clicked_;
}

bool PopupEntry::on_key_press_event(GdkEventKey* event)
{
  if(event->keyval == GDK_Escape)
  {
    editing_canceled_ = true;

    editing_done();
    remove_widget();

    return true;
  }

  entry_->grab_focus();

  // Hackish :/ Synthesize a key press event for the entry.

  GdkEvent synth_event;
  synth_event.key = *event;

  synth_event.key.window = Glib::unwrap(entry_->get_window()); // TODO: Use a C++ Gdk::Event.
  synth_event.key.send_event = true;

  entry_->event(&synth_event);

  return Gtk::EventBox::on_key_press_event(event);
}

void PopupEntry::start_editing_vfunc(GdkEvent*)
{
  entry_->select_region(0, -1);

  // TODO: This is a key-binding signal. Investigate whether we really need to use a keybinding signal
  // when creating a derived CellRenderer.
  entry_->signal_activate().connect(sigc::mem_fun(*this, &Self::on_entry_activate));
  entry_->signal_key_press_event().connect(sigc::mem_fun(*this, &Self::on_entry_key_press_event));

  //TODO: Doesn't this mean that we have multiple connection, because this is never disconnected?
  button_->signal_clicked().connect(sigc::mem_fun(*this, &Self::on_button_clicked));
}

void PopupEntry::on_button_clicked()
{
  signal_arrow_clicked_.emit();
}

void PopupEntry::on_entry_activate()
{
  editing_done();
  //remove_widget(); // TODO: this line causes the widget to be removed twice -- dunno why
}

bool PopupEntry::on_entry_key_press_event(GdkEventKey* event)
{
  if(event->keyval == GDK_Escape)
  {
    editing_canceled_ = true;

    editing_done();
    remove_widget();

    return true;
  }

  return false;
}


bool grab_on_window(const Glib::RefPtr<Gdk::Window>& window, guint32 activate_time)
{
  if(window->pointer_grab(true,
        Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::POINTER_MOTION_MASK,
        activate_time) == 0)
  {
    if(window->keyboard_grab(true, activate_time) == 0)
      return true;
    else
      Gdk::Window::pointer_ungrab(activate_time);
  }

  return false;
}


CellRendererPopup::CellRendererPopup()
:
  Glib::ObjectBase      (typeid(CellRendererPopup)),
  Gtk::CellRendererText (),
  button_width_         (-1),
  popup_window_         (Gtk::WINDOW_POPUP),
  focus_widget_         (0),
  popup_entry_          (0),
  shown_                (false),
  editing_canceled_     (false)
{
  signal_show_popup_.connect(sigc::mem_fun(*this, &Self::on_show_popup));
  signal_hide_popup_.connect(sigc::mem_fun(*this, &Self::on_hide_popup));

  popup_window_.signal_button_press_event().connect(sigc::mem_fun(*this, &Self::on_button_press_event));
  popup_window_.signal_key_press_event   ().connect(sigc::mem_fun(*this, &Self::on_key_press_event));
  popup_window_.signal_style_changed     ().connect(sigc::mem_fun(*this, &Self::on_style_changed));
}

CellRendererPopup::~CellRendererPopup()
{}

PopupEntry* CellRendererPopup::get_popup_entry()
{
  return popup_entry_;
}

Gtk::Window* CellRendererPopup::get_popup_window()
{
  return &popup_window_;
}

void CellRendererPopup::set_focus_widget(Gtk::Widget& focus_widget)
{
  focus_widget_ = &focus_widget;
}

Gtk::Widget* CellRendererPopup::get_focus_widget()
{
  return focus_widget_;
}

CellRendererPopup::SignalShowPopup& CellRendererPopup::signal_show_popup()
{
  return signal_show_popup_;
}

CellRendererPopup::SignalHidePopup& CellRendererPopup::signal_hide_popup()
{
  return signal_hide_popup_;
}

void CellRendererPopup::hide_popup()
{
  signal_hide_popup_();
}

void CellRendererPopup::get_size_vfunc(Gtk::Widget& widget,
                                       const Gdk::Rectangle* cell_area,
                                       int* x_offset, int* y_offset,
                                       int* width,    int* height) const
{
  Gtk::CellRendererText::get_size_vfunc(widget, cell_area, x_offset, y_offset, width, height);

  // We cache this because it takes a really long time to get the width.
  if(button_width_ < 0)
    button_width_ = PopupEntry::get_button_width();

  if(width)
    *width += button_width_;
}

Gtk::CellEditable* CellRendererPopup::start_editing_vfunc(GdkEvent*,
                                                          Gtk::Widget&,
                                                          const Glib::ustring& path,
                                                          const Gdk::Rectangle&,
                                                          const Gdk::Rectangle&,
                                                          Gtk::CellRendererState)
{
  // If the cell isn't editable we return 0.
  if(!property_editable())
    return 0;

  std::auto_ptr<PopupEntry> popup_entry (new PopupEntry(path));

  popup_entry->signal_editing_done ().connect(sigc::mem_fun(*this, &Self::on_popup_editing_done));
  popup_entry->signal_arrow_clicked().connect(sigc::mem_fun(*this, &Self::on_popup_arrow_clicked));
  popup_entry->signal_hide         ().connect(sigc::mem_fun(*this, &Self::on_popup_hide));

  popup_entry->set_text(property_text());
  popup_entry->show();

  // Release auto_ptr<> ownership, and let gtkmm manage the widget.
  popup_entry_ = Gtk::manage(popup_entry.release());

  return popup_entry_;
}

void CellRendererPopup::on_show_popup(const Glib::ustring&, int, int y1, int x2, int y2)
{
  // I'm not sure this is ok to do, but we need to show the window to be
  // able to get the allocation right.
  popup_window_.move(-500, -500);
  popup_window_.show();

  const Gtk::Allocation alloc = popup_window_.get_allocation();

  int x = x2;
  int y = y2;

  const int button_height = y2 - y1;

  int       screen_height = Gdk::screen_height() - y;
  const int screen_width  = Gdk::screen_width();

  // Check if it fits in the available height.
  if(alloc.get_height() > screen_height)
  {
    // It doesn't fit, so we see if we have the minimum space needed.
    if((alloc.get_height() > screen_height) && (y - button_height > screen_height))
    {
      // We don't, so we show the popup above the cell instead of below it.
      screen_height = y - button_height;
      y -= (alloc.get_height() + button_height);
      y = std::max(0, y);
    }
  }

  // We try to line it up with the right edge of the column, but we don't
  // want it to go off the edges of the screen.
  x = std::min(x, screen_width);

  x -= alloc.get_width();
  x = std::max(0, x);

  popup_window_.add_modal_grab();

  popup_window_.move(x, y);
  popup_window_.show();

  shown_ = true;

  if(focus_widget_)
    focus_widget_->grab_focus();

  grab_on_window(popup_window_.get_window(), gtk_get_current_event_time());
}

void CellRendererPopup::on_hide_popup()
{
  popup_window_.remove_modal_grab();
  popup_window_.hide();

  if(popup_entry_)
    popup_entry_->editing_done();

  // This may look weird (the test), but the weak pointer will actually
  // be nulled out for some cells, like the date cell.
  if (popup_entry_)
    popup_entry_->remove_widget();

  shown_ = false;
  editing_canceled_ = false;
}

bool CellRendererPopup::on_button_press_event(GdkEventButton* event)
{
  if(event->button != 1)
    return false;

  // If the event happened outside the popup, cancel editing.

  //gdk_event_get_root_coords ((GdkEvent *) event, &x, &y);
  const double x = event->x_root;
  const double y = event->y_root;

  int xoffset = 0, yoffset = 0;
  popup_window_.get_window()->get_root_origin(xoffset, yoffset);

  const Gtk::Allocation alloc = popup_window_.get_allocation();

  xoffset += alloc.get_x();
  yoffset += alloc.get_y();

  const int x1 = alloc.get_x() + xoffset;
  const int y1 = alloc.get_y() + yoffset;
  const int x2 = x1 + alloc.get_width();
  const int y2 = y1 + alloc.get_height();

  if(x > x1 && x < x2 && y > y1 && y < y2)
    return false;

  editing_canceled_ = true;
  signal_hide_popup_();

  return false;
}

bool CellRendererPopup::on_key_press_event(GdkEventKey* event)
{
  switch(event->keyval)
  {
    case GDK_Escape:
      editing_canceled_ = true; break;

    case GDK_Return:
    case GDK_KP_Enter:
    case GDK_ISO_Enter:
    case GDK_3270_Enter:
      editing_canceled_ = false; break;

    default:
      return false;
  }

  signal_hide_popup_();

  return true;
}

void CellRendererPopup::on_style_changed(const Glib::RefPtr<Gtk::Style>&)
{
  // Invalidate the cache.
  button_width_ = -1;
}

void CellRendererPopup::on_popup_editing_done()
{
  if(editing_canceled_ || popup_entry_->get_editing_canceled())
    return;

  edited(popup_entry_->get_path(), popup_entry_->get_text());
}

void CellRendererPopup::on_popup_arrow_clicked()
{
  if(shown_)
  {
    editing_canceled_ = true;
    signal_hide_popup_();
    return;
  }

  if(!grab_on_window(popup_entry_->get_window(), gtk_get_current_event_time()))
    return;

  popup_entry_->select_region(0, 0);

  int x = 0, y = 0;
  popup_entry_->get_window()->get_origin(x, y);

  const Gtk::Allocation alloc = popup_entry_->get_allocation();

  signal_show_popup_(popup_entry_->get_path(), x, y, x + alloc.get_width(), y + alloc.get_height());
}

void CellRendererPopup::on_popup_hide()
{
  popup_entry_ = 0;
}




CellRendererList::CellRendererList()
:
  Glib::ObjectBase  (typeid(CellRendererList)),
  CellRendererPopup (),
  list_store_       (Gtk::ListStore::create(popup_columns())),
  tree_view_        (list_store_)
{
  tree_view_.set_headers_visible(false);
  tree_view_.append_column("", popup_columns().item);
  tree_view_.signal_button_release_event().connect(
      sigc::mem_fun(*this, &Self::on_tree_view_button_release_event));

  const Glib::RefPtr<Gtk::TreeSelection> selection = tree_view_.get_selection();
  selection->set_mode(Gtk::SELECTION_BROWSE);
  selection->signal_changed().connect(sigc::mem_fun(*this, &Self::on_tree_selection_changed));

  Gtk::Frame *const frame = new Gtk::Frame();
  get_popup_window()->add(*Gtk::manage(frame));

  frame->add(tree_view_);
  frame->set_shadow_type(Gtk::SHADOW_OUT);
  frame->show_all();

  set_focus_widget(tree_view_);
}

CellRendererList::~CellRendererList()
{}

void CellRendererList::append_list_item(const Glib::ustring& text)
{
  Gtk::TreeModel::Row row = *list_store_->append();
  row[popup_columns().item] = text;
}

Glib::ustring CellRendererList::get_selected_item()
{
  if(const Gtk::TreeModel::iterator selected = tree_view_.get_selection()->get_selected())
  {
    return (*selected)[popup_columns().item];
  }

  return Glib::ustring();
}

void CellRendererList::on_show_popup(const Glib::ustring& path, int x1, int y1, int x2, int y2)
{
  tree_view_.set_size_request(x2 - x1, -1);

  CellRendererPopup::on_show_popup(path, x1, y1, x2, y2);
}

bool CellRendererList::on_tree_view_button_release_event(GdkEventButton* event)
{
  if(event->button == 1)
  {
    hide_popup();
    return true;
  }

  return false;
}

void CellRendererList::on_tree_selection_changed()
{
  get_popup_entry()->set_text(get_selected_item());
}
