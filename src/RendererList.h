#ifndef RENDERERLIST_H
#define RENDERERLIST_H

//$Id: RendererList.h,v 1.1 2004/11/06 18:38:02 markus Rel $

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


#include <gdkmm.h>
#include <gtkmm/window.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <gtkmm/cellrenderertext.h>


class PopupEntry;


class CellRendererPopup : public Gtk::CellRendererText
{
public:
  CellRendererPopup();
  virtual ~CellRendererPopup();

  PopupEntry*  get_popup_entry();
  Gtk::Window* get_popup_window();

  void set_focus_widget(Gtk::Widget& focus_widget);
  Gtk::Widget* get_focus_widget();

  typedef sigc::signal<void,const Glib::ustring&,int,int,int,int> SignalShowPopup;
  SignalShowPopup& signal_show_popup();

  typedef sigc::signal<void> SignalHidePopup;
  SignalHidePopup& signal_hide_popup();

  void hide_popup();

protected:
  virtual void get_size_vfunc(Gtk::Widget& widget,
                              const Gdk::Rectangle* cell_area,
                              int* x_offset, int* y_offset,
                              int* width,    int* height) const;

  virtual Gtk::CellEditable* start_editing_vfunc(GdkEvent* event,
                                                 Gtk::Widget& widget,
                                                 const Glib::ustring& path,
                                                 const Gdk::Rectangle& background_area,
                                                 const Gdk::Rectangle& cell_area,
                                                 Gtk::CellRendererState flags);

  virtual void on_show_popup(const Glib::ustring& path, int x1, int y1, int x2, int y2);
  virtual void on_hide_popup();

private:
  typedef CellRendererPopup Self;

  SignalShowPopup  signal_show_popup_;
  SignalHidePopup signal_hide_popup_;

  mutable int   button_width_; //mutable because it is just a cache.
  Gtk::Window   popup_window_;
  Gtk::Widget*  focus_widget_;
  PopupEntry*   popup_entry_;
  bool          shown_;
  bool          editing_canceled_;

  bool on_button_press_event(GdkEventButton* event);
  bool on_key_press_event(GdkEventKey* event);
  void on_style_changed(const Glib::RefPtr<Gtk::Style>& previous_style);

  void on_popup_editing_done();
  void on_popup_arrow_clicked();
  void on_popup_hide();
};



class CellRendererList : public CellRendererPopup
{
public:
  CellRendererList();
  virtual ~CellRendererList();

  void append_list_item(const Glib::ustring& text);
  Glib::ustring get_selected_item();

protected:
  virtual void on_show_popup(const Glib::ustring& path, int x1, int y1, int x2, int y2);

private:
  typedef CellRendererList Self;

  Glib::RefPtr<Gtk::ListStore>  list_store_;
  Gtk::TreeView                 tree_view_;

  bool on_tree_view_button_release_event(GdkEventButton* event);
  void on_tree_selection_changed();
};

#endif
