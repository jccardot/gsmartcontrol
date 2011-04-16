/**************************************************************************
 Copyright:
      (C) 2011  Alexander Shaduri <ashaduri 'at' gmail.com>
 License: See LICENSE_gsmartcontrol.txt
***************************************************************************/

#include <gtkmm/button.h>
#include <gtkmm/entry.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/stock.h>
#include <gdk/gdkkeysyms.h>  // GDK_Escape

#include "hz/fs_path.h"
#include "hz/string_sprintf.h"
#include "applib/app_gtkmm_utils.h"

#include "gsc_add_device_window.h"
#include "gsc_main_window.h"




GscAddDeviceWindow::GscAddDeviceWindow(BaseObjectType* gtkcobj, const app_ui_res_ref_t& ref_ui)
		: AppUIResWidget<GscAddDeviceWindow, true>(gtkcobj, ref_ui), main_window_(0)
{
	// Connect callbacks

	APP_GTKMM_CONNECT_VIRTUAL(delete_event);  // make sure the event handler is called

	Gtk::Button* window_cancel_button = 0;
	APP_UI_RES_AUTO_CONNECT(window_cancel_button, clicked);

	Gtk::Button* window_ok_button = 0;
	APP_UI_RES_AUTO_CONNECT(window_ok_button, clicked);

	Gtk::Button* device_name_browse_button = 0;
	APP_UI_RES_AUTO_CONNECT(device_name_browse_button, clicked);


	Glib::ustring device_name_tooltip = "Device name, e.g. %s.";
#ifdef _WIN32
	device_name_tooltip = hz::string_sprintf(device_name_tooltip.c_str(), "\"pd0\" for the first physical drive");
#else
	device_name_tooltip = hz::string_sprintf(device_name_tooltip.c_str(), "\"/dev/sda\" or \"/dev/twa0\"");
#endif
	Gtk::Label* device_name_label = lookup_widget<Gtk::Label*>("device_name_label");
	app_gtkmm_set_widget_tooltip(*device_name_label, device_name_tooltip);


	Gtk::Entry* device_name_entry = 0;
	APP_UI_RES_AUTO_CONNECT(device_name_entry, changed);


	// Accelerators

	Glib::RefPtr<Gtk::AccelGroup> accel_group = this->get_accel_group();
	if (window_cancel_button) {
		window_cancel_button->add_accelerator("clicked", accel_group, GDK_Escape,
				Gdk::ModifierType(0), Gtk::AccelFlags(0));
	}


#ifndef _WIN32
	// "Browse" doesn't make sense in win32, hide it.
	if (device_name_browse_button) {
		device_name_browse_button->hide();
	}
#endif


	// initial state of OK button
	on_device_name_entry_changed();

	// show();
}



void GscAddDeviceWindow::set_main_window(GscMainWindow* main_window)
{
	main_window_ = main_window;
}



void GscAddDeviceWindow::on_window_cancel_button_clicked()
{
	destroy(this);
}



void GscAddDeviceWindow::on_window_ok_button_clicked()
{
	std::string dev, type, params;
	if (Gtk::Entry* entry = lookup_widget<Gtk::Entry*>("device_name_entry")) {
		dev = entry->get_text();
	}
	if (Gtk::Entry* entry = lookup_widget<Gtk::Entry*>("device_type_entry")) {
		type = entry->get_text();
	}
	if (Gtk::Entry* entry = lookup_widget<Gtk::Entry*>("smartctl_params_entry")) {
		params = entry->get_text();
	}
	if (main_window_ && !dev.empty()) {
		main_window_->add_device(dev, type, params);
	}

	destroy(this);
}



void GscAddDeviceWindow::on_device_name_browse_button_clicked()
{
	std::string default_file;

	Gtk::Entry* entry = this->lookup_widget<Gtk::Entry*>("device_name_entry");
	if (!entry)
		return;

	Gtk::FileChooserDialog dialog(*this, "Choose Device...",
			Gtk::FILE_CHOOSER_ACTION_OPEN);

	// Add response buttons the the dialog
	dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_ACCEPT);

	// Note: This works on absolute paths only (otherwise it's gtk warning).
	hz::FsPath p(entry->get_text());
	if (p.is_absolute())
		dialog.set_filename(p.str());  // change to its dir and select it if exists.

	// Show the dialog and wait for a user response
	int result = dialog.run();  // the main cycle blocks here

	// Handle the response
	switch (result) {
		case Gtk::RESPONSE_ACCEPT:
		{
			entry->set_text(std::string(dialog.get_filename()));
			break;
		}

		case Gtk::RESPONSE_CANCEL: case Gtk::RESPONSE_DELETE_EVENT:
			// nothing, the dialog is closed already
			break;

		default:
			debug_out_error("app", DBG_FUNC_MSG << "Unknown dialog response code: " << result << ".\n");
			break;
	}

}



void GscAddDeviceWindow::on_device_name_entry_changed()
{
	// Allow OK only if name is not empty
	Gtk::Entry* entry = lookup_widget<Gtk::Entry*>("device_name_entry");
	Gtk::Button* ok_button = lookup_widget<Gtk::Button*>("window_ok_button");
	if (entry && ok_button) {
		ok_button->set_sensitive(!entry->get_text().empty());
	}
}







