#include <Elementary.h>
#include <libintl.h>
#include <unistd.h>
#include <Ecore.h>          
#include "logic.h"
#include <signal.h>

#define _(String) gettext(String)

static Evas_Object *start_spinner;
static Evas_Object *end_spinner;
static Evas_Object *vlabel;
static Evas_Object *ampm_start_label;
static Evas_Object *ampm_end_label;
static Evas_Object *radio_24h;
static Evas_Object *radio_12h;
static int use_12h = 0;

static pid_t read_daemon_pid(void);

static int
to_24h(int h12, int is_pm)
{
    if (h12 == 12) return is_pm ? 12 : 0;
    return is_pm ? h12 + 12 : h12;
}

static void
to_12h(int h24, int *h12_out, int *is_pm_out)
{
    *is_pm_out = (h24 >= 12) ? 1 : 0;
    int h = h24 % 12;
    *h12_out = (h == 0) ? 12 : h;
}

static Eina_Bool
kill_force_cb(void *data EINA_UNUSED)
{
    pid_t pid = read_daemon_pid();
    if (pid > 0)
        kill(pid, SIGKILL);

    return ECORE_CALLBACK_CANCEL;
}

static void
kill_daemon(void)
{
    pid_t pid = read_daemon_pid();

    if (pid > 0)
        kill(pid, SIGTERM);

    ecore_timer_add(0.15, kill_force_cb, NULL);

    printf("Daemon termination attempted.\n");
}

static pid_t
read_daemon_pid(void)
{
    FILE *f;
    pid_t pid = -1;
    const char *home = getenv("HOME");

    if (!home)
        return -1;

    char path[256];
    snprintf(path, sizeof(path), "%s/.config/nl-ease.pid", home);

    f = fopen(path, "r");
    if (!f)
        return -1;

    fscanf(f, "%d", &pid);
    fclose(f);

    return pid;
}

static int
daemon_running(void)
{
    pid_t pid = read_daemon_pid();

    if (pid <= 0)
        return 0;
    // kill(pid,0) doesn't send signals
    return (kill(pid, 0) == 0);
}

static void
on_toggle_changed(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
    int enabled = elm_check_state_get(obj);
    
    logic_set_enabled(enabled);

    if (enabled == 0) {
        printf("Enabled -> OFF: killing daemon...\n");
        kill_daemon();
    } else {
        printf("Enabled -> ON\n");
    }
}

static void
on_slider_changed(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
    int visual_value = (int)elm_slider_value_get(obj);
    char buf[20];
    sprintf(buf, "%d", visual_value);
    elm_object_text_set(vlabel, buf);
    logic_set_temperature(visual_value);
}

static void
on_schedule_changed(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
    static int prev_start = -1;
    static int prev_end   = -1;

    int start_raw = (int)elm_spinner_value_get(start_spinner);
    int end_raw   = (int)elm_spinner_value_get(end_spinner);

    int start24, end24;

    if (use_12h) {
        // Find the wrap and update AM/PM before reading the label 
        if (prev_start >= 0) {
            if ((prev_start == 11 && start_raw == 12) ||
                (prev_start == 12 && start_raw == 11)) {
                const char *t = elm_object_text_get(ampm_start_label);
                elm_object_text_set(ampm_start_label, (t && t[0] == 'P') ? "AM" : "PM");
            }
        }
        if (prev_end >= 0) {
            if ((prev_end == 11 && end_raw == 12) ||
                (prev_end == 12 && end_raw == 11)) {
                const char *t = elm_object_text_get(ampm_end_label);
                elm_object_text_set(ampm_end_label, (t && t[0] == 'P') ? "AM" : "PM");
            }
        }

        prev_start = start_raw;
        prev_end   = end_raw;

        // read the updated label
        const char *sl = elm_object_text_get(ampm_start_label);
        const char *el = elm_object_text_get(ampm_end_label);
        int start_pm = (sl && sl[0] == 'P') ? 1 : 0;
        int end_pm   = (el && el[0] == 'P') ? 1 : 0;

        start24 = to_24h(start_raw, start_pm);
        end24   = to_24h(end_raw,   end_pm);
    } else {
        prev_start = -1;
        prev_end   = -1;
        start24 = start_raw;
        end24   = end_raw;
    }

    logic_set_schedule(start24, end24);
}

static void
on_format_changed(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
    use_12h = (elm_radio_value_get(obj) == 1);
    logic_get_state()->use_12h = use_12h;
    logic_save();

    // read 24h values from logic
    const AppState *s = logic_get_state();

    if (use_12h) {
        // start conversion to 12h
        int h12, is_pm;
        to_12h(s->start_hour, &h12, &is_pm);
        elm_spinner_min_max_set(start_spinner, 1, 12);
        elm_spinner_wrap_set(start_spinner, EINA_TRUE);
        elm_spinner_value_set(start_spinner, h12);
        elm_object_text_set(ampm_start_label, is_pm ? "PM" : "AM");
        evas_object_show(ampm_start_label);

        to_12h(s->end_hour, &h12, &is_pm);
        elm_spinner_min_max_set(end_spinner, 1, 12);
        elm_spinner_wrap_set(end_spinner, EINA_TRUE);
        elm_spinner_value_set(end_spinner, h12);
        elm_object_text_set(ampm_end_label, is_pm ? "PM" : "AM");
        evas_object_show(ampm_end_label);
        
    } else {
        // back to 24h
        elm_spinner_min_max_set(start_spinner, 0, 23);
        elm_spinner_value_set(start_spinner, s->start_hour);
        elm_object_text_set(ampm_start_label, "");
        evas_object_hide(ampm_start_label);

        elm_spinner_min_max_set(end_spinner, 0, 23);
        elm_spinner_value_set(end_spinner, s->end_hour);
        elm_object_text_set(ampm_end_label, "");
        evas_object_hide(ampm_end_label);
    }
}

static void
on_save_clicked(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
    logic_save();
}

static void
on_launch_daemon_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
    // first time config save
    logic_save();

    if (!daemon_running())
    {
        ecore_exe_run("nl-ease --daemon", NULL);
        printf("Daemon started.\n");
    }
    else
    {
        printf("Daemon already running.\n");
    }

    elm_exit();
}

static void
on_win_delete(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
    const AppState *s = logic_get_state();
    
    if (!s->enabled) {
        printf("Closing GUI with Enabled=OFF -> killing daemon\n");
        kill_daemon();
    }
    
    elm_exit();
}

void
ui_init(void)
{
    Evas_Object *win = elm_win_util_standard_add("nl-ease", "nl-ease");
    elm_win_autodel_set(win, EINA_TRUE);

    evas_object_resize(win, 300, 280);
    evas_object_size_hint_min_set(win, 300, 280);
    evas_object_size_hint_max_set(win, 300, 280);
    
    elm_win_size_base_set(win, 300, 280);
    elm_win_size_step_set(win, 0, 0);

    Evas_Object *box = elm_box_add(win);
    evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_win_resize_object_add(win, box);
    evas_object_show(box);

    // Toggle Enabled
    Evas_Object *toggle = elm_check_add(win);
    elm_object_text_set(toggle, _("Enabled"));
    evas_object_smart_callback_add(toggle, "changed", on_toggle_changed, NULL);
    elm_box_pack_end(box, toggle);
    evas_object_show(toggle);

    // Temperature Slider + Value
    Evas_Object *hbox = elm_box_add(win);
    elm_box_horizontal_set(hbox, EINA_TRUE);
    elm_box_pack_end(box, hbox);
    evas_object_show(hbox);
    
    Evas_Object *slider = elm_slider_add(win);
    elm_slider_min_max_set(slider, 2500, 6500);
    elm_slider_value_set(slider, 4500);
    elm_object_text_set(slider, _("Temperature"));
    evas_object_smart_callback_add(slider, "changed", on_slider_changed, NULL);
    elm_slider_indicator_show_set(slider, EINA_TRUE);
    evas_object_size_hint_min_set(slider, 220, -1);
    elm_box_pack_end(hbox, slider);
    evas_object_show(slider);
    
    Evas_Object *label = elm_label_add(win);
    elm_object_text_set(label, "4500");
    vlabel = label;
    evas_object_show(label);
    elm_box_pack_end(hbox, label);

    // Start / End time 
    Evas_Object *fmt_label = elm_label_add(win);
    elm_object_text_set(fmt_label, _("Time format:"));
    elm_box_pack_end(box, fmt_label);
    evas_object_show(fmt_label);

    Evas_Object *fmt_box = elm_box_add(win);
    elm_box_horizontal_set(fmt_box, EINA_TRUE);
    elm_box_padding_set(fmt_box, 10, 0);
    elm_box_pack_end(box, fmt_box);
    evas_object_show(fmt_box);

    radio_24h = elm_radio_add(win);
    elm_object_text_set(radio_24h, _("24h"));
    elm_radio_state_value_set(radio_24h, 0);
    elm_radio_group_add(radio_24h, radio_24h);
    elm_radio_value_set(radio_24h, 0);
    evas_object_smart_callback_add(radio_24h, "changed", on_format_changed, NULL);
    elm_box_pack_end(fmt_box, radio_24h);
    evas_object_show(radio_24h);

    radio_12h = elm_radio_add(win);
    elm_object_text_set(radio_12h, _("12h"));
    elm_radio_state_value_set(radio_12h, 1);
    elm_radio_group_add(radio_12h, radio_24h);
    evas_object_smart_callback_add(radio_12h, "changed", on_format_changed, NULL);
    elm_box_pack_end(fmt_box, radio_12h);
    evas_object_show(radio_12h);

    // Start time
    Evas_Object *start_label = elm_label_add(win);
    elm_object_text_set(start_label, _("Start time:"));
    elm_box_pack_end(box, start_label);
    evas_object_show(start_label);

    Evas_Object *start_hbox = elm_box_add(win);
    elm_box_horizontal_set(start_hbox, EINA_TRUE);
    elm_box_padding_set(start_hbox, 6, 0);
    elm_box_pack_end(box, start_hbox);
    evas_object_show(start_hbox);

    start_spinner = elm_spinner_add(win);
    elm_spinner_min_max_set(start_spinner, 0, 23);
    elm_spinner_wrap_set(start_spinner, EINA_TRUE);
    elm_spinner_value_set(start_spinner, 22);
    evas_object_smart_callback_add(start_spinner, "changed", on_schedule_changed, NULL);
    elm_box_pack_end(start_hbox, start_spinner);
    evas_object_show(start_spinner);

    ampm_start_label = elm_label_add(win);
    elm_object_text_set(ampm_start_label, "");
    elm_box_pack_end(start_hbox, ampm_start_label);
    evas_object_show(ampm_start_label);

    // End time
    Evas_Object *end_label = elm_label_add(win);
    elm_object_text_set(end_label, _("End time:"));
    elm_box_pack_end(box, end_label);
    evas_object_show(end_label);

    Evas_Object *end_hbox = elm_box_add(win);
    elm_box_horizontal_set(end_hbox, EINA_TRUE);
    elm_box_padding_set(end_hbox, 6, 0);
    elm_box_pack_end(box, end_hbox);
    evas_object_show(end_hbox);

    end_spinner = elm_spinner_add(win);
    elm_spinner_min_max_set(end_spinner, 0, 23);
    elm_spinner_wrap_set(end_spinner, EINA_TRUE);
    elm_spinner_value_set(end_spinner, 6);
    evas_object_smart_callback_add(end_spinner, "changed", on_schedule_changed, NULL);
    elm_box_pack_end(end_hbox, end_spinner);
    evas_object_show(end_spinner);

    ampm_end_label = elm_label_add(win);
    elm_object_text_set(ampm_end_label, "");
    elm_box_pack_end(end_hbox, ampm_end_label);
    evas_object_show(ampm_end_label);

    // Buttons
    Evas_Object *btn_box = elm_box_add(win);
    elm_box_horizontal_set(btn_box, EINA_FALSE);
    elm_box_homogeneous_set(btn_box, EINA_TRUE);
    elm_box_padding_set(btn_box, 0, 8);
    evas_object_size_hint_weight_set(btn_box, EVAS_HINT_EXPAND, 0);
    elm_box_pack_end(box, btn_box);
    evas_object_show(btn_box);

    Evas_Object *btn_save = elm_button_add(win);
    elm_object_text_set(btn_save, _("Save Configuration"));
    evas_object_smart_callback_add(btn_save, "clicked", on_save_clicked, NULL);
    elm_box_pack_end(btn_box, btn_save);
    evas_object_show(btn_save);

    Evas_Object *btn_daemon = elm_button_add(win);
    elm_object_text_set(btn_daemon, _("Close and Launch Daemon"));
    evas_object_smart_callback_add(btn_daemon, "clicked", on_launch_daemon_clicked, win);
    elm_box_pack_end(btn_box, btn_daemon);
    evas_object_show(btn_daemon);

    // Sync UI
    AppState *s = logic_get_state();
    elm_check_state_set(toggle, s->enabled);
    
    elm_slider_value_set(slider, s->temperature);
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", s->temperature);
    elm_object_text_set(vlabel, buf);
    
    elm_spinner_value_set(start_spinner, s->start_hour);
    elm_spinner_value_set(end_spinner, s->end_hour);
    
    // back to 12h if selected and saved
    if (s->use_12h) {
        use_12h = 1;
        elm_radio_value_set(radio_24h, 1);

        int h12, is_pm;
        to_12h(s->start_hour, &h12, &is_pm);
        elm_spinner_min_max_set(start_spinner, 1, 12);
        elm_spinner_wrap_set(start_spinner, EINA_TRUE);
        elm_spinner_value_set(start_spinner, h12);
        elm_object_text_set(ampm_start_label, is_pm ? "PM" : "AM");
        evas_object_show(ampm_start_label);

        to_12h(s->end_hour, &h12, &is_pm);
        elm_spinner_min_max_set(end_spinner, 1, 12);
        elm_spinner_wrap_set(end_spinner, EINA_TRUE);
        elm_spinner_value_set(end_spinner, h12);
        elm_object_text_set(ampm_end_label, is_pm ? "PM" : "AM");
        evas_object_show(ampm_end_label);
    }

    evas_object_smart_callback_add(win, "delete,request", on_win_delete, NULL);

    evas_object_show(win);
}
