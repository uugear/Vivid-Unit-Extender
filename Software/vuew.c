#include <gtk/gtk.h>
#include "vuelib.h"

#define FACTORY_RESET_ALL           0
#define TEMPERATURE_CONDITION_1     1
#define AUTO_ON                     2
#define TEMPERATURE_CONDITION_2     3
#define POWER_CUT                   4
#define TEMPERATURE_CONDITION_3     5
#define WATCHDOG                    6
#define VIN_CALIBRATION             7
#define OVERVOLTAGE                 8
#define VOUT_CALIBRATION            9
#define UNDERVOLTAGE                10
#define IOUT_CALIBRATION            11

#define FACTORY_RESET               "Reset to factory settings?"

const char *button_icons[12] = {
        "/usr/share/icons/hicolor/48x48/apps/vuew_reset.png",
        "/usr/share/icons/hicolor/48x48/apps/vuew_temp_cond1.png",
        "/usr/share/icons/hicolor/48x48/apps/vuew_auto_on.png",
        "/usr/share/icons/hicolor/48x48/apps/vuew_temp_cond2.png",
        "/usr/share/icons/hicolor/48x48/apps/vuew_power_cut.png",
        "/usr/share/icons/hicolor/48x48/apps/vuew_temp_cond3.png",
        "/usr/share/icons/hicolor/48x48/apps/vuew_watchdog.png",
        "/usr/share/icons/hicolor/48x48/apps/vuew_vin_cali.png",
        "/usr/share/icons/hicolor/48x48/apps/vuew_overvoltage.png",
        "/usr/share/icons/hicolor/48x48/apps/vuew_vout_cali.png",
        "/usr/share/icons/hicolor/48x48/apps/vuew_undervoltage.png",
        "/usr/share/icons/hicolor/48x48/apps/vuew_iout_cali.png"
    };

void on_edit_temp_cond(GtkWidget *widget, gpointer data);
void on_edit_temp_cond(GtkWidget *widget, gpointer data);
void on_edit_auto_on(GtkWidget *widget, gpointer data);
void on_edit_power_cut(GtkWidget *widget, gpointer data);
void on_edit_watchdog(GtkWidget *widget, gpointer data);
void on_edit_overvoltage(GtkWidget *widget, gpointer data);
void on_edit_undervoltage(GtkWidget *widget, gpointer data);
void on_edit_vin_calibration(GtkWidget *widget, gpointer data);
void on_edit_vout_calibration(GtkWidget *widget, gpointer data);
void on_edit_iout_calibration(GtkWidget *widget, gpointer data);
void on_factory_reset_all(GtkWidget *widget, gpointer data);

void (*on_edit_funcs[12])(GtkWidget *button, gpointer data) = {
    on_factory_reset_all,
    on_edit_temp_cond,
    on_edit_auto_on,
    on_edit_temp_cond,
    on_edit_power_cut,
    on_edit_temp_cond,
    on_edit_watchdog,
    on_edit_vin_calibration,
    on_edit_overvoltage,
    on_edit_vout_calibration,
    on_edit_undervoltage,
    on_edit_iout_calibration
};

char info[128];

int i2c_fd = -1;

GtkWidget *label_voltage_current, *label_watchdog, *label_cpu_gpu_fan;
GtkWidget *button_labels[12];


gboolean show_confirmation_dialog(GtkWidget *parent, const gchar *message) {
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(parent),
                                               GTK_DIALOG_MODAL,
                                               GTK_MESSAGE_QUESTION,
                                               GTK_BUTTONS_OK_CANCEL,
                                               "%s", message);
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    return response == GTK_RESPONSE_OK;
}

void update_button_labels(int index) {
    
    for (int i = 0; i < 3; i ++) {
        if (index == -1 
            || (i == 0 && index == TEMPERATURE_CONDITION_1) 
            || (i == 1 && index == TEMPERATURE_CONDITION_2) 
            || (i == 2 && index == TEMPERATURE_CONDITION_3)) {
            int cond = get_temp_cond_funcs[i](i2c_fd);
            if (cond == 0) {
                sprintf(info, "Temperature Condition %d is disabled", i + 1);
            } else {
                sprintf(info, "CPU %s GPU temperature ≥ %d°C, fan speed ➔ %d%%", (cond & 0x80) > 0 ? "and" : "or", (cond & 0x7F), get_fan_speed_funcs[i](i2c_fd));
            }
            gtk_label_set_text(GTK_LABEL(button_labels[i * 2 + 1]), info);
        }
    }
    
    if (index == -1 || index == AUTO_ON) {
        int auto_on_delay = get_auto_on_delay(i2c_fd);
        if (auto_on_delay == 0) {
            sprintf(info, "Auto-ON when powered is disabled");
        } else {
            sprintf(info, "Auto-ON when powered with %d second%s delay", auto_on_delay, auto_on_delay > 1 ? "s" : "");
        }
        gtk_label_set_text(GTK_LABEL(button_labels[AUTO_ON]), info);
    }
    
    if (index == -1 || index == POWER_CUT) {
        int power_cut_delay = get_power_cut_delay(i2c_fd);
        if (power_cut_delay == 0) {
            sprintf(info, "Power-cut after shutdown is disabled");
        } else {
            sprintf(info, "Power-cut after shutdown with %d second%s delay", power_cut_delay, power_cut_delay > 1 ? "s" : "");
        }
        gtk_label_set_text(GTK_LABEL(button_labels[POWER_CUT]), info);
    }
    
    if (index == -1 || index == WATCHDOG) {
        int watchdog_threshold = get_watchdog_threshold(i2c_fd);
        if (watchdog_threshold == 0) {
            sprintf(info, "Watchdog is disabled");
        } else {
            sprintf(info, "Watchdog: allow %d missed heartbeat%s", watchdog_threshold, watchdog_threshold > 1 ? "s" : "");
        }
        gtk_label_set_text(GTK_LABEL(button_labels[WATCHDOG]), info);
    }
    
    if (index == -1 || index == OVERVOLTAGE) {
        int overvoltage = get_overvoltage_tolerance(i2c_fd);
        if (overvoltage == 0) {
            sprintf(info, "Overvoltage protection is disabled");
        } else {
            sprintf(info, "Overvoltage tolerance: %.2fV", (float)overvoltage / 100.0f);
        }
        gtk_label_set_text(GTK_LABEL(button_labels[OVERVOLTAGE]), info);
    }
    
    if (index == -1 || index == UNDERVOLTAGE) {
        int undervoltage = get_undervoltage_tolerance(i2c_fd);
        if (undervoltage == 0) {
            sprintf(info, "Undervoltage protection is disabled");
        } else {
            sprintf(info, "Undervoltage tolerance: %.2fV", (float)undervoltage / 100.0f);
        }
        gtk_label_set_text(GTK_LABEL(button_labels[UNDERVOLTAGE]), info);
    }
    
    if (index == -1 || index == VIN_CALIBRATION) {
        int vin_calibration = get_signed_char(get_vin_calibration(i2c_fd));
        sprintf(info, "Input voltage calibration: %+.2fV", (float)vin_calibration / 100.0f);
        gtk_label_set_text(GTK_LABEL(button_labels[VIN_CALIBRATION]), info);
    }
    
    if (index == -1 || index == VOUT_CALIBRATION) {
        int vout_calibration = get_signed_char(get_vout_calibration(i2c_fd));
        sprintf(info, "Output voltage calibration: %+.2fV", (float)vout_calibration / 100.0f);
        gtk_label_set_text(GTK_LABEL(button_labels[VOUT_CALIBRATION]), info);
    }
    
    if (index == -1 || index == IOUT_CALIBRATION) {
        int iout_calibration = get_signed_char(get_iout_calibration(i2c_fd));
        sprintf(info, "Output current calibration: %+.2fA", (float)iout_calibration / 100.0f);
        gtk_label_set_text(GTK_LABEL(button_labels[IOUT_CALIBRATION]), info);
    }
}

void update_text() {
    int via_extender = is_powered_via_extender(i2c_fd);
    
    if (via_extender) {
        sprintf(info, "Vin: %.3fV   Vout: %.3fV   Iout: %.3fA", get_vin(i2c_fd, true), get_vout(i2c_fd, true), get_iout(i2c_fd, true));
    } else {
        sprintf(info, "Device is not powered via Vivid Unit Extender");
    }
    gtk_label_set_text(GTK_LABEL(label_voltage_current), info);
    
    sprintf(info, "Heartbeat Counter: %d    Missed Hartbeats: %d", get_heartbeat_counter(i2c_fd), get_missed_heartbeats(i2c_fd));
    gtk_label_set_text(GTK_LABEL(label_watchdog), info);
    
    sprintf(info, "CPU: %d°C    GPU: %d°C    Fan Speed: %d%%", get_cpu_temperature(i2c_fd), get_gpu_temperature(i2c_fd), get_fan_speed(i2c_fd));
    gtk_label_set_text(GTK_LABEL(label_cpu_gpu_fan), info);
}

gboolean on_timeout_update_text(gpointer data) {
    update_text();
    return TRUE;
}


void set_label_attributes(GtkLabel *label) {
    PangoAttrList *attrs = pango_attr_list_new();
    pango_attr_list_insert(attrs, pango_attr_weight_new(PANGO_WEIGHT_BOLD));
    gtk_label_set_attributes(label, attrs);
    pango_attr_list_unref(attrs);
}


void on_temp_cond_toggled(GtkToggleButton *toggle_button, gpointer data) {
    GtkWidget *combo = GTK_WIDGET(g_object_get_data(G_OBJECT(toggle_button), "combo"));
    GtkWidget *scale_temp = GTK_WIDGET(g_object_get_data(G_OBJECT(toggle_button), "scale_temp"));
    GtkWidget *scale_speed = GTK_WIDGET(g_object_get_data(G_OBJECT(toggle_button), "scale_speed"));
    
    gboolean is_active = gtk_toggle_button_get_active(toggle_button);
    gtk_widget_set_sensitive(combo, is_active);
    gtk_widget_set_sensitive(scale_temp, is_active);
    gtk_widget_set_sensitive(scale_speed, is_active);
}

void on_temp_cond_reset(GtkWidget *widget, gpointer data) {
    if (show_confirmation_dialog(GTK_WIDGET(data), FACTORY_RESET)) {
        GtkWidget *checkbox = GTK_WIDGET(g_object_get_data(G_OBJECT(data), "checkbox"));
        GtkWidget *combo = GTK_WIDGET(g_object_get_data(G_OBJECT(checkbox), "combo"));
        GtkWidget *scale_temp = GTK_WIDGET(g_object_get_data(G_OBJECT(checkbox), "scale_temp"));
        GtkWidget *scale_speed = GTK_WIDGET(g_object_get_data(G_OBJECT(checkbox), "scale_speed"));
        const char *name = gtk_widget_get_name(GTK_WIDGET(data));
        int cond_id = atoi(name);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbox), true);
        gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
        switch (cond_id) {
          case 1:
          gtk_range_set_value(GTK_RANGE(scale_temp), 65);
          gtk_range_set_value(GTK_RANGE(scale_speed), 100);
          break;
          case 2:
          gtk_range_set_value(GTK_RANGE(scale_temp), 60);
          gtk_range_set_value(GTK_RANGE(scale_speed), 80);
          break;
          case 3:
          gtk_range_set_value(GTK_RANGE(scale_temp), 55);
          gtk_range_set_value(GTK_RANGE(scale_speed), 65);
          break;
        }
    }
}

void on_temp_cond_change(GtkWidget *widget, gpointer data) {
    GtkWidget *checkbox = GTK_WIDGET(g_object_get_data(G_OBJECT(data), "checkbox"));
    GtkWidget *combo = GTK_WIDGET(g_object_get_data(G_OBJECT(checkbox), "combo"));
    GtkWidget *scale_temp = GTK_WIDGET(g_object_get_data(G_OBJECT(checkbox), "scale_temp"));
    GtkWidget *scale_speed = GTK_WIDGET(g_object_get_data(G_OBJECT(checkbox), "scale_speed"));
    const char *name = gtk_widget_get_name(GTK_WIDGET(data));
    int cond_index = atoi(name) - 1;
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbox))) {
        int cond = gtk_range_get_value(GTK_RANGE(scale_temp));
        if (gtk_combo_box_get_active(GTK_COMBO_BOX(combo)) == 1) {
          cond |= 0x80;
        }
        set_temp_cond_funcs[cond_index](i2c_fd, cond);
        set_fan_speed_funcs[cond_index](i2c_fd, gtk_range_get_value(GTK_RANGE(scale_speed)));
    } else {
        set_temp_cond_funcs[cond_index](i2c_fd, 0);
    }
    update_button_labels(cond_index * 2 + 1);
    gtk_widget_destroy(GTK_WIDGET(data));
}

// edit temperature condition (1, 2, 3)
void on_edit_temp_cond(GtkWidget *widget, gpointer data) {

    const char *name = gtk_widget_get_name(widget);
    int cond_id = atoi(&name[strlen(name) - 1]);
    int cond = get_temp_cond_funcs[cond_id - 1](i2c_fd);
    int speed = get_fan_speed_funcs[cond_id - 1](i2c_fd);

    sprintf(info, "Edit %s", name);
    GtkWidget *dialog = gtk_dialog_new_with_buttons(info, GTK_WINDOW(data), GTK_DIALOG_MODAL, NULL);
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_set_border_width(GTK_CONTAINER(content_area), 10);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 300, 100);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(content_area), vbox);
    
    sprintf(info, "Enable %s", name);
    GtkWidget *checkbox = gtk_check_button_new_with_label(info);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbox), cond > 0);
    gtk_box_pack_start(GTK_BOX(vbox), checkbox, FALSE, FALSE, 5);

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *label1 = gtk_label_new("If CPU ");
    GtkWidget *combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), "or");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), "and");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), (cond & 0x80) > 0 ? 1 : 0);
    gtk_box_pack_start(GTK_BOX(hbox), label1, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), combo, FALSE, FALSE, 0);


    GtkWidget *scale_temp = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 1, 127, 1);
    gtk_range_set_value(GTK_RANGE(scale_temp), (cond & 0x7F));
    gtk_scale_set_draw_value(GTK_SCALE(scale_temp), TRUE);
    gtk_widget_set_size_request(scale_temp, 500, -1);
    gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new(" GPU temperature >= "), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), scale_temp, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new("°C"), FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *label2 = gtk_label_new("Set fan speed to ");

    GtkWidget *scale_speed = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 100, 1);
    gtk_range_set_value(GTK_RANGE(scale_speed), speed);
    gtk_scale_set_draw_value(GTK_SCALE(scale_speed), TRUE);
    gtk_widget_set_size_request(scale_speed, 500, -1);
    gtk_box_pack_start(GTK_BOX(hbox), label2, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), scale_speed, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new("%"), FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *btn_reset = gtk_button_new_with_label("Reset");
    GtkWidget *btn_change = gtk_button_new_with_label("Change");
    gtk_box_pack_end(GTK_BOX(hbox), btn_change, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(hbox), btn_reset, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
    
    g_object_set_data(G_OBJECT(checkbox), "combo", combo);
    g_object_set_data(G_OBJECT(checkbox), "scale_temp", scale_temp);
    g_object_set_data(G_OBJECT(checkbox), "scale_speed", scale_speed);
    g_signal_connect(checkbox, "toggled", G_CALLBACK(on_temp_cond_toggled), NULL);
    
    on_temp_cond_toggled(GTK_TOGGLE_BUTTON(checkbox), NULL);
    
    g_object_set_data(G_OBJECT(dialog), "checkbox", checkbox);
    sprintf(info, "%d", cond_id);
    gtk_widget_set_name(dialog, info);
    
    g_signal_connect(btn_reset, "clicked", G_CALLBACK(on_temp_cond_reset), dialog);
    g_signal_connect(btn_change, "clicked", G_CALLBACK(on_temp_cond_change), dialog);

    gtk_widget_show_all(dialog);

    g_signal_connect_swapped(dialog, "response", G_CALLBACK(gtk_widget_destroy), dialog);
}

void on_auto_on_reset(GtkWidget *widget, gpointer data) {
    if (show_confirmation_dialog(GTK_WIDGET(data), FACTORY_RESET)) {
        GtkWidget *checkbox = GTK_WIDGET(g_object_get_data(G_OBJECT(data), "checkbox"));
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbox), false);
        GtkWidget *scale_delay = GTK_WIDGET(g_object_get_data(G_OBJECT(checkbox), "scale_delay"));
        gtk_range_set_value(GTK_RANGE(scale_delay), 1);
    }
}

void on_auto_on_change(GtkWidget *widget, gpointer data) {
    GtkWidget *checkbox = GTK_WIDGET(g_object_get_data(G_OBJECT(data), "checkbox"));
    GtkWidget *scale_delay = GTK_WIDGET(g_object_get_data(G_OBJECT(checkbox), "scale_delay"));
    int delay = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbox)) ? gtk_range_get_value(GTK_RANGE(scale_delay)) : 0;
    set_auto_on_delay(i2c_fd, delay);
    update_button_labels(AUTO_ON);
    gtk_widget_destroy(GTK_WIDGET(data));
}

void on_auto_on_toggled(GtkToggleButton *toggle_button, gpointer data) {
    GtkWidget *scale_delay = GTK_WIDGET(g_object_get_data(G_OBJECT(toggle_button), "scale_delay"));
    gboolean is_active = gtk_toggle_button_get_active(toggle_button);
    gtk_widget_set_sensitive(scale_delay, is_active);
}

void on_edit_auto_on(GtkWidget *button, gpointer data) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Configure Auto-ON", GTK_WINDOW(data), GTK_DIALOG_MODAL, NULL);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
    gtk_container_add(GTK_CONTAINER(content_area), vbox);

    int auto_on_delay = get_auto_on_delay(i2c_fd);
    GtkWidget *checkbox = gtk_check_button_new_with_label("Enable Auto-ON when powered");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbox), auto_on_delay > 0);
    gtk_box_pack_start(GTK_BOX(vbox), checkbox, FALSE, FALSE, 5);

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *label_delay = gtk_label_new("Auto-ON with");
    gtk_box_pack_start(GTK_BOX(hbox), label_delay, FALSE, FALSE, 5);

    GtkWidget *scale_delay = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 1, 255, 1);
    gtk_range_set_value(GTK_RANGE(scale_delay), auto_on_delay);
    gtk_scale_set_draw_value(GTK_SCALE(scale_delay), TRUE);
    gtk_widget_set_size_request(scale_delay, 500, -1);
    gtk_box_pack_start(GTK_BOX(hbox), scale_delay, TRUE, TRUE, 0);

    GtkWidget *label_seconds = gtk_label_new("seconds delay");
    gtk_box_pack_start(GTK_BOX(hbox), label_seconds, FALSE, FALSE, 5);

    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);

    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *btn_reset = gtk_button_new_with_label("Reset");
    GtkWidget *btn_change = gtk_button_new_with_label("Change");
    gtk_box_pack_end(GTK_BOX(hbox), btn_change, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(hbox), btn_reset, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
    
    g_object_set_data(G_OBJECT(checkbox), "scale_delay", scale_delay);

    g_signal_connect(checkbox, "toggled", G_CALLBACK(on_auto_on_toggled), NULL);
    
    on_auto_on_toggled(GTK_TOGGLE_BUTTON(checkbox), NULL);
    
    g_object_set_data(G_OBJECT(dialog), "checkbox", checkbox);
    
    g_signal_connect(btn_reset, "clicked", G_CALLBACK(on_auto_on_reset), dialog);
    
    g_signal_connect(btn_change, "clicked", G_CALLBACK(on_auto_on_change), dialog);

    gtk_widget_show_all(dialog);

    g_signal_connect_swapped(dialog, "response", G_CALLBACK(gtk_widget_destroy), dialog);
}

void on_power_cut_reset(GtkWidget *widget, gpointer data) {
    if (show_confirmation_dialog(GTK_WIDGET(data), FACTORY_RESET)) {
        GtkWidget *checkbox = GTK_WIDGET(g_object_get_data(G_OBJECT(data), "checkbox"));
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbox), true);
        GtkWidget *scale_delay = GTK_WIDGET(g_object_get_data(G_OBJECT(checkbox), "scale_delay"));
        gtk_range_set_value(GTK_RANGE(scale_delay), 1);
    }
}

void on_power_cut_change(GtkWidget *widget, gpointer data) {
    GtkWidget *checkbox = GTK_WIDGET(g_object_get_data(G_OBJECT(data), "checkbox"));
    GtkWidget *scale_delay = GTK_WIDGET(g_object_get_data(G_OBJECT(checkbox), "scale_delay"));
    int delay = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbox)) ? gtk_range_get_value(GTK_RANGE(scale_delay)) : 0;
    set_power_cut_delay(i2c_fd, delay);
    update_button_labels(POWER_CUT);
    gtk_widget_destroy(GTK_WIDGET(data));
}

void on_power_cut_toggled(GtkToggleButton *toggle_button, gpointer data) {
    GtkWidget *scale_delay = GTK_WIDGET(g_object_get_data(G_OBJECT(toggle_button), "scale_delay"));
    gboolean is_active = gtk_toggle_button_get_active(toggle_button);
    gtk_widget_set_sensitive(scale_delay, is_active);
}

void on_edit_power_cut(GtkWidget *button, gpointer data) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Configure Power-cut", GTK_WINDOW(data), GTK_DIALOG_MODAL, NULL);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
    gtk_container_add(GTK_CONTAINER(content_area), vbox);

    int power_cut_delay = get_power_cut_delay(i2c_fd);
    GtkWidget *checkbox = gtk_check_button_new_with_label("Enable Power-cut after shutdown");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbox), power_cut_delay > 0);
    gtk_box_pack_start(GTK_BOX(vbox), checkbox, FALSE, FALSE, 5);

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *label_delay = gtk_label_new("Power-cut with");
    gtk_box_pack_start(GTK_BOX(hbox), label_delay, FALSE, FALSE, 5);

    GtkWidget *scale_delay = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 1, 255, 1);
    gtk_range_set_value(GTK_RANGE(scale_delay), power_cut_delay);
    gtk_scale_set_draw_value(GTK_SCALE(scale_delay), TRUE);
    gtk_widget_set_size_request(scale_delay, 500, -1);
    gtk_box_pack_start(GTK_BOX(hbox), scale_delay, TRUE, TRUE, 0);

    GtkWidget *label_seconds = gtk_label_new("seconds delay");
    gtk_box_pack_start(GTK_BOX(hbox), label_seconds, FALSE, FALSE, 5);

    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);

    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *btn_reset = gtk_button_new_with_label("Reset");
    GtkWidget *btn_change = gtk_button_new_with_label("Change");
    gtk_box_pack_end(GTK_BOX(hbox), btn_change, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(hbox), btn_reset, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
    
    g_object_set_data(G_OBJECT(checkbox), "scale_delay", scale_delay);

    g_signal_connect(checkbox, "toggled", G_CALLBACK(on_power_cut_toggled), NULL);
    
    on_power_cut_toggled(GTK_TOGGLE_BUTTON(checkbox), NULL);
    
    g_object_set_data(G_OBJECT(dialog), "checkbox", checkbox);
    
    g_signal_connect(btn_reset, "clicked", G_CALLBACK(on_power_cut_reset), dialog);
    
    g_signal_connect(btn_change, "clicked", G_CALLBACK(on_power_cut_change), dialog);

    gtk_widget_show_all(dialog);

    g_signal_connect_swapped(dialog, "response", G_CALLBACK(gtk_widget_destroy), dialog);
}

void on_watchdog_reset(GtkWidget *widget, gpointer data) {
    if (show_confirmation_dialog(GTK_WIDGET(data), FACTORY_RESET)) {
        GtkWidget *checkbox = GTK_WIDGET(g_object_get_data(G_OBJECT(data), "checkbox"));
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbox), true);
        GtkWidget *scale_threshold = GTK_WIDGET(g_object_get_data(G_OBJECT(checkbox), "scale_threshold"));
        gtk_range_set_value(GTK_RANGE(scale_threshold), 6);
    }
}

void on_watchdog_change(GtkWidget *widget, gpointer data) {
    GtkWidget *checkbox = GTK_WIDGET(g_object_get_data(G_OBJECT(data), "checkbox"));
    GtkWidget *scale_threshold = GTK_WIDGET(g_object_get_data(G_OBJECT(checkbox), "scale_threshold"));
    int threshold = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbox)) ? gtk_range_get_value(GTK_RANGE(scale_threshold)) : 0;
    set_watchdog_threshold(i2c_fd, threshold);
    update_button_labels(WATCHDOG);
    gtk_widget_destroy(GTK_WIDGET(data));
}

void on_watchdog_toggled(GtkToggleButton *toggle_button, gpointer data) {
    GtkWidget *scale_threshold = GTK_WIDGET(g_object_get_data(G_OBJECT(toggle_button), "scale_threshold"));
    gboolean is_active = gtk_toggle_button_get_active(toggle_button);
    gtk_widget_set_sensitive(scale_threshold, is_active);
}

void on_edit_watchdog(GtkWidget *button, gpointer data) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Configure Watchdog", GTK_WINDOW(data), GTK_DIALOG_MODAL, NULL);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
    gtk_container_add(GTK_CONTAINER(content_area), vbox);

    int threshold = get_watchdog_threshold(i2c_fd);
    GtkWidget *checkbox = gtk_check_button_new_with_label("Enable Watchdog");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbox), threshold > 0);
    gtk_box_pack_start(GTK_BOX(vbox), checkbox, FALSE, FALSE, 5);

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *label_1 = gtk_label_new("Allow");
    gtk_box_pack_start(GTK_BOX(hbox), label_1, FALSE, FALSE, 5);

    GtkWidget *scale_threshold = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 1, 255, 1);
    gtk_range_set_value(GTK_RANGE(scale_threshold), threshold);
    gtk_scale_set_draw_value(GTK_SCALE(scale_threshold), TRUE);
    gtk_widget_set_size_request(scale_threshold, 500, -1);
    gtk_box_pack_start(GTK_BOX(hbox), scale_threshold, TRUE, TRUE, 0);

    GtkWidget *label_2 = gtk_label_new("missed heartbeats");
    gtk_box_pack_start(GTK_BOX(hbox), label_2, FALSE, FALSE, 5);

    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);

    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *btn_reset = gtk_button_new_with_label("Reset");
    GtkWidget *btn_change = gtk_button_new_with_label("Change");
    gtk_box_pack_end(GTK_BOX(hbox), btn_change, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(hbox), btn_reset, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
    
    g_object_set_data(G_OBJECT(checkbox), "scale_threshold", scale_threshold);

    g_signal_connect(checkbox, "toggled", G_CALLBACK(on_watchdog_toggled), NULL);
    
    on_watchdog_toggled(GTK_TOGGLE_BUTTON(checkbox), NULL);
    
    g_object_set_data(G_OBJECT(dialog), "checkbox", checkbox);
    
    g_signal_connect(btn_reset, "clicked", G_CALLBACK(on_watchdog_reset), dialog);
    
    g_signal_connect(btn_change, "clicked", G_CALLBACK(on_watchdog_change), dialog);

    gtk_widget_show_all(dialog);

    g_signal_connect_swapped(dialog, "response", G_CALLBACK(gtk_widget_destroy), dialog);
}

void on_overvoltage_reset(GtkWidget *widget, gpointer data) {
    if (show_confirmation_dialog(GTK_WIDGET(data), FACTORY_RESET)) {
        GtkWidget *checkbox = GTK_WIDGET(g_object_get_data(G_OBJECT(data), "checkbox"));
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbox), true);
        GtkWidget *scale_tolerance = GTK_WIDGET(g_object_get_data(G_OBJECT(checkbox), "scale_tolerance"));
        gtk_range_set_value(GTK_RANGE(scale_tolerance), 0.5f);
    }
}

void on_overvoltage_change(GtkWidget *widget, gpointer data) {
    GtkWidget *checkbox = GTK_WIDGET(g_object_get_data(G_OBJECT(data), "checkbox"));
    GtkWidget *scale_tolerance = GTK_WIDGET(g_object_get_data(G_OBJECT(checkbox), "scale_tolerance"));
    int tolerance = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbox)) ? (int)(100 * gtk_range_get_value(GTK_RANGE(scale_tolerance))) : 0;
    set_overvoltage_tolerance(i2c_fd, tolerance);
    update_button_labels(OVERVOLTAGE);
    gtk_widget_destroy(GTK_WIDGET(data));
}

void on_overvoltage_toggled(GtkToggleButton *toggle_button, gpointer data) {
    GtkWidget *scale_tolerance = GTK_WIDGET(g_object_get_data(G_OBJECT(toggle_button), "scale_tolerance"));
    gboolean is_active = gtk_toggle_button_get_active(toggle_button);
    gtk_widget_set_sensitive(scale_tolerance, is_active);
}

void on_edit_overvoltage(GtkWidget *button, gpointer data) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Configure Overvoltage Protection", GTK_WINDOW(data), GTK_DIALOG_MODAL, NULL);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
    gtk_container_add(GTK_CONTAINER(content_area), vbox);

    int tolerance = get_overvoltage_tolerance(i2c_fd);
    GtkWidget *checkbox = gtk_check_button_new_with_label("Enable Overvoltage Protection");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbox), tolerance > 0);
    gtk_box_pack_start(GTK_BOX(vbox), checkbox, FALSE, FALSE, 5);

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *label_1 = gtk_label_new("Overvoltage tolerance:");
    gtk_box_pack_start(GTK_BOX(hbox), label_1, FALSE, FALSE, 5);

    GtkWidget *scale_tolerance = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0.01, 1, 0.01);
    gtk_range_set_value(GTK_RANGE(scale_tolerance), (float)tolerance / 100.0f);
    gtk_scale_set_draw_value(GTK_SCALE(scale_tolerance), TRUE);
    gtk_widget_set_size_request(scale_tolerance, 500, -1);
    gtk_box_pack_start(GTK_BOX(hbox), scale_tolerance, TRUE, TRUE, 0);

    GtkWidget *label_2 = gtk_label_new("V");
    gtk_box_pack_start(GTK_BOX(hbox), label_2, FALSE, FALSE, 5);

    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);

    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *btn_reset = gtk_button_new_with_label("Reset");
    GtkWidget *btn_change = gtk_button_new_with_label("Change");
    gtk_box_pack_end(GTK_BOX(hbox), btn_change, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(hbox), btn_reset, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
    
    g_object_set_data(G_OBJECT(checkbox), "scale_tolerance", scale_tolerance);

    g_signal_connect(checkbox, "toggled", G_CALLBACK(on_overvoltage_toggled), NULL);
    
    on_overvoltage_toggled(GTK_TOGGLE_BUTTON(checkbox), NULL);
    
    g_object_set_data(G_OBJECT(dialog), "checkbox", checkbox);
    
    g_signal_connect(btn_reset, "clicked", G_CALLBACK(on_overvoltage_reset), dialog);
    
    g_signal_connect(btn_change, "clicked", G_CALLBACK(on_overvoltage_change), dialog);

    gtk_widget_show_all(dialog);

    g_signal_connect_swapped(dialog, "response", G_CALLBACK(gtk_widget_destroy), dialog);
}

void on_undervoltage_reset(GtkWidget *widget, gpointer data) {
    if (show_confirmation_dialog(GTK_WIDGET(data), FACTORY_RESET)) {
        GtkWidget *checkbox = GTK_WIDGET(g_object_get_data(G_OBJECT(data), "checkbox"));
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbox), true);
        GtkWidget *scale_tolerance = GTK_WIDGET(g_object_get_data(G_OBJECT(checkbox), "scale_tolerance"));
        gtk_range_set_value(GTK_RANGE(scale_tolerance), 0.5f);
    }
}

void on_undervoltage_change(GtkWidget *widget, gpointer data) {
    GtkWidget *checkbox = GTK_WIDGET(g_object_get_data(G_OBJECT(data), "checkbox"));
    GtkWidget *scale_tolerance = GTK_WIDGET(g_object_get_data(G_OBJECT(checkbox), "scale_tolerance"));
    int tolerance = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbox)) ? (int)(100 * gtk_range_get_value(GTK_RANGE(scale_tolerance))) : 0;
    set_undervoltage_tolerance(i2c_fd, tolerance);
    update_button_labels(UNDERVOLTAGE);
    gtk_widget_destroy(GTK_WIDGET(data));
}

void on_undervoltage_toggled(GtkToggleButton *toggle_button, gpointer data) {
    GtkWidget *scale_tolerance = GTK_WIDGET(g_object_get_data(G_OBJECT(toggle_button), "scale_tolerance"));
    gboolean is_active = gtk_toggle_button_get_active(toggle_button);
    gtk_widget_set_sensitive(scale_tolerance, is_active);
}

void on_edit_undervoltage(GtkWidget *button, gpointer data) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Configure Undervoltage Protection", GTK_WINDOW(data), GTK_DIALOG_MODAL, NULL);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
    gtk_container_add(GTK_CONTAINER(content_area), vbox);

    int tolerance = get_undervoltage_tolerance(i2c_fd);
    GtkWidget *checkbox = gtk_check_button_new_with_label("Enable Undervoltage Protection");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbox), tolerance > 0);
    gtk_box_pack_start(GTK_BOX(vbox), checkbox, FALSE, FALSE, 5);

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *label_1 = gtk_label_new("Undervoltage tolerance:");
    gtk_box_pack_start(GTK_BOX(hbox), label_1, FALSE, FALSE, 5);

    GtkWidget *scale_tolerance = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0.01, 1, 0.01);
    gtk_range_set_value(GTK_RANGE(scale_tolerance), (float)tolerance / 100.0f);
    gtk_scale_set_draw_value(GTK_SCALE(scale_tolerance), TRUE);
    gtk_widget_set_size_request(scale_tolerance, 500, -1);
    gtk_box_pack_start(GTK_BOX(hbox), scale_tolerance, TRUE, TRUE, 0);

    GtkWidget *label_2 = gtk_label_new("V");
    gtk_box_pack_start(GTK_BOX(hbox), label_2, FALSE, FALSE, 5);

    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);

    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *btn_reset = gtk_button_new_with_label("Reset");
    GtkWidget *btn_change = gtk_button_new_with_label("Change");
    gtk_box_pack_end(GTK_BOX(hbox), btn_change, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(hbox), btn_reset, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
    
    g_object_set_data(G_OBJECT(checkbox), "scale_tolerance", scale_tolerance);

    g_signal_connect(checkbox, "toggled", G_CALLBACK(on_undervoltage_toggled), NULL);
    
    on_undervoltage_toggled(GTK_TOGGLE_BUTTON(checkbox), NULL);
    
    g_object_set_data(G_OBJECT(dialog), "checkbox", checkbox);
    
    g_signal_connect(btn_reset, "clicked", G_CALLBACK(on_undervoltage_reset), dialog);
    
    g_signal_connect(btn_change, "clicked", G_CALLBACK(on_undervoltage_change), dialog);

    gtk_widget_show_all(dialog);

    g_signal_connect_swapped(dialog, "response", G_CALLBACK(gtk_widget_destroy), dialog);
}

void on_vin_calibration_reset(GtkWidget *widget, gpointer data) {
    if (show_confirmation_dialog(GTK_WIDGET(data), FACTORY_RESET)) {
        GtkWidget *checkbox = GTK_WIDGET(g_object_get_data(G_OBJECT(data), "checkbox"));
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbox), true);
        GtkWidget *scale_calibration = GTK_WIDGET(g_object_get_data(G_OBJECT(checkbox), "scale_calibration"));
        gtk_range_set_value(GTK_RANGE(scale_calibration), -0.3f);
    }
}

void on_vin_calibration_change(GtkWidget *widget, gpointer data) {
    GtkWidget *checkbox = GTK_WIDGET(g_object_get_data(G_OBJECT(data), "checkbox"));
    GtkWidget *scale_calibration = GTK_WIDGET(g_object_get_data(G_OBJECT(checkbox), "scale_calibration"));
    int calibration = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbox)) ? (int)(100 * gtk_range_get_value(GTK_RANGE(scale_calibration))) : 0;
    set_vin_calibration(i2c_fd, calibration);
    update_button_labels(VIN_CALIBRATION);
    gtk_widget_destroy(GTK_WIDGET(data));
}

void on_vin_calibration_toggled(GtkToggleButton *toggle_button, gpointer data) {
    GtkWidget *scale_calibration = GTK_WIDGET(g_object_get_data(G_OBJECT(toggle_button), "scale_calibration"));
    gboolean is_active = gtk_toggle_button_get_active(toggle_button);
    gtk_widget_set_sensitive(scale_calibration, is_active);
    if (!is_active) {
      gtk_range_set_value(GTK_RANGE(scale_calibration), 0.0f);
    }
}

void on_edit_vin_calibration(GtkWidget *button, gpointer data) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Configure Vin Calibration", GTK_WINDOW(data), GTK_DIALOG_MODAL, NULL);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
    gtk_container_add(GTK_CONTAINER(content_area), vbox);

    int calibration = get_signed_char(get_vin_calibration(i2c_fd));
    GtkWidget *checkbox = gtk_check_button_new_with_label("Enable Input Voltage Calibration");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbox), calibration != 0);
    gtk_box_pack_start(GTK_BOX(vbox), checkbox, FALSE, FALSE, 5);

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *label_1 = gtk_label_new("Input voltage calibration:");
    gtk_box_pack_start(GTK_BOX(hbox), label_1, FALSE, FALSE, 5);

    GtkWidget *scale_calibration = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, -1.28, 1.27, 0.01);
    gtk_range_set_value(GTK_RANGE(scale_calibration), (float)calibration / 100.0f);
    gtk_scale_set_draw_value(GTK_SCALE(scale_calibration), TRUE);
    gtk_widget_set_size_request(scale_calibration, 500, -1);
    gtk_box_pack_start(GTK_BOX(hbox), scale_calibration, TRUE, TRUE, 0);

    GtkWidget *label_2 = gtk_label_new("V");
    gtk_box_pack_start(GTK_BOX(hbox), label_2, FALSE, FALSE, 5);

    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);
    
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *remarks_label = gtk_label_new("Better to disable overvoltage and undervoltage protection before changing this value");
    set_label_attributes(GTK_LABEL(remarks_label));
    gtk_box_pack_start(GTK_BOX(hbox), remarks_label, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);

    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *btn_reset = gtk_button_new_with_label("Reset");
    GtkWidget *btn_change = gtk_button_new_with_label("Change");
    gtk_box_pack_end(GTK_BOX(hbox), btn_change, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(hbox), btn_reset, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
    
    g_object_set_data(G_OBJECT(checkbox), "scale_calibration", scale_calibration);

    g_signal_connect(checkbox, "toggled", G_CALLBACK(on_vin_calibration_toggled), NULL);
    
    on_vin_calibration_toggled(GTK_TOGGLE_BUTTON(checkbox), NULL);
    
    g_object_set_data(G_OBJECT(dialog), "checkbox", checkbox);
    
    g_signal_connect(btn_reset, "clicked", G_CALLBACK(on_vin_calibration_reset), dialog);
    
    g_signal_connect(btn_change, "clicked", G_CALLBACK(on_vin_calibration_change), dialog);

    gtk_widget_show_all(dialog);

    g_signal_connect_swapped(dialog, "response", G_CALLBACK(gtk_widget_destroy), dialog);
}

void on_vout_calibration_reset(GtkWidget *widget, gpointer data) {
    if (show_confirmation_dialog(GTK_WIDGET(data), FACTORY_RESET)) {
        GtkWidget *checkbox = GTK_WIDGET(g_object_get_data(G_OBJECT(data), "checkbox"));
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbox), true);
        GtkWidget *scale_calibration = GTK_WIDGET(g_object_get_data(G_OBJECT(checkbox), "scale_calibration"));
        gtk_range_set_value(GTK_RANGE(scale_calibration), -0.3f);
    }
}

void on_vout_calibration_change(GtkWidget *widget, gpointer data) {
    GtkWidget *checkbox = GTK_WIDGET(g_object_get_data(G_OBJECT(data), "checkbox"));
    GtkWidget *scale_calibration = GTK_WIDGET(g_object_get_data(G_OBJECT(checkbox), "scale_calibration"));
    int calibration = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbox)) ? (int)(100 * gtk_range_get_value(GTK_RANGE(scale_calibration))) : 0;
    set_vout_calibration(i2c_fd, calibration);
    update_button_labels(VOUT_CALIBRATION);
    gtk_widget_destroy(GTK_WIDGET(data));
}

void on_vout_calibration_toggled(GtkToggleButton *toggle_button, gpointer data) {
    GtkWidget *scale_calibration = GTK_WIDGET(g_object_get_data(G_OBJECT(toggle_button), "scale_calibration"));
    gboolean is_active = gtk_toggle_button_get_active(toggle_button);
    gtk_widget_set_sensitive(scale_calibration, is_active);
    if (!is_active) {
      gtk_range_set_value(GTK_RANGE(scale_calibration), 0.0f);
    }
}

void on_edit_vout_calibration(GtkWidget *button, gpointer data) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Configure Vout Calibration", GTK_WINDOW(data), GTK_DIALOG_MODAL, NULL);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
    gtk_container_add(GTK_CONTAINER(content_area), vbox);

    int calibration = get_signed_char(get_vout_calibration(i2c_fd));
    GtkWidget *checkbox = gtk_check_button_new_with_label("Enable Output Voltage Calibration");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbox), calibration != 0);
    gtk_box_pack_start(GTK_BOX(vbox), checkbox, FALSE, FALSE, 5);

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *label_1 = gtk_label_new("Output voltage calibration:");
    gtk_box_pack_start(GTK_BOX(hbox), label_1, FALSE, FALSE, 5);

    GtkWidget *scale_calibration = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, -1.28, 1.27, 0.01);
    gtk_range_set_value(GTK_RANGE(scale_calibration), (float)calibration / 100.0f);
    gtk_scale_set_draw_value(GTK_SCALE(scale_calibration), TRUE);
    gtk_widget_set_size_request(scale_calibration, 500, -1);
    gtk_box_pack_start(GTK_BOX(hbox), scale_calibration, TRUE, TRUE, 0);

    GtkWidget *label_2 = gtk_label_new("V");
    gtk_box_pack_start(GTK_BOX(hbox), label_2, FALSE, FALSE, 5);

    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);

    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *btn_reset = gtk_button_new_with_label("Reset");
    GtkWidget *btn_change = gtk_button_new_with_label("Change");
    gtk_box_pack_end(GTK_BOX(hbox), btn_change, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(hbox), btn_reset, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
    
    g_object_set_data(G_OBJECT(checkbox), "scale_calibration", scale_calibration);

    g_signal_connect(checkbox, "toggled", G_CALLBACK(on_vout_calibration_toggled), NULL);
    
    on_vout_calibration_toggled(GTK_TOGGLE_BUTTON(checkbox), NULL);
    
    g_object_set_data(G_OBJECT(dialog), "checkbox", checkbox);
    
    g_signal_connect(btn_reset, "clicked", G_CALLBACK(on_vout_calibration_reset), dialog);
    
    g_signal_connect(btn_change, "clicked", G_CALLBACK(on_vout_calibration_change), dialog);

    gtk_widget_show_all(dialog);

    g_signal_connect_swapped(dialog, "response", G_CALLBACK(gtk_widget_destroy), dialog);
}

void on_iout_calibration_reset(GtkWidget *widget, gpointer data) {
    if (show_confirmation_dialog(GTK_WIDGET(data), FACTORY_RESET)) {
        GtkWidget *checkbox = GTK_WIDGET(g_object_get_data(G_OBJECT(data), "checkbox"));
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbox), true);
        GtkWidget *scale_calibration = GTK_WIDGET(g_object_get_data(G_OBJECT(checkbox), "scale_calibration"));
        gtk_range_set_value(GTK_RANGE(scale_calibration), -0.2f);
    }
}

void on_iout_calibration_change(GtkWidget *widget, gpointer data) {
    GtkWidget *checkbox = GTK_WIDGET(g_object_get_data(G_OBJECT(data), "checkbox"));
    GtkWidget *scale_calibration = GTK_WIDGET(g_object_get_data(G_OBJECT(checkbox), "scale_calibration"));
    int calibration = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbox)) ? (int)(100 * gtk_range_get_value(GTK_RANGE(scale_calibration))) : 0;
    set_iout_calibration(i2c_fd, calibration);
    update_button_labels(IOUT_CALIBRATION);
    gtk_widget_destroy(GTK_WIDGET(data));
}

void on_iout_calibration_toggled(GtkToggleButton *toggle_button, gpointer data) {
    GtkWidget *scale_calibration = GTK_WIDGET(g_object_get_data(G_OBJECT(toggle_button), "scale_calibration"));
    gboolean is_active = gtk_toggle_button_get_active(toggle_button);
    gtk_widget_set_sensitive(scale_calibration, is_active);
    if (!is_active) {
      gtk_range_set_value(GTK_RANGE(scale_calibration), 0.0f);
    }
}

void on_edit_iout_calibration(GtkWidget *button, gpointer data) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Configure Iout Calibration", GTK_WINDOW(data), GTK_DIALOG_MODAL, NULL);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
    gtk_container_add(GTK_CONTAINER(content_area), vbox);

    int calibration = get_signed_char(get_iout_calibration(i2c_fd));
    GtkWidget *checkbox = gtk_check_button_new_with_label("Enable Output Current Calibration");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbox), calibration != 0);
    gtk_box_pack_start(GTK_BOX(vbox), checkbox, FALSE, FALSE, 5);

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *label_1 = gtk_label_new("Output current calibration:");
    gtk_box_pack_start(GTK_BOX(hbox), label_1, FALSE, FALSE, 5);

    GtkWidget *scale_calibration = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, -1.28, 1.27, 0.01);
    gtk_range_set_value(GTK_RANGE(scale_calibration), (float)calibration / 100.0f);
    gtk_scale_set_draw_value(GTK_SCALE(scale_calibration), TRUE);
    gtk_widget_set_size_request(scale_calibration, 500, -1);
    gtk_box_pack_start(GTK_BOX(hbox), scale_calibration, TRUE, TRUE, 0);

    GtkWidget *label_2 = gtk_label_new("A");
    gtk_box_pack_start(GTK_BOX(hbox), label_2, FALSE, FALSE, 5);

    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);

    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *btn_reset = gtk_button_new_with_label("Reset");
    GtkWidget *btn_change = gtk_button_new_with_label("Change");
    gtk_box_pack_end(GTK_BOX(hbox), btn_change, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(hbox), btn_reset, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
    
    g_object_set_data(G_OBJECT(checkbox), "scale_calibration", scale_calibration);

    g_signal_connect(checkbox, "toggled", G_CALLBACK(on_iout_calibration_toggled), NULL);
    
    on_iout_calibration_toggled(GTK_TOGGLE_BUTTON(checkbox), NULL);
    
    g_object_set_data(G_OBJECT(dialog), "checkbox", checkbox);
    
    g_signal_connect(btn_reset, "clicked", G_CALLBACK(on_iout_calibration_reset), dialog);
    
    g_signal_connect(btn_change, "clicked", G_CALLBACK(on_iout_calibration_change), dialog);

    gtk_widget_show_all(dialog);

    g_signal_connect_swapped(dialog, "response", G_CALLBACK(gtk_widget_destroy), dialog);
}

void on_factory_reset_all(GtkWidget *button, gpointer data) {
  if (show_confirmation_dialog(GTK_WIDGET(data), FACTORY_RESET)) {
      set_auto_on_delay(i2c_fd, 0);
      set_power_cut_delay(i2c_fd, 1);
      set_watchdog_threshold(i2c_fd, 6);
      set_overvoltage_tolerance(i2c_fd, 0);
      set_undervoltage_tolerance(i2c_fd, 0);
      set_temp_cond1(i2c_fd, 65);
      set_fan_speed1(i2c_fd, 100);
      set_temp_cond2(i2c_fd, 60);
      set_fan_speed2(i2c_fd, 80);
      set_temp_cond3(i2c_fd, 55);
      set_fan_speed3(i2c_fd, 65);
      set_vin_calibration(i2c_fd, 0xE2);
      set_vout_calibration(i2c_fd, 0xE2);
      set_iout_calibration(i2c_fd, 0xEC);
      set_overvoltage_tolerance(i2c_fd, 50);
      set_undervoltage_tolerance(i2c_fd, 50);
      update_button_labels(-1);
  }
}

int main(int argc, char *argv[]) {
    
    if (geteuid() != 0) {
        restart_with_sudo(argc, argv);
        return 1;
    }
    
    i2c_fd = init_i2c_device(I2C_BUS, I2C_ADDR);
    if (i2c_fd < 0) {
      return 1;
    }
    
    GtkWidget *window;
    GtkWidget *vbox;

    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    sprintf(info, "Vivid Unit Extender Dashboard V%.2f", VUE_VERSION);
    gtk_window_set_title(GTK_WINDOW(window), info);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
    
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    GtkWidget *spacer_top = gtk_label_new("");
    gtk_widget_set_size_request(spacer_top, -1, 5);
    gtk_box_pack_start(GTK_BOX(vbox), spacer_top, FALSE, FALSE, 0);

    label_voltage_current = gtk_label_new("");
    label_watchdog = gtk_label_new("");
    label_cpu_gpu_fan = gtk_label_new("");
    update_text();

    set_label_attributes(GTK_LABEL(label_voltage_current));
    set_label_attributes(GTK_LABEL(label_watchdog));
    set_label_attributes(GTK_LABEL(label_cpu_gpu_fan));

    gtk_box_pack_start(GTK_BOX(vbox), label_voltage_current, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), label_watchdog, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), label_cpu_gpu_fan, FALSE, FALSE, 0);

    GtkWidget *spacer = gtk_label_new("");
    gtk_widget_set_size_request(spacer, -1, 6);
    gtk_box_pack_start(GTK_BOX(vbox), spacer, FALSE, FALSE, 0);

    const char *button_label_texts[12] = {
        "Reset all to factory settings",
        "CPU or GPU temperature ≥ 65°C, fan speed ➔ 100%",
        "Auto-ON when powered is disabled",
        "CPU or GPU temperature ≥ 60°C, fan speed ➔ 80%",
        "Power-cut after shutdown with 1 second delay",
        "CPU or GPU temperature ≥ 55°C, fan speed ➔ 65%",
        "Watchdog: allow 6 missed heartbeats",
        "Input voltage calibration: -0.30V",
        "Overvoltage tolerance: 0.50V",
        "Output voltage calibration: -0.30V",
        "Undervoltage tolerance: 0.50V",
        "Output current calibration: -0.20A"
    };

    GtkWidget *grid = gtk_grid_new();
    gtk_box_pack_start(GTK_BOX(vbox), grid, TRUE, TRUE, 0);

    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 5);

    for (int i = 0; i < 12; i++) {
        GtkWidget *button = gtk_button_new();
        GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
        GtkWidget *image = gtk_image_new_from_file(button_icons[i]);
        gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, FALSE, 0);
        button_labels[i] = gtk_label_new(button_label_texts[i]);
        gtk_box_pack_start(GTK_BOX(hbox), button_labels[i], TRUE, TRUE, 0);
        gtk_container_add(GTK_CONTAINER(button), hbox);
        gtk_widget_set_size_request(button, 500, 75);

        gtk_grid_attach(GTK_GRID(grid), button, i % 2, (i / 2), 1, 1);
        
        if (i == TEMPERATURE_CONDITION_1 || i == TEMPERATURE_CONDITION_2 || i == TEMPERATURE_CONDITION_3) {
            sprintf(info, "Temperature Condition %d", 1 + (i - 1) / 2);
            gtk_widget_set_name(button, info);
        }
        
        g_signal_connect(button, "clicked", G_CALLBACK(on_edit_funcs[i]), window);
    }

    update_button_labels(-1);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    g_timeout_add_seconds(1, (GSourceFunc)on_timeout_update_text, NULL);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}
