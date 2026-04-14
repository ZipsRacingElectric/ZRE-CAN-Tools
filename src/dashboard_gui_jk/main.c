// Dashboard GUI (JK Branch) ---------------------------------------------------------------------------------------------------
//
// Description:
//   Single-file GTK4 dashboard for Zips Racing DART display.
//   Layout: dark background with red→dark→green gradient, large center speed,
//   data logger top-left, BMS/VCU/AMK right panel, nav buttons bottom.

#include <gtk/gtk.h>
#include <stdbool.h>
#include "debug.h"
#include "error_codes.h"
#include "can_device/can_device.h"
#include "can_database/can_database.h"

// ============================================================
// Global CAN database
// ============================================================

canDatabase_t database;

typedef struct { float r, g, b; float percent; } BarData;

// ============================================================
// UI widget references (populated in activate())
// ============================================================

static struct {
    GtkWidget*  bg;              // Cairo drawing area (background layer)
    GtkWidget*  grid;            // Main layout grid
    GtkWidget*  dataLoggerPanel; // Top-left data logger box  (used by Cairo)
    GtkWidget*  velocityTitle;   // "Vehicle Velocity:" label (used by Cairo)
    GtkWidget*  rightPanel;      // BMS/VCU/AMK panel         (used by Cairo)
    GtkWidget*  buttonPanel;     // Bottom nav button box      (used by Cairo)
    GtkWidget*  bseBar;          // BSE pedal bar drawing area
    GtkWidget*  appsBar;         // APPS pedal bar drawing area
    GtkWidget*  stack;           // Root GtkStack (speed / endr / CAN pages)
    /* Endurance page widget refs (mirror of speed page for Cairo bg) */
    GtkWidget*  endrGrid;
    GtkWidget*  endrDataLoggerPanel;
    GtkWidget*  endrRightPanel;
    GtkWidget*  endrBseBar;
    GtkWidget*  endrAppsBar;
    GtkLabel*   packVoltage;
    GtkLabel*   powerAvg;
    GtkLabel*   energyDelivered;
    GtkLabel**  signalLabels;    // One value label per CAN signal (indexed by global index)
    size_t      signalLabelCount;
    GtkLabel*   cellVoltLabels[144]; // One value label per cell voltage (indexed 0-143)
    ssize_t     cellVoltIdx[144];    // Pre-cached signal indices for CELL_VOLTAGE_0..143
    // Pre-cached named signal indices — looked up once in activate(), never change
    ssize_t     idx_speed;
    ssize_t     idx_session;
    ssize_t     idx_vcu_fault;
    ssize_t     idx_bse;
    ssize_t     idx_apps;
    ssize_t     idx_mtr_temp;
    ssize_t     idx_inv_temp;
    ssize_t     idx_pack_voltage;
    ssize_t     idx_power_avg;
    ssize_t     idx_energy_delivered;
    ssize_t     idx_cell_temp[60];   // SENSE_LINE_*_TEMPERATURE signals
    GtkLabel*   speedVal;        // Large center speed number
    GtkLabel*   loggerTitle;     // "LOGGER OFF"
    GtkLabel*   loggerStat;      // "Session\nNo. 273"
    GtkLabel*   bmsMax;
    GtkLabel*   bmsAvg;
    GtkLabel*   vcuFaults;
    GtkLabel*   mtrTemp;
    GtkLabel*   invTemp;
    // Duplicate right-panel labels on ENDR page (same data, separate widgets)
    GtkLabel*   endrBmsMax;
    GtkLabel*   endrBmsAvg;
    GtkLabel*   endrVcuFaults;
    GtkLabel*   endrMtrTemp;
    GtkLabel*   endrInvTemp;
    // Data logger labels on ENDR page (separate from speed page)
    GtkLabel*   endrLoggerTitle;
    GtkLabel*   endrLoggerStat;
} ui;

// ============================================================
// Pango label helpers
// ============================================================

static GtkLabel* make_label(const char* text, const char* font_desc, float r, float g, float b)
{
    GtkLabel* label = GTK_LABEL(gtk_label_new(text));

    PangoAttrList* attrs = pango_attr_list_new();

    if (font_desc) {
        PangoFontDescription* fd = pango_font_description_from_string(font_desc);
        pango_attr_list_insert(attrs, pango_attr_font_desc_new(fd));
        /* pango_attr_font_desc_new copies the descriptor, so free the original */
        pango_font_description_free(fd);
    }

    pango_attr_list_insert(attrs,
        pango_attr_foreground_new(
            (guint16)(r * 65535),
            (guint16)(g * 65535),
            (guint16)(b * 65535)));

    gtk_label_set_attributes(label, attrs);
    pango_attr_list_unref(attrs);

    return label;
    }

// ============================================================
// CAN update callbacks (30 fps via g_timeout_add)
// ============================================================

static void update_speed(void)
{
    float val = 0.0f;
    char text[8] = "--";
    if (canDatabaseGetFloat(&database, ui.idx_speed, &val) == CAN_DATABASE_VALID){
		val *= 0.00424999;
        if ( val < -0 ) val = -val;
        snprintf(text, sizeof(text), "%.0f", val);}
    gtk_label_set_text(ui.speedVal, text);
}

// Single pass for both logger title and session number (same signal)
static void update_logger(void)
{
    float val = 0.0f;
    if (canDatabaseGetFloat(&database, ui.idx_session, &val) != CAN_DATABASE_VALID)
        return;
    gtk_label_set_text(ui.loggerTitle, val > 0.5f ? "LOGGER ON" : "LOGGER OFF");
    gtk_label_set_text(ui.endrLoggerTitle, val > 0.5f ? "LOGGER ON" : "LOGGER OFF");
    char text[32];
    snprintf(text, sizeof(text), "Session\nNo. %.0f", val);
    gtk_label_set_text(ui.loggerStat, text);
    gtk_label_set_text(ui.endrLoggerStat, text);
}

static const char* BMS_CELL_TEMP_SIGNALS[] = {
    /* BMS_TEMPERATURES_0_12 */
    "SENSE_LINE_4_TEMPERATURE",
    "SENSE_LINE_8_TEMPERATURE",
    "SENSE_LINE_11_TEMPERATURE",
    "SENSE_LINE_7_TEMPERATURE",
    "SENSE_LINE_3_TEMPERATURE",
    /* BMS_TEMPERATURES_12_24 */
    "SENSE_LINE_13_TEMPERATURE",
    "SENSE_LINE_17_TEMPERATURE",
    "SENSE_LINE_21_TEMPERATURE",
    "SENSE_LINE_16_TEMPERATURE",
    "SENSE_LINE_20_TEMPERATURE",
    /* BMS_TEMPERATURES_24_36 */
    "SENSE_LINE_28_TEMPERATURE",
    "SENSE_LINE_32_TEMPERATURE",
    "SENSE_LINE_35_TEMPERATURE",
    "SENSE_LINE_31_TEMPERATURE",
    "SENSE_LINE_27_TEMPERATURE",
    /* BMS_TEMPERATURES_36_48 */
    "SENSE_LINE_37_TEMPERATURE",
    "SENSE_LINE_41_TEMPERATURE",
    "SENSE_LINE_45_TEMPERATURE",
    "SENSE_LINE_40_TEMPERATURE",
    "SENSE_LINE_44_TEMPERATURE",
    /* BMS_TEMPERATURES_48_60 */
    "SENSE_LINE_52_TEMPERATURE",
    "SENSE_LINE_56_TEMPERATURE",
    "SENSE_LINE_59_TEMPERATURE",
    "SENSE_LINE_55_TEMPERATURE",
    "SENSE_LINE_51_TEMPERATURE",
    /* BMS_TEMPERATURES_60_72 */
    "SENSE_LINE_61_TEMPERATURE",
    "SENSE_LINE_65_TEMPERATURE",
    "SENSE_LINE_69_TEMPERATURE",
    "SENSE_LINE_64_TEMPERATURE",
    "SENSE_LINE_68_TEMPERATURE",
    /* BMS_TEMPERATURES_72_84 */
    "SENSE_LINE_76_TEMPERATURE",
    "SENSE_LINE_80_TEMPERATURE",
    "SENSE_LINE_83_TEMPERATURE",
    "SENSE_LINE_79_TEMPERATURE",
    "SENSE_LINE_75_TEMPERATURE",
    /* BMS_TEMPERATURES_84_96 */
    "SENSE_LINE_85_TEMPERATURE",
    "SENSE_LINE_89_TEMPERATURE",
    "SENSE_LINE_93_TEMPERATURE",
    "SENSE_LINE_88_TEMPERATURE",
    "SENSE_LINE_92_TEMPERATURE",
    /* BMS_TEMPERATURES_96_108 */
    "SENSE_LINE_100_TEMPERATURE",
    "SENSE_LINE_104_TEMPERATURE",
    "SENSE_LINE_107_TEMPERATURE",
    "SENSE_LINE_103_TEMPERATURE",
    "SENSE_LINE_99_TEMPERATURE",
    /* BMS_TEMPERATURES_108_120 */
    "SENSE_LINE_109_TEMPERATURE",
    "SENSE_LINE_113_TEMPERATURE",
    "SENSE_LINE_117_TEMPERATURE",
    "SENSE_LINE_112_TEMPERATURE",
    "SENSE_LINE_116_TEMPERATURE",
    /* BMS_TEMPERATURES_120_132 */
    "SENSE_LINE_124_TEMPERATURE",
    "SENSE_LINE_128_TEMPERATURE",
    "SENSE_LINE_131_TEMPERATURE",
    "SENSE_LINE_127_TEMPERATURE",
    "SENSE_LINE_123_TEMPERATURE",
    /* BMS_TEMPERATURES_132_144 */
    "SENSE_LINE_133_TEMPERATURE",
    "SENSE_LINE_137_TEMPERATURE",
    "SENSE_LINE_141_TEMPERATURE",
    "SENSE_LINE_136_TEMPERATURE",
    "SENSE_LINE_140_TEMPERATURE",
};
static const int BMS_CELL_TEMP_COUNT =
    (int)(sizeof(BMS_CELL_TEMP_SIGNALS) / sizeof(BMS_CELL_TEMP_SIGNALS[0]));

// Single pass computes both max and avg temperature, updates both speed and endr labels
static void update_bms_temps(void)
{
    float sum = 0.0f, maxTemp = -1000.0f;
    int count = 0;

    for (int i = 0; i < BMS_CELL_TEMP_COUNT; ++i) {
        float val = 0.0f;
        if (canDatabaseGetFloat(&database, ui.idx_cell_temp[i], &val) != CAN_DATABASE_VALID)
            continue;
        sum += val;
        if (val > maxTemp) maxTemp = val;
        ++count;
    }

    char maxText[16], avgText[16];
    if (count > 0) {
        snprintf(maxText, sizeof(maxText), "%.1fC", maxTemp);
        snprintf(avgText, sizeof(avgText), "%.1fC", sum / (float)count);
    } else {
        maxText[0] = avgText[0] = '-'; maxText[1] = avgText[1] = '-'; maxText[2] = avgText[2] = '\0';
    }
    gtk_label_set_text(ui.bmsMax,     maxText);
    gtk_label_set_text(ui.bmsAvg,     avgText);
    gtk_label_set_text(ui.endrBmsMax, maxText);
    gtk_label_set_text(ui.endrBmsAvg, avgText);
}

static void update_vcu_faults(void)
{
    float val = 0.0f;
    const char* text = (canDatabaseGetFloat(&database, ui.idx_vcu_fault, &val) == CAN_DATABASE_VALID)
                       ? (val > 0.5f ? "FAULT" : "No faults") : "No faults";
    gtk_label_set_text(ui.vcuFaults,     text);
    gtk_label_set_text(ui.endrVcuFaults, text);
}

static void update_mtr_temp(void)
{
    float val = 0.0f;
    char text[16] = "--";
    if (canDatabaseGetFloat(&database, ui.idx_mtr_temp, &val) == CAN_DATABASE_VALID)
        snprintf(text, sizeof(text), "%.1fC", val);
    gtk_label_set_text(ui.mtrTemp,     text);
    gtk_label_set_text(ui.endrMtrTemp, text);
}

static void update_inv_temp(void)
{
    float val = 0.0f;
    char text[16] = "--";
    if (canDatabaseGetFloat(&database, ui.idx_inv_temp, &val) == CAN_DATABASE_VALID)
        snprintf(text, sizeof(text), "%.1fC", val);
    gtk_label_set_text(ui.invTemp,     text);
    gtk_label_set_text(ui.endrInvTemp, text);
}

static BarData bseData  = {0.80f, 0.20f, 0.20f, 0.0f};
static BarData appsData = {0.20f, 0.75f, 0.25f, 0.0f};

// Updates bseData and redraws both speed and endr BSE bars
static void update_bse_bar(void)
{
    float val = 0.0f;
    if (canDatabaseGetFloat(&database, ui.idx_bse, &val) == CAN_DATABASE_VALID) {
        if (val < 0.0f)   val = 0.0f;
        if (val > 100.0f) val = 100.0f;
        bseData.percent = val / 100.0f;
    } else {
        bseData.percent = 0.0f;
    }
    if (ui.bseBar)     gtk_widget_queue_draw(ui.bseBar);
    if (ui.endrBseBar) gtk_widget_queue_draw(ui.endrBseBar);
}

// Updates appsData and redraws both speed and endr APPS bars
static void update_apps_bar(void)
{
    float val = 0.0f;
    if (canDatabaseGetFloat(&database, ui.idx_apps, &val) == CAN_DATABASE_VALID) {
        if (val < 0.0f)   val = 0.0f;
        if (val > 100.0f) val = 100.0f;
        appsData.percent = val / 100.0f;
    } else {
        appsData.percent = 0.0f;
    }
    if (ui.appsBar)     gtk_widget_queue_draw(ui.appsBar);
    if (ui.endrAppsBar) gtk_widget_queue_draw(ui.endrAppsBar);
}

// ============================================================
// Cairo background drawing
//
// Layers drawn (back to front):
//   1. Solid dark fill
//   2. Red→dark→green horizontal gradient over the grid area
//   3. Horizontal border lines (top / bottom of grid area)
//   4. Tick-mark decals along the top border
//   5. Bordered rectangle around the right BMS/VCU/AMK panel
// ============================================================

static void draw_tick_row(cairo_t* cr, float x0, float x1, float y, int n, bool downward)
{
    if (n < 2)
        return;

    float spacing  = (x1 - x0) / (float)(n - 1);
    float sign     = downward ? 1.0f : -1.0f;
    float h_outer  = 7.0f;
    float h_inner  = 4.5f;
    float half_w   = 2.5f;

    cairo_set_line_width(cr, 1.0f);

    for (int i = 0; i < n; ++i) {
        float x = x0 + i * spacing;

        /* Light outer pair */
        cairo_set_source_rgba(cr, 0.75, 0.75, 0.75, 0.85);
        cairo_move_to(cr, x - half_w, y + sign);
        cairo_line_to(cr, x - half_w, y + sign * (1.0f + h_outer));
        cairo_stroke(cr);
        cairo_move_to(cr, x + half_w, y + sign);
        cairo_line_to(cr, x + half_w, y + sign * (1.0f + h_outer));
        cairo_stroke(cr);

        /* Dark center tick */
        cairo_set_source_rgba(cr, 0.30, 0.30, 0.30, 0.90);
        cairo_move_to(cr, x, y);
        cairo_line_to(cr, x, y + sign * h_inner);
        cairo_stroke(cr);
    }
}

static void draw_background(GtkDrawingArea* area, cairo_t* cr,
                             int width, int height, gpointer user_data)
{
    (void) user_data;
    (void) area;

    /* ---- Determine gradient region from widget bounds ---- */
    graphene_rect_t b;
    float xGMin = 0.0f, yGMin = 0.0f, xGMax = (float)width, yGMax = (float)height;

    if (ui.grid && gtk_widget_compute_bounds(ui.grid, GTK_WIDGET(area), &b)) {
        xGMin = b.origin.x;
        yGMin = b.origin.y;
        xGMax = b.origin.x + b.size.width;
        yGMax = b.origin.y + b.size.height;
    }

    /* Clip right edge to right panel if available */
    if (ui.rightPanel && gtk_widget_compute_bounds(ui.rightPanel, GTK_WIDGET(area), &b))
        xGMax = b.origin.x + b.size.width;

    /* ---- 2. Top and bottom border lines ---- */
    cairo_set_source_rgba(cr, 0.55f, 0.55f, 0.55f, 1.0f);
    cairo_set_line_width(cr, 1.0f);

    cairo_move_to(cr, xGMin, yGMin);
    cairo_line_to(cr, xGMax, yGMin);
    cairo_stroke(cr);

    cairo_move_to(cr, xGMin, yGMax);
    cairo_line_to(cr, xGMax, yGMax);
    cairo_stroke(cr);

    /* ---- 4. Tick-mark decals along top border ---- */
    float margin      = 15.0f;
    float tick_span_x = xGMax - xGMin - margin * 2.0f;
    int   tick_count  = (int)(tick_span_x / 25.0f) + 1;
    draw_tick_row(cr, xGMin + margin, xGMax - margin, yGMin, tick_count, true);

    /* ---- 6. Bordered rectangle around the right BMS/VCU/AMK panel ---- */
    if (ui.rightPanel && gtk_widget_compute_bounds(ui.rightPanel, GTK_WIDGET(area), &b)) {
        float pad = 5.0f;
        cairo_set_source_rgba(cr, 0.55f, 0.55f, 0.55f, 1.0f);
        cairo_set_line_width(cr, 1.0f);
        cairo_rectangle(cr,
            b.origin.x - pad,
            b.origin.y - pad,
            b.size.width  + pad * 2.0f,
            b.size.height + pad * 2.0f);
        cairo_stroke(cr);
    }
}

// ============================================================
// Thin vertical progress-bar widget (BSE / APPS indicators)
// Uses GtkDrawingArea, coloured solid fill.
// ============================================================

static void draw_vert_bar(GtkDrawingArea* area, cairo_t* cr, int w, int h, gpointer data)
{
    (void) area;
    const BarData* d = data;
    float pct = d->percent < 0.0f ? 0.0f : d->percent > 1.0f ? 1.0f : d->percent;
    int fill_h = (int)(h * pct);
    /* Background */
    cairo_set_source_rgba(cr, 0.12f, 0.12f, 0.12f, 1.0f);
    cairo_rectangle(cr, 0, 0, w, h);
    cairo_fill(cr);
    /* Fill bar from bottom */
    if (fill_h > 0) {
        cairo_set_source_rgba(cr, d->r, d->g, d->b, 1.0f);
        cairo_rectangle(cr, 0, h - fill_h, w, fill_h);
        cairo_fill(cr);
    }
    /* Border */
    cairo_set_source_rgba(cr, 0.45f, 0.45f, 0.45f, 1.0f);
    cairo_set_line_width(cr, 1.0f);
    cairo_rectangle(cr, 0.5f, 0.5f, w - 1.0f, h - 1.0f);
    cairo_stroke(cr);
}

// ============================================================
// Page switching
// ============================================================

static void switch_to_bms(GtkWidget* btn, gpointer data)
{
    (void)btn; (void)data;
    gtk_stack_set_visible_child_name(GTK_STACK(ui.stack), "bms");
}

static void switch_to_can(GtkWidget* btn, gpointer data)
{
    (void)btn; (void)data;
    gtk_stack_set_visible_child_name(GTK_STACK(ui.stack), "can");
}

static void switch_to_endr(GtkWidget* btn, gpointer data)
{
    (void)btn; (void)data;
    gtk_stack_set_visible_child_name(GTK_STACK(ui.stack), "endr");
}

static void switch_to_speed(GtkWidget* btn, gpointer data)
{
    (void)btn; (void)data;
    gtk_stack_set_visible_child_name(GTK_STACK(ui.stack), "speed");
}

// ============================================================
// Endurance page Cairo background (same decorations as speed
// page but referencing endr-specific widget pointers)
// ============================================================

static void draw_endr_background(GtkDrawingArea* area, cairo_t* cr,
                                  int width, int height, gpointer user_data)
{
    (void)user_data;
    (void)area;

    graphene_rect_t b;
    float xGMin = 0.0f, yGMin = 0.0f, xGMax = (float)width, yGMax = (float)height;

    if (ui.endrGrid && gtk_widget_compute_bounds(ui.endrGrid, GTK_WIDGET(area), &b)) {
        xGMin = b.origin.x;
        yGMin = b.origin.y;
        xGMax = b.origin.x + b.size.width;
        yGMax = b.origin.y + b.size.height;
    }
    if (ui.endrRightPanel && gtk_widget_compute_bounds(ui.endrRightPanel, GTK_WIDGET(area), &b))
        xGMax = b.origin.x + b.size.width;

    cairo_set_source_rgba(cr, 0.55f, 0.55f, 0.55f, 1.0f);
    cairo_set_line_width(cr, 1.0f);
    cairo_move_to(cr, xGMin, yGMin);
    cairo_line_to(cr, xGMax, yGMin);
    cairo_stroke(cr);
    cairo_move_to(cr, xGMin, yGMax);
    cairo_line_to(cr, xGMax, yGMax);
    cairo_stroke(cr);

    float margin     = 15.0f;
    float tick_span  = xGMax - xGMin - margin * 2.0f;
    int   tick_count = (int)(tick_span / 25.0f) + 1;
    draw_tick_row(cr, xGMin + margin, xGMax - margin, yGMin, tick_count, true);

    if (ui.endrRightPanel && gtk_widget_compute_bounds(ui.endrRightPanel, GTK_WIDGET(area), &b)) {
        float pad = 5.0f;
        cairo_set_source_rgba(cr, 0.55f, 0.55f, 0.55f, 1.0f);
        cairo_set_line_width(cr, 1.0f);
        cairo_rectangle(cr,
            b.origin.x - pad, b.origin.y - pad,
            b.size.width + pad * 2.0f, b.size.height + pad * 2.0f);
        cairo_stroke(cr);
    }
}

// ============================================================
// Endurance page CAN update callbacks
// ============================================================

static void update_pack_voltage(void)
{
    float val = 0.0f;
    char text[16] = "--";
    if (canDatabaseGetFloat(&database, ui.idx_pack_voltage, &val) == CAN_DATABASE_VALID)
        snprintf(text, sizeof(text), "%.1f", val);
    gtk_label_set_text(ui.packVoltage, text);
}

static void update_power_avg(void)
{
    float val = 0.0f;
    char text[16] = "--";
    if (canDatabaseGetFloat(&database, ui.idx_power_avg, &val) == CAN_DATABASE_VALID)
        snprintf(text, sizeof(text), "%.1f", val);
    gtk_label_set_text(ui.powerAvg, text);
}

static void update_energy_delivered(void)
{
    float val = 0.0f;
    char text[16] = "--";
    if (canDatabaseGetFloat(&database, ui.idx_energy_delivered, &val) == CAN_DATABASE_VALID)
        snprintf(text, sizeof(text), "%.2f", val);
    gtk_label_set_text(ui.energyDelivered, text);
}

// ============================================================
// CAN bus page update — refreshes all signal value labels
// ============================================================

static gboolean update_can_page(gpointer unused)
{
    (void)unused;
    for (size_t i = 0; i < ui.signalLabelCount; ++i) {
        if (ui.signalLabels[i] == NULL)
            continue;
        float val = 0.0f;
        char text[24];
        if (canDatabaseGetFloat(&database, (ssize_t)i, &val) == CAN_DATABASE_VALID)
            snprintf(text, sizeof(text), "%.4g", val);
        else
            snprintf(text, sizeof(text), "--");
        gtk_label_set_text(ui.signalLabels[i], text);
    }
    return TRUE;
}

static gboolean update_bms_volt_page(gpointer unused)
{
    (void)unused;
    for (int c = 0; c < 144; ++c) {
        if (ui.cellVoltLabels[c] == NULL) continue;
        float val = 0.0f;
        char text[12];
        if (ui.cellVoltIdx[c] >= 0 &&
            canDatabaseGetFloat(&database, ui.cellVoltIdx[c], &val) == CAN_DATABASE_VALID)
            snprintf(text, sizeof(text), "%.3fV", val);
        else
            snprintf(text, sizeof(text), "--");
        gtk_label_set_text(ui.cellVoltLabels[c], text);
    }
    return TRUE;
}

// Single 30fps master update — replaces many individual timers
static gboolean update_all(gpointer unused)
{
    (void)unused;
    update_speed();
    update_logger();
    update_bms_temps();
    update_vcu_faults();
    update_mtr_temp();
    update_inv_temp();
    update_bse_bar();
    update_apps_bar();
    update_pack_voltage();
    update_power_avg();
    update_energy_delivered();
    update_can_page(NULL);
    update_bms_volt_page(NULL);
    return TRUE;
}

// ============================================================
// activate() — build all widgets
// ============================================================

static void activate(GtkApplication* app, gpointer title_ptr)
{
    /* ---- Window ---- */
    GtkWidget* window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), (const char*)title_ptr);
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 480);
    gtk_window_fullscreen(GTK_WINDOW(window));

    /* ---- Global CSS ---- */
    GtkCssProvider* css = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css,
        /* Window background */
        "window { background: linear-gradient(90deg, #e4a089, #67a56f); }"

        /* Navigation buttons */
        ".nav-btn {"
        "  background: #181818;"
        "  color: #bbbbbb;"
        "  border: 1px solid #484848;"
        "  border-radius: 2px;"
        "  min-width: 80px;"
        "  min-height: 36px;"
        "  padding: 0px 4px;"
        "}"
        ".nav-btn:hover { background: #252525; border-color: #777; }"
        ".nav-btn-active {"
        "  background: #242424;"
        "  color: #ffffff;"
        "  border: 1px solid #888888;"
        "  border-radius: 2px;"
        "  min-width: 80px;"
        "  min-height: 36px;"
        "  padding: 0px 4px;"
        "}"
        ".rotated-label { transform: rotate(-90deg); }"
        ".speed-box {"
        "  background: #000000;"
        "  border: 2px solid #000000;"
        "  border-radius: 4px;"
        "}"
        ".cell-tile {"
        "  background: #111111;"
        "  border: 1px solid #333333;"
        "  border-radius: 2px;"
        "}"
        , -1);
    gtk_style_context_add_provider_for_display(
        gtk_widget_get_display(window),
        GTK_STYLE_PROVIDER(css),
        GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(css);

    /* ---- Root stack: switches between speed page and CAN bus page ---- */
    ui.stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(ui.stack), GTK_STACK_TRANSITION_TYPE_NONE);
    gtk_window_set_child(GTK_WINDOW(window), ui.stack);

    /* ---- Speed page: Cairo bg under grid ---- */
    GtkWidget* overlay = gtk_overlay_new();
    gtk_stack_add_named(GTK_STACK(ui.stack), overlay, "speed");

    ui.bg = gtk_drawing_area_new();
    gtk_drawing_area_set_draw_func(
        GTK_DRAWING_AREA(ui.bg), draw_background, NULL, NULL);
    gtk_overlay_set_child(GTK_OVERLAY(overlay), ui.bg);

    /* ---- Main grid ---- */
    ui.grid = gtk_grid_new();
    gtk_widget_set_margin_top   (ui.grid, 8);
    gtk_widget_set_margin_bottom(ui.grid, 8);
    gtk_widget_set_margin_start (ui.grid, 4);
    gtk_widget_set_margin_end   (ui.grid, 4);
    gtk_widget_set_hexpand(ui.grid, TRUE);
    gtk_widget_set_vexpand(ui.grid, TRUE);
    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), ui.grid);
    gtk_overlay_set_measure_overlay(GTK_OVERLAY(overlay), ui.grid, TRUE);

    /* ==========================================================
       Column 0 — BSE vertical bar (left edge)
       ========================================================== */
    ui.bseBar = gtk_drawing_area_new();
    gtk_drawing_area_set_draw_func(
        GTK_DRAWING_AREA(ui.bseBar), draw_vert_bar, (gpointer)&bseData, NULL);
    gtk_widget_set_size_request(ui.bseBar, 10, -1);
    gtk_widget_set_vexpand(ui.bseBar, TRUE);
    gtk_widget_set_margin_end(ui.bseBar, 6);
    gtk_grid_attach(GTK_GRID(ui.grid), ui.bseBar, 0, 0, 1, 4);

    GtkWidget* bseLabel = GTK_WIDGET(make_label("BSE", "Monospace 7", 0.7f, 0.7f, 0.7f));
    gtk_widget_add_css_class(bseLabel, "rotated-label");
    gtk_widget_set_margin_bottom(bseLabel, 4);
    gtk_widget_set_valign(bseLabel, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(ui.grid), bseLabel, 0, 4, 1, 1);

    /* ==========================================================
       Columns 1-2, Row 0 — Data Logger panel (top-left)
       ========================================================== */
    ui.dataLoggerPanel = gtk_grid_new();
    gtk_widget_set_margin_top  (ui.dataLoggerPanel, 12);
    gtk_widget_set_margin_start(ui.dataLoggerPanel, 8);
    gtk_widget_set_valign(ui.dataLoggerPanel, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(ui.grid), ui.dataLoggerPanel, 1, 0, 1, 1);

    ui.loggerTitle = make_label("LOGGER OFF", "Monospace Bold 13", 1.0f, 1.0f, 1.0f);
    gtk_label_set_xalign(ui.loggerTitle, 0.0f);
    gtk_grid_attach(GTK_GRID(ui.dataLoggerPanel), GTK_WIDGET(ui.loggerTitle), 0, 0, 1, 1);

    ui.loggerStat = make_label("Session\nNo. 273", "Monospace 8", 0.75f, 0.75f, 0.75f);
    gtk_label_set_xalign(ui.loggerStat, 0.0f);
    gtk_widget_set_margin_top(GTK_WIDGET(ui.loggerStat), 2);
    gtk_grid_attach(GTK_GRID(ui.dataLoggerPanel), GTK_WIDGET(ui.loggerStat), 0, 1, 1, 1);

    /* ==========================================================
       Columns 2-3, Rows 0-3 — Combined black box: speed + BMS/VCU/AMK
       ========================================================== */
    GtkWidget* mainBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_add_css_class(mainBox, "speed-box");
    gtk_widget_set_hexpand(mainBox, TRUE);
    gtk_widget_set_vexpand(mainBox, TRUE);
    gtk_grid_attach(GTK_GRID(ui.grid), mainBox, 2, 1, 2, 3);

    /* Left side: Vehicle Velocity / speed / MPH */
    GtkWidget* speedBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_hexpand(speedBox, TRUE);
    gtk_widget_set_vexpand(speedBox, TRUE);
    gtk_box_append(GTK_BOX(mainBox), speedBox);

    ui.velocityTitle = GTK_WIDGET(make_label("Vehicle Velocity:", "Monospace 10", 0.70f, 0.70f, 0.70f));
    gtk_label_set_xalign(GTK_LABEL(ui.velocityTitle), 0.5f);
    gtk_widget_set_margin_top(ui.velocityTitle, 6);
    gtk_widget_set_vexpand(ui.velocityTitle, FALSE);
    gtk_box_append(GTK_BOX(speedBox), ui.velocityTitle);

    ui.speedVal = make_label("42", "Technology Bold 96", 0.957f, 0.576f, 0.118f);
    gtk_label_set_xalign(ui.speedVal, 0.5f);
    gtk_widget_set_halign(GTK_WIDGET(ui.speedVal), GTK_ALIGN_CENTER);
    gtk_widget_set_valign(GTK_WIDGET(ui.speedVal), GTK_ALIGN_CENTER);
    gtk_widget_set_vexpand(GTK_WIDGET(ui.speedVal), TRUE);
    gtk_box_append(GTK_BOX(speedBox), GTK_WIDGET(ui.speedVal));

    GtkLabel* mphLabel = make_label("MPH", "Monospace Bold 15", 0.80f, 0.80f, 0.80f);
    gtk_label_set_xalign(mphLabel, 0.5f);
    gtk_widget_set_margin_bottom(GTK_WIDGET(mphLabel), 6);
    gtk_widget_set_vexpand(GTK_WIDGET(mphLabel), FALSE);
    gtk_box_append(GTK_BOX(speedBox), GTK_WIDGET(mphLabel));

    /* Right side: BMS / VCU / AMK */
    ui.rightPanel = gtk_grid_new();
    gtk_widget_set_margin_start(ui.rightPanel, 8);
    gtk_widget_set_margin_end  (ui.rightPanel, 8);
    gtk_widget_set_valign(ui.rightPanel, GTK_ALIGN_CENTER);
    gtk_widget_set_vexpand(ui.rightPanel, TRUE);
    gtk_grid_set_row_spacing(GTK_GRID(ui.rightPanel), 1);
    gtk_box_append(GTK_BOX(mainBox), ui.rightPanel);

    int r = 0;

    /* -- BMS -- */
    GtkLabel* bmsTitle = make_label("BMS", "Monospace Bold 15", 1.0f, 1.0f, 1.0f);
    gtk_label_set_xalign(bmsTitle, 0.5f);
    gtk_widget_set_hexpand(GTK_WIDGET(bmsTitle), TRUE);
    gtk_widget_set_margin_bottom(GTK_WIDGET(bmsTitle), 2);
    gtk_grid_attach(GTK_GRID(ui.rightPanel), GTK_WIDGET(bmsTitle), 0, r++, 2, 1);

    GtkLabel* bmsMaxLbl = make_label("MAX:", "Monospace 11", 0.70f, 0.70f, 0.70f);
    gtk_label_set_xalign(bmsMaxLbl, 0.0f);
    gtk_widget_set_margin_start(GTK_WIDGET(bmsMaxLbl), 4);
    gtk_grid_attach(GTK_GRID(ui.rightPanel), GTK_WIDGET(bmsMaxLbl), 0, r, 1, 1);
    ui.bmsMax = make_label("25.1C", "Monospace 11", 0.90f, 0.90f, 0.90f);
    gtk_label_set_xalign(ui.bmsMax, 1.0f);
    gtk_widget_set_margin_end(GTK_WIDGET(ui.bmsMax), 4);
    gtk_grid_attach(GTK_GRID(ui.rightPanel), GTK_WIDGET(ui.bmsMax), 1, r++, 1, 1);

    GtkLabel* bmsAvgLbl = make_label("AVG:", "Monospace 11", 0.70f, 0.70f, 0.70f);
    gtk_label_set_xalign(bmsAvgLbl, 0.0f);
    gtk_widget_set_margin_start(GTK_WIDGET(bmsAvgLbl), 4);
    gtk_grid_attach(GTK_GRID(ui.rightPanel), GTK_WIDGET(bmsAvgLbl), 0, r, 1, 1);
    ui.bmsAvg = make_label("22.1C", "Monospace 11", 0.90f, 0.90f, 0.90f);
    gtk_label_set_xalign(ui.bmsAvg, 1.0f);
    gtk_widget_set_margin_end(GTK_WIDGET(ui.bmsAvg), 4);
    gtk_grid_attach(GTK_GRID(ui.rightPanel), GTK_WIDGET(ui.bmsAvg), 1, r++, 1, 1);

    /* spacer */
    GtkWidget* sp1 = gtk_label_new("");
    gtk_widget_set_size_request(sp1, 0, 8);
    gtk_grid_attach(GTK_GRID(ui.rightPanel), sp1, 0, r++, 2, 1);

    /* -- VCU -- */
    GtkLabel* vcuTitle = make_label("VCU", "Monospace Bold 15", 1.0f, 1.0f, 1.0f);
    gtk_label_set_xalign(vcuTitle, 0.5f);
    gtk_widget_set_margin_bottom(GTK_WIDGET(vcuTitle), 2);
    gtk_grid_attach(GTK_GRID(ui.rightPanel), GTK_WIDGET(vcuTitle), 0, r++, 2, 1);

    ui.vcuFaults = make_label("No faults", "Monospace 11", 0.75f, 0.75f, 0.75f);
    gtk_label_set_xalign(ui.vcuFaults, 0.5f);
    gtk_grid_attach(GTK_GRID(ui.rightPanel), GTK_WIDGET(ui.vcuFaults), 0, r++, 2, 1);

    /* spacer */
    GtkWidget* sp2 = gtk_label_new("");
    gtk_widget_set_size_request(sp2, 0, 8);
    gtk_grid_attach(GTK_GRID(ui.rightPanel), sp2, 0, r++, 2, 1);

    /* -- AMK -- */
    GtkLabel* amkTitle = make_label("AMK", "Monospace Bold 15", 1.0f, 1.0f, 1.0f);
    gtk_label_set_xalign(amkTitle, 0.5f);
    gtk_widget_set_margin_bottom(GTK_WIDGET(amkTitle), 2);
    gtk_grid_attach(GTK_GRID(ui.rightPanel), GTK_WIDGET(amkTitle), 0, r++, 2, 1);

    GtkLabel* mtrLbl = make_label("MTR:", "Monospace 11", 0.70f, 0.70f, 0.70f);
    gtk_label_set_xalign(mtrLbl, 0.0f);
    gtk_widget_set_margin_start(GTK_WIDGET(mtrLbl), 4);
    gtk_grid_attach(GTK_GRID(ui.rightPanel), GTK_WIDGET(mtrLbl), 0, r, 1, 1);
    ui.mtrTemp = make_label("91.4C", "Monospace 11", 0.90f, 0.90f, 0.90f);
    gtk_label_set_xalign(ui.mtrTemp, 1.0f);
    gtk_widget_set_margin_end(GTK_WIDGET(ui.mtrTemp), 4);
    gtk_grid_attach(GTK_GRID(ui.rightPanel), GTK_WIDGET(ui.mtrTemp), 1, r++, 1, 1);

    GtkLabel* invLbl = make_label("INV:", "Monospace 11", 0.70f, 0.70f, 0.70f);
    gtk_label_set_xalign(invLbl, 0.0f);
    gtk_widget_set_margin_start(GTK_WIDGET(invLbl), 4);
    gtk_grid_attach(GTK_GRID(ui.rightPanel), GTK_WIDGET(invLbl), 0, r, 1, 1);
    ui.invTemp = make_label("46.6C", "Monospace 11", 0.90f, 0.90f, 0.90f);
    gtk_label_set_xalign(ui.invTemp, 1.0f);
    gtk_widget_set_margin_end(GTK_WIDGET(ui.invTemp), 4);
    gtk_grid_attach(GTK_GRID(ui.rightPanel), GTK_WIDGET(ui.invTemp), 1, r++, 1, 1);

    /* ==========================================================
       Row 4 (cols 1-3) — Navigation button bar
       ========================================================== */
    ui.buttonPanel = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_widget_set_margin_top   (ui.buttonPanel, 12);
    gtk_widget_set_margin_bottom(ui.buttonPanel, 4);
    gtk_widget_set_margin_start (ui.buttonPanel, 4);
    gtk_box_set_homogeneous(GTK_BOX(ui.buttonPanel), TRUE);
    gtk_grid_attach(GTK_GRID(ui.grid), ui.buttonPanel, 1, 4, 3, 1);

    const char* btn_labels[] = {"SPEED", "ENDR", "BMS", "CAN", "BACK"};
    for (int i = 0; i < 5; ++i) {
        GtkWidget* btn = gtk_button_new_with_label(btn_labels[i]);
        if (i == 0)
            gtk_widget_add_css_class(btn, "nav-btn-active");
        else
            gtk_widget_add_css_class(btn, "nav-btn");
        if (i == 1)  /* ENDR button → endurance page */
            g_signal_connect(btn, "clicked", G_CALLBACK(switch_to_endr), NULL);
        if (i == 2)  /* BMS button → BMS cell voltage page */
            g_signal_connect(btn, "clicked", G_CALLBACK(switch_to_bms), NULL);
        if (i == 3)  /* CAN button → CAN bus page */
            g_signal_connect(btn, "clicked", G_CALLBACK(switch_to_can), NULL);
        gtk_box_append(GTK_BOX(ui.buttonPanel), btn);
    }

    /* ==========================================================
       Column 4, Rows 0-3 — APPS vertical bar (right edge)
       ========================================================== */
    ui.appsBar = gtk_drawing_area_new();
    gtk_drawing_area_set_draw_func(
        GTK_DRAWING_AREA(ui.appsBar), draw_vert_bar, (gpointer)&appsData, NULL);
    gtk_widget_set_size_request(ui.appsBar, 10, -1);
    gtk_widget_set_vexpand(ui.appsBar, TRUE);
    gtk_widget_set_margin_start(ui.appsBar, 6);
    gtk_grid_attach(GTK_GRID(ui.grid), ui.appsBar, 4, 0, 1, 4);

    GtkWidget* appsLabel = GTK_WIDGET(make_label("APPS", "Monospace 7", 0.7f, 0.7f, 0.7f));
    gtk_widget_add_css_class(appsLabel, "rotated-label");
    gtk_widget_set_margin_bottom(appsLabel, 4);
    gtk_widget_set_valign(appsLabel, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(ui.grid), appsLabel, 4, 4, 1, 1);

    /* ==========================================================
       Endurance page — mirrors speed page layout, center box
       shows PACK_VOLTAGE / POWER_ROLLING_AVERAGE / ENERGY_DELIVERED
       ========================================================== */
    GtkWidget* endrOverlay = gtk_overlay_new();
    gtk_stack_add_named(GTK_STACK(ui.stack), endrOverlay, "endr");

    GtkWidget* endrBg = gtk_drawing_area_new();
    gtk_drawing_area_set_draw_func(
        GTK_DRAWING_AREA(endrBg), draw_endr_background, NULL, NULL);
    gtk_overlay_set_child(GTK_OVERLAY(endrOverlay), endrBg);

    ui.endrGrid = gtk_grid_new();
    gtk_widget_set_margin_top   (ui.endrGrid, 8);
    gtk_widget_set_margin_bottom(ui.endrGrid, 8);
    gtk_widget_set_margin_start (ui.endrGrid, 4);
    gtk_widget_set_margin_end   (ui.endrGrid, 4);
    gtk_widget_set_hexpand(ui.endrGrid, TRUE);
    gtk_widget_set_vexpand(ui.endrGrid, TRUE);
    gtk_overlay_add_overlay(GTK_OVERLAY(endrOverlay), ui.endrGrid);
    gtk_overlay_set_measure_overlay(GTK_OVERLAY(endrOverlay), ui.endrGrid, TRUE);

    /* Col 0 — BSE bar */
    ui.endrBseBar = gtk_drawing_area_new();
    gtk_drawing_area_set_draw_func(
        GTK_DRAWING_AREA(ui.endrBseBar), draw_vert_bar, (gpointer)&bseData, NULL);
    gtk_widget_set_size_request(ui.endrBseBar, 10, -1);
    gtk_widget_set_vexpand(ui.endrBseBar, TRUE);
    gtk_widget_set_margin_end(ui.endrBseBar, 6);
    gtk_grid_attach(GTK_GRID(ui.endrGrid), ui.endrBseBar, 0, 0, 1, 4);

    GtkWidget* endrBseLbl = GTK_WIDGET(make_label("BSE", "Monospace 7", 0.7f, 0.7f, 0.7f));
    gtk_widget_add_css_class(endrBseLbl, "rotated-label");
    gtk_widget_set_margin_bottom(endrBseLbl, 4);
    gtk_widget_set_valign(endrBseLbl, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(ui.endrGrid), endrBseLbl, 0, 4, 1, 1);

    /* Col 1, Row 0 — data logger panel */
    ui.endrDataLoggerPanel = gtk_grid_new();
    gtk_widget_set_margin_top  (ui.endrDataLoggerPanel, 12);
    gtk_widget_set_margin_start(ui.endrDataLoggerPanel, 8);
    gtk_widget_set_valign(ui.endrDataLoggerPanel, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(ui.endrGrid), ui.endrDataLoggerPanel, 1, 0, 1, 1);

    ui.endrLoggerTitle = make_label("LOGGER OFF", "Monospace Bold 13", 1.0f, 1.0f, 1.0f);
    gtk_label_set_xalign(ui.endrLoggerTitle, 0.0f);
    gtk_grid_attach(GTK_GRID(ui.endrDataLoggerPanel), GTK_WIDGET(ui.endrLoggerTitle), 0, 0, 1, 1);

    ui.endrLoggerStat = make_label("Session\nNo. 273", "Monospace 8", 0.75f, 0.75f, 0.75f);
    gtk_label_set_xalign(ui.endrLoggerStat, 0.0f);
    gtk_widget_set_margin_top(GTK_WIDGET(ui.endrLoggerStat), 2);
    gtk_grid_attach(GTK_GRID(ui.endrDataLoggerPanel), GTK_WIDGET(ui.endrLoggerStat), 0, 1, 1, 1);

    /* Cols 2-3, Rows 1-3 — black box: 3 stacked energy signals */
    GtkWidget* endrMainBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_add_css_class(endrMainBox, "speed-box");
    gtk_widget_set_hexpand(endrMainBox, TRUE);
    gtk_widget_set_vexpand(endrMainBox, TRUE);
    gtk_grid_attach(GTK_GRID(ui.endrGrid), endrMainBox, 2, 1, 2, 3);

    /* Left side: 3 signal sections stacked vertically */
    GtkWidget* endrSignalBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_hexpand(endrSignalBox, TRUE);
    gtk_widget_set_vexpand(endrSignalBox, TRUE);
    gtk_box_append(GTK_BOX(endrMainBox), endrSignalBox);

    /* Helper: each signal section = title + value, sharing vertical space */
    struct { const char* title; const char* unit; GtkLabel** dest; } endr_signals[] = {
        { "Pack Voltage",        "V",   &ui.packVoltage     },
        { "Power Rolling Avg",   "kW",  &ui.powerAvg        },
        { "Energy Delivered",    "kWh", &ui.energyDelivered },
    };
    for (int si = 0; si < 3; ++si) {
        GtkWidget* section = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        gtk_widget_set_vexpand(section, TRUE);
        gtk_widget_set_hexpand(section, TRUE);
        gtk_box_append(GTK_BOX(endrSignalBox), section);

        /* Title row: signal name + unit */
        GtkWidget* titleRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
        gtk_widget_set_halign(titleRow, GTK_ALIGN_CENTER);
        gtk_widget_set_margin_top(titleRow, 4);
        gtk_box_append(GTK_BOX(section), titleRow);

        GtkLabel* titleLbl = make_label(endr_signals[si].title,
                                        "Monospace 10", 0.70f, 0.70f, 0.70f);
        gtk_box_append(GTK_BOX(titleRow), GTK_WIDGET(titleLbl));

        GtkLabel* unitLbl = make_label(endr_signals[si].unit,
                                       "Monospace 10", 0.50f, 0.50f, 0.50f);
        gtk_box_append(GTK_BOX(titleRow), GTK_WIDGET(unitLbl));

        /* Value — large, centered, Technology Bold */
        *endr_signals[si].dest = make_label("--", "Technology Bold 48",
                                             0.957f, 0.576f, 0.118f);
        gtk_label_set_xalign(*endr_signals[si].dest, 0.5f);
        gtk_widget_set_halign(GTK_WIDGET(*endr_signals[si].dest), GTK_ALIGN_CENTER);
        gtk_widget_set_valign(GTK_WIDGET(*endr_signals[si].dest), GTK_ALIGN_CENTER);
        gtk_widget_set_vexpand(GTK_WIDGET(*endr_signals[si].dest), TRUE);
        gtk_box_append(GTK_BOX(section), GTK_WIDGET(*endr_signals[si].dest));

        /* Separator between sections (skip after last) */
        if (si < 2) {
            GtkWidget* div = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
            gtk_box_append(GTK_BOX(endrSignalBox), div);
        }
    }

    /* Right side: BMS / VCU / AMK (same content as speed page) */
    ui.endrRightPanel = gtk_grid_new();
    gtk_widget_set_margin_start(ui.endrRightPanel, 8);
    gtk_widget_set_margin_end  (ui.endrRightPanel, 8);
    gtk_widget_set_valign(ui.endrRightPanel, GTK_ALIGN_CENTER);
    gtk_widget_set_vexpand(ui.endrRightPanel, TRUE);
    gtk_grid_set_row_spacing(GTK_GRID(ui.endrRightPanel), 1);
    gtk_box_append(GTK_BOX(endrMainBox), ui.endrRightPanel);

    int er = 0;

    GtkLabel* endrBmsTitle = make_label("BMS", "Monospace Bold 15", 1.0f, 1.0f, 1.0f);
    gtk_label_set_xalign(endrBmsTitle, 0.5f);
    gtk_widget_set_hexpand(GTK_WIDGET(endrBmsTitle), TRUE);
    gtk_widget_set_margin_bottom(GTK_WIDGET(endrBmsTitle), 2);
    gtk_grid_attach(GTK_GRID(ui.endrRightPanel), GTK_WIDGET(endrBmsTitle), 0, er++, 2, 1);

    GtkLabel* endrBmsMaxLbl = make_label("MAX:", "Monospace 11", 0.70f, 0.70f, 0.70f);
    gtk_label_set_xalign(endrBmsMaxLbl, 0.0f);
    gtk_widget_set_margin_start(GTK_WIDGET(endrBmsMaxLbl), 4);
    gtk_grid_attach(GTK_GRID(ui.endrRightPanel), GTK_WIDGET(endrBmsMaxLbl), 0, er, 1, 1);
    ui.endrBmsMax = make_label("--", "Monospace 11", 0.90f, 0.90f, 0.90f);
    gtk_label_set_xalign(ui.endrBmsMax, 1.0f);
    gtk_widget_set_margin_end(GTK_WIDGET(ui.endrBmsMax), 4);
    gtk_grid_attach(GTK_GRID(ui.endrRightPanel), GTK_WIDGET(ui.endrBmsMax), 1, er++, 1, 1);

    GtkLabel* endrBmsAvgLbl = make_label("AVG:", "Monospace 11", 0.70f, 0.70f, 0.70f);
    gtk_label_set_xalign(endrBmsAvgLbl, 0.0f);
    gtk_widget_set_margin_start(GTK_WIDGET(endrBmsAvgLbl), 4);
    gtk_grid_attach(GTK_GRID(ui.endrRightPanel), GTK_WIDGET(endrBmsAvgLbl), 0, er, 1, 1);
    ui.endrBmsAvg = make_label("--", "Monospace 11", 0.90f, 0.90f, 0.90f);
    gtk_label_set_xalign(ui.endrBmsAvg, 1.0f);
    gtk_widget_set_margin_end(GTK_WIDGET(ui.endrBmsAvg), 4);
    gtk_grid_attach(GTK_GRID(ui.endrRightPanel), GTK_WIDGET(ui.endrBmsAvg), 1, er++, 1, 1);

    GtkWidget* endrSp1 = gtk_label_new(""); gtk_widget_set_size_request(endrSp1, 0, 8);
    gtk_grid_attach(GTK_GRID(ui.endrRightPanel), endrSp1, 0, er++, 2, 1);

    GtkLabel* endrVcuTitle = make_label("VCU", "Monospace Bold 15", 1.0f, 1.0f, 1.0f);
    gtk_label_set_xalign(endrVcuTitle, 0.5f);
    gtk_widget_set_margin_bottom(GTK_WIDGET(endrVcuTitle), 2);
    gtk_grid_attach(GTK_GRID(ui.endrRightPanel), GTK_WIDGET(endrVcuTitle), 0, er++, 2, 1);
    ui.endrVcuFaults = make_label("No faults", "Monospace 11", 0.75f, 0.75f, 0.75f);
    gtk_label_set_xalign(ui.endrVcuFaults, 0.5f);
    gtk_grid_attach(GTK_GRID(ui.endrRightPanel), GTK_WIDGET(ui.endrVcuFaults), 0, er++, 2, 1);

    GtkWidget* endrSp2 = gtk_label_new(""); gtk_widget_set_size_request(endrSp2, 0, 8);
    gtk_grid_attach(GTK_GRID(ui.endrRightPanel), endrSp2, 0, er++, 2, 1);

    GtkLabel* endrAmkTitle = make_label("AMK", "Monospace Bold 15", 1.0f, 1.0f, 1.0f);
    gtk_label_set_xalign(endrAmkTitle, 0.5f);
    gtk_widget_set_margin_bottom(GTK_WIDGET(endrAmkTitle), 2);
    gtk_grid_attach(GTK_GRID(ui.endrRightPanel), GTK_WIDGET(endrAmkTitle), 0, er++, 2, 1);

    GtkLabel* endrMtrLbl = make_label("MTR:", "Monospace 11", 0.70f, 0.70f, 0.70f);
    gtk_label_set_xalign(endrMtrLbl, 0.0f);
    gtk_widget_set_margin_start(GTK_WIDGET(endrMtrLbl), 4);
    gtk_grid_attach(GTK_GRID(ui.endrRightPanel), GTK_WIDGET(endrMtrLbl), 0, er, 1, 1);
    ui.endrMtrTemp = make_label("--", "Monospace 11", 0.90f, 0.90f, 0.90f);
    gtk_label_set_xalign(ui.endrMtrTemp, 1.0f);
    gtk_widget_set_margin_end(GTK_WIDGET(ui.endrMtrTemp), 4);
    gtk_grid_attach(GTK_GRID(ui.endrRightPanel), GTK_WIDGET(ui.endrMtrTemp), 1, er++, 1, 1);

    GtkLabel* endrInvLbl = make_label("INV:", "Monospace 11", 0.70f, 0.70f, 0.70f);
    gtk_label_set_xalign(endrInvLbl, 0.0f);
    gtk_widget_set_margin_start(GTK_WIDGET(endrInvLbl), 4);
    gtk_grid_attach(GTK_GRID(ui.endrRightPanel), GTK_WIDGET(endrInvLbl), 0, er, 1, 1);
    ui.endrInvTemp = make_label("--", "Monospace 11", 0.90f, 0.90f, 0.90f);
    gtk_label_set_xalign(ui.endrInvTemp, 1.0f);
    gtk_widget_set_margin_end(GTK_WIDGET(ui.endrInvTemp), 4);
    gtk_grid_attach(GTK_GRID(ui.endrRightPanel), GTK_WIDGET(ui.endrInvTemp), 1, er++, 1, 1);

    /* Row 4 — nav buttons (ENDR active) */
    GtkWidget* endrBtnPanel = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_widget_set_margin_top   (endrBtnPanel, 12);
    gtk_widget_set_margin_bottom(endrBtnPanel, 4);
    gtk_widget_set_margin_start (endrBtnPanel, 4);
    gtk_box_set_homogeneous(GTK_BOX(endrBtnPanel), TRUE);
    gtk_grid_attach(GTK_GRID(ui.endrGrid), endrBtnPanel, 1, 4, 3, 1);

    const char* endr_btn_labels[] = {"SPEED", "ENDR", "BMS", "CAN", "BACK"};
    for (int i = 0; i < 5; ++i) {
        GtkWidget* btn = gtk_button_new_with_label(endr_btn_labels[i]);
        if (i == 1)
            gtk_widget_add_css_class(btn, "nav-btn-active");
        else
            gtk_widget_add_css_class(btn, "nav-btn");
        if (i == 0)
            g_signal_connect(btn, "clicked", G_CALLBACK(switch_to_speed), NULL);
        if (i == 2)
            g_signal_connect(btn, "clicked", G_CALLBACK(switch_to_bms), NULL);
        if (i == 3)
            g_signal_connect(btn, "clicked", G_CALLBACK(switch_to_can), NULL);
        gtk_box_append(GTK_BOX(endrBtnPanel), btn);
    }

    /* Col 4 — APPS bar */
    ui.endrAppsBar = gtk_drawing_area_new();
    gtk_drawing_area_set_draw_func(
        GTK_DRAWING_AREA(ui.endrAppsBar), draw_vert_bar, (gpointer)&appsData, NULL);
    gtk_widget_set_size_request(ui.endrAppsBar, 10, -1);
    gtk_widget_set_vexpand(ui.endrAppsBar, TRUE);
    gtk_widget_set_margin_start(ui.endrAppsBar, 6);
    gtk_grid_attach(GTK_GRID(ui.endrGrid), ui.endrAppsBar, 4, 0, 1, 4);

    GtkWidget* endrAppsLbl = GTK_WIDGET(make_label("APPS", "Monospace 7", 0.7f, 0.7f, 0.7f));
    gtk_widget_add_css_class(endrAppsLbl, "rotated-label");
    gtk_widget_set_margin_bottom(endrAppsLbl, 4);
    gtk_widget_set_valign(endrAppsLbl, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(ui.endrGrid), endrAppsLbl, 4, 4, 1, 1);


    /* ==========================================================
       CAN bus page — scrollable signal list grouped by message
       ========================================================== */
    ui.signalLabelCount = database.signalCount;
    ui.signalLabels = calloc(ui.signalLabelCount, sizeof(GtkLabel*));

    GtkWidget* canPage = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_stack_add_named(GTK_STACK(ui.stack), canPage, "can");

    /* Header bar */
    GtkWidget* canHeader = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_margin_start (canHeader, 8);
    gtk_widget_set_margin_end   (canHeader, 8);
    gtk_widget_set_margin_top   (canHeader, 6);
    gtk_widget_set_margin_bottom(canHeader, 6);
    gtk_box_append(GTK_BOX(canPage), canHeader);

    GtkLabel* canTitle = make_label("CAN Bus Monitor", "Monospace Bold 13", 0.0f, 0.0f, 0.0f);
    gtk_label_set_xalign(canTitle, 0.0f);
    gtk_widget_set_hexpand(GTK_WIDGET(canTitle), TRUE);
    gtk_box_append(GTK_BOX(canHeader), GTK_WIDGET(canTitle));

    GtkWidget* backBtn = gtk_button_new_with_label("BACK");
    gtk_widget_add_css_class(backBtn, "nav-btn");
    g_signal_connect(backBtn, "clicked", G_CALLBACK(switch_to_speed), NULL);
    gtk_box_append(GTK_BOX(canHeader), backBtn);

    /* Scrollable signal list */
    GtkWidget* scroll = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_box_append(GTK_BOX(canPage), scroll);

    GtkWidget* signalList = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_margin_start(signalList, 8);
    gtk_widget_set_margin_end  (signalList, 8);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), signalList);

    for (size_t mi = 0; mi < database.messageCount; ++mi) {
        canMessage_t* msg = canDatabaseGetMessage(&database, (ssize_t)mi);
        if (msg == NULL || msg->signalCount == 0) continue;

        /* Message name as group header */
        GtkLabel* msgHdr = make_label(msg->name, "Monospace Bold 10", 0.0f, 0.0f, 0.0f);
        gtk_label_set_xalign(msgHdr, 0.0f);
        gtk_widget_set_margin_top   (GTK_WIDGET(msgHdr), 8);
        gtk_widget_set_margin_bottom(GTK_WIDGET(msgHdr), 2);
        gtk_box_append(GTK_BOX(signalList), GTK_WIDGET(msgHdr));

        GtkWidget* sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
        gtk_box_append(GTK_BOX(signalList), sep);

        for (size_t si = 0; si < msg->signalCount; ++si) {
            canSignal_t* sig = &msg->signals[si];
            size_t gi = (size_t)canDatabaseGetGlobalIndex(&database, mi, si);

            GtkWidget* row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
            gtk_widget_set_margin_start(row, 12);
            gtk_box_append(GTK_BOX(signalList), row);

            /* Signal name (expands to fill) */
            GtkLabel* nameLbl = make_label(sig->name, "Monospace 9", 0.0f, 0.0f, 0.0f);
            gtk_label_set_xalign(nameLbl, 0.0f);
            gtk_widget_set_hexpand(GTK_WIDGET(nameLbl), TRUE);
            gtk_box_append(GTK_BOX(row), GTK_WIDGET(nameLbl));

            /* Unit */
            if (sig->unit && sig->unit[0] != '\0') {
                GtkLabel* unitLbl = make_label(sig->unit, "Monospace 9", 0.0f, 0.0f, 0.0f);
                gtk_label_set_xalign(unitLbl, 1.0f);
                gtk_widget_set_margin_end(GTK_WIDGET(unitLbl), 4);
                gtk_box_append(GTK_BOX(row), GTK_WIDGET(unitLbl));
            }

            /* Value (fixed width so column stays aligned) */
            GtkLabel* valLbl = make_label("--", "Monospace 9", 0.0f, 0.0f, 0.0f);
            gtk_label_set_xalign(valLbl, 1.0f);
            gtk_widget_set_size_request(GTK_WIDGET(valLbl), 80, -1);
            gtk_box_append(GTK_BOX(row), GTK_WIDGET(valLbl));

            if (gi < ui.signalLabelCount)
                ui.signalLabels[gi] = valLbl;
        }
    }

    /* ==========================================================
       Pre-cache all named signal indices (done once, used every frame)
       ========================================================== */
    ui.idx_speed            = canDatabaseFindSignal(&database, "RL_ACTUAL_SPEED_VALUE");
    ui.idx_session          = canDatabaseFindSignal(&database, "SESSION_NUMBER");
    ui.idx_vcu_fault        = canDatabaseFindSignal(&database, "VCU_FAULT");
    ui.idx_bse              = canDatabaseFindSignal(&database, "BSE_FRONT_PERCENT");
    ui.idx_apps             = canDatabaseFindSignal(&database, "APPS_1_PERCENT");
    ui.idx_mtr_temp         = canDatabaseFindSignal(&database, "AMK_MOTOR_TEMPERATURE_MAX");
    ui.idx_inv_temp         = canDatabaseFindSignal(&database, "AMK_INVERTER_TEMPERATURE_MAX");
    ui.idx_pack_voltage     = canDatabaseFindSignal(&database, "PACK_VOLTAGE");
    ui.idx_power_avg        = canDatabaseFindSignal(&database, "POWER_ROLLING_AVERAGE");
    ui.idx_energy_delivered = canDatabaseFindSignal(&database, "ENERGY_DELIVERED");
    for (int i = 0; i < BMS_CELL_TEMP_COUNT; ++i)
        ui.idx_cell_temp[i] = canDatabaseFindSignal(&database, BMS_CELL_TEMP_SIGNALS[i]);

    /* ==========================================================
       BMS cell voltage page — 12×12 grid of CELL_VOLTAGE_0..143
       ========================================================== */

    /* Pre-cache cell voltage signal indices */
    for (int c = 0; c < 144; ++c) {
        char name[24];
        snprintf(name, sizeof(name), "CELL_VOLTAGE_%d", c);
        ui.cellVoltIdx[c]    = canDatabaseFindSignal(&database, name);
        ui.cellVoltLabels[c] = NULL;
    }

    GtkWidget* bmsPage = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_name(bmsPage, "bmsPage");
    gtk_stack_add_named(GTK_STACK(ui.stack), bmsPage, "bms");

    /* Header bar */
    GtkWidget* bmsHeader = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_margin_start (bmsHeader, 8);
    gtk_widget_set_margin_end   (bmsHeader, 8);
    gtk_widget_set_margin_top   (bmsHeader, 6);
    gtk_widget_set_margin_bottom(bmsHeader, 4);
    gtk_box_append(GTK_BOX(bmsPage), bmsHeader);

    GtkLabel* bmsPageTitle = make_label("BMS Cell Voltages", "Monospace Bold 13", 1.0f, 1.0f, 1.0f);
    gtk_label_set_xalign(bmsPageTitle, 0.0f);
    gtk_widget_set_hexpand(GTK_WIDGET(bmsPageTitle), TRUE);
    gtk_box_append(GTK_BOX(bmsHeader), GTK_WIDGET(bmsPageTitle));

    GtkWidget* bmsBtnPanel = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_box_append(GTK_BOX(bmsHeader), bmsBtnPanel);

    GtkWidget* bmsBackBtn = gtk_button_new_with_label("BACK");
    gtk_widget_add_css_class(bmsBackBtn, "nav-btn");
    g_signal_connect(bmsBackBtn, "clicked", G_CALLBACK(switch_to_speed), NULL);
    gtk_box_append(GTK_BOX(bmsBtnPanel), bmsBackBtn);

    GtkWidget* bmsToCanBtn = gtk_button_new_with_label("CAN");
    gtk_widget_add_css_class(bmsToCanBtn, "nav-btn");
    g_signal_connect(bmsToCanBtn, "clicked", G_CALLBACK(switch_to_can), NULL);
    gtk_box_append(GTK_BOX(bmsBtnPanel), bmsToCanBtn);

    /* Separator below header */
    GtkWidget* bmsHdrSep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_append(GTK_BOX(bmsPage), bmsHdrSep);

    /* Scrollable area wrapping the cell grid */
    GtkWidget* bmsScroll = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(bmsScroll),
                                   GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_vexpand(bmsScroll, TRUE);
    gtk_box_append(GTK_BOX(bmsPage), bmsScroll);

    /* 12-column grid of cell tiles */
    GtkWidget* cellGrid = gtk_grid_new();
    gtk_grid_set_row_spacing   (GTK_GRID(cellGrid), 2);
    gtk_grid_set_column_spacing(GTK_GRID(cellGrid), 2);
    gtk_widget_set_margin_top   (cellGrid, 4);
    gtk_widget_set_margin_bottom(cellGrid, 4);
    gtk_widget_set_margin_start (cellGrid, 4);
    gtk_widget_set_margin_end   (cellGrid, 4);
    gtk_widget_set_hexpand(cellGrid, TRUE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(bmsScroll), cellGrid);

    for (int c = 0; c < 144; ++c) {
        int col = c % 12;
        int row = c / 12;

        GtkWidget* tile = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
        gtk_widget_add_css_class(tile, "cell-tile");
        gtk_widget_set_margin_top   (tile, 1);
        gtk_widget_set_margin_bottom(tile, 1);
        gtk_widget_set_margin_start (tile, 1);
        gtk_widget_set_margin_end   (tile, 1);
        gtk_widget_set_hexpand(tile, TRUE);
        gtk_widget_set_size_request(tile, 60, 34);

        /* Cell index label e.g. "C 47" */
        char idxText[8];
        snprintf(idxText, sizeof(idxText), "C%d", c);
        GtkLabel* idxLbl = make_label(idxText, "Monospace 7", 0.55f, 0.55f, 0.55f);
        gtk_label_set_xalign(idxLbl, 0.5f);
        gtk_widget_set_margin_top(GTK_WIDGET(idxLbl), 2);
        gtk_box_append(GTK_BOX(tile), GTK_WIDGET(idxLbl));

        /* Voltage value label */
        GtkLabel* voltLbl = make_label("--", "Monospace Bold 9",
                                       0.957f, 0.576f, 0.118f); /* #F4931E orange */
        gtk_label_set_xalign(voltLbl, 0.5f);
        gtk_widget_set_margin_bottom(GTK_WIDGET(voltLbl), 2);
        gtk_box_append(GTK_BOX(tile), GTK_WIDGET(voltLbl));

        gtk_grid_attach(GTK_GRID(cellGrid), tile, col, row, 1, 1);
        ui.cellVoltLabels[c] = voltLbl;
    }

    /* Single 30fps timer drives all UI updates */
    g_timeout_add(33, G_SOURCE_FUNC(update_all), NULL);

    gtk_window_present(GTK_WINDOW(window));
}

// ============================================================
// main()
// ============================================================

int main(int argc, char** argv)
{
    if (argc != 4) {
        fprintf(stderr, "Usage: dashboard-gui-jk <App Name> <Device Name> <DBC File>\n");
                return -1;
        }

    debugInit();

    canDevice_t* device = canInit(argv[2]);
    if (device == NULL) {
        fprintf(stderr, "Invalid device '%s': %s\n", argv[2], errorCodeToMessage(errno));
                return errno;
        }

    if (canDatabaseInit(&database, device, argv[3]) != 0) {
        fprintf(stderr, "Invalid DBC '%s': %s\n", argv[3], errorCodeToMessage(errno));
                return errno;
        }

    /* Build GTK application ID from argv[1] */
    const char* appName = argv[1];
    const char* domain  = "org.zre";
    size_t idLen = strlen(domain) + 1 + strlen(appName) + 1;
    char* appId  = malloc(idLen);
    snprintf(appId, idLen, "%s.%s", domain, appName);

    GtkApplication* app = gtk_application_new(appId, 0);
    g_signal_connect(app, "activate", G_CALLBACK(activate), (gpointer)appName);
    int status = g_application_run(G_APPLICATION(app), 1, argv);
    g_object_unref(app);
    free(appId);

        return status;
}
