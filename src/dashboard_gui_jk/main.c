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
    GtkLabel*   speedVal;        // Large center speed number
    GtkLabel*   loggerTitle;     // "LOGGER ON"
    GtkLabel*   loggerStat;      // "Session\nNo. 273"
    GtkLabel*   bmsMax;
    GtkLabel*   bmsAvg;
    GtkLabel*   vcuFaults;
    GtkLabel*   mtrTemp;
    GtkLabel*   invTemp;
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

static gboolean update_speed(GtkLabel* label)
{
    ssize_t idx = canDatabaseFindSignal(&database, "VCU_VEHICLE_SPEED");
    float val = 0.0f;
    char text[8] = "--";
    if (idx >= 0 && canDatabaseGetFloat(&database, idx, &val) == CAN_DATABASE_VALID)
        snprintf(text, sizeof(text), "%.0f", val);
    gtk_label_set_text(label, text);
    return TRUE;
}

static gboolean update_logger_title(GtkLabel* label)
{
    ssize_t idx = canDatabaseFindSignal(&database, "LOGGER_STATUS");
    float val = 0.0f;
    if (idx >= 0 && canDatabaseGetFloat(&database, idx, &val) == CAN_DATABASE_VALID)
        gtk_label_set_text(label, val > 0.5f ? "LOGGER ON" : "LOGGER OFF");
    return TRUE;
}

static gboolean update_logger_stat(GtkLabel* label)
{
    ssize_t idx = canDatabaseFindSignal(&database, "LOGGER_SESSION");
    float val = 0.0f;
    char text[32] = "Session\n--";
    if (idx >= 0 && canDatabaseGetFloat(&database, idx, &val) == CAN_DATABASE_VALID)
        snprintf(text, sizeof(text), "Session\nNo. %.0f", val);
    gtk_label_set_text(label, text);
        return TRUE;
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

static gboolean update_bms_max(GtkLabel* label)
{
    char text[16] = "--";
    bool found = false;
    float maxTemp = -1000.0f;

    for (int i = 0; i < BMS_CELL_TEMP_COUNT; ++i) {
        ssize_t idx = canDatabaseFindSignal(&database, BMS_CELL_TEMP_SIGNALS[i]);
        float val = 0.0f;
        if (idx >= 0 && canDatabaseGetFloat(&database, idx, &val) == CAN_DATABASE_VALID) {
            if (!found || val > maxTemp) {
                maxTemp = val;
                found = true;
            }
        }
    }

    if (found)
        snprintf(text, sizeof(text), "%.1fC", maxTemp);
    gtk_label_set_text(label, text);
    return TRUE;
}

static gboolean update_bms_avg(GtkLabel* label)
{
    char text[16] = "--";
    float sum = 0.0f;
    int count = 0;

    for (int i = 0; i < BMS_CELL_TEMP_COUNT; ++i) {
        ssize_t idx = canDatabaseFindSignal(&database, BMS_CELL_TEMP_SIGNALS[i]);
        float val = 0.0f;
        if (idx >= 0 && canDatabaseGetFloat(&database, idx, &val) == CAN_DATABASE_VALID) {
            sum += val;
            ++count;
        }
    }

    if (count > 0)
        snprintf(text, sizeof(text), "%.1fC", sum / (float)count);
    gtk_label_set_text(label, text);
    return TRUE;
}

static gboolean update_vcu_faults(GtkLabel* label)
{
    ssize_t idx = canDatabaseFindSignal(&database, "VCU_FAULT");
    float val = 0.0f;
    if (idx >= 0 && canDatabaseGetFloat(&database, idx, &val) == CAN_DATABASE_VALID)
        gtk_label_set_text(label, val > 0.5f ? "FAULT" : "No faults");
    return TRUE;
}

static gboolean update_mtr_temp(GtkLabel* label)
{
    ssize_t idx = canDatabaseFindSignal(&database, "AMK_MOTOR_TEMP");
    float val = 0.0f;
    char text[16] = "--";
    if (idx >= 0 && canDatabaseGetFloat(&database, idx, &val) == CAN_DATABASE_VALID)
        snprintf(text, sizeof(text), "%.1fC", val);
    gtk_label_set_text(label, text);
        return TRUE;
    }

static gboolean update_inv_temp(GtkLabel* label)
{
    ssize_t idx = canDatabaseFindSignal(&database, "AMK_INVERTER_TEMP");
    float val = 0.0f;
    char text[16] = "--";
    if (idx >= 0 && canDatabaseGetFloat(&database, idx, &val) == CAN_DATABASE_VALID)
        snprintf(text, sizeof(text), "%.1fC", val);
    gtk_label_set_text(label, text);
    return TRUE;
}

static BarData bseData  = {0.80f, 0.20f, 0.20f, 0.0f};
static BarData appsData = {0.20f, 0.75f, 0.25f, 0.0f};

static gboolean update_bse_bar(GtkWidget* area)
{
    ssize_t idx = canDatabaseFindSignal(&database, "BSE_FRONT_PERCENT");
    float val = 0.0f;
    if (idx >= 0 && canDatabaseGetFloat(&database, idx, &val) == CAN_DATABASE_VALID) {
        if (val < 0.0f)   val = 0.0f;
        if (val > 100.0f) val = 100.0f;
        bseData.percent = val / 100.0f;
    } else {
        bseData.percent = 0.0f;
    }
    gtk_widget_queue_draw(area);
    return TRUE;
}

static gboolean update_apps_bar(GtkWidget* area)
{
    ssize_t idx = canDatabaseFindSignal(&database, "APPS_1_PERCENT");
    float val = 0.0f;
    if (idx >= 0 && canDatabaseGetFloat(&database, idx, &val) == CAN_DATABASE_VALID) {
        if (val < 0.0f)   val = 0.0f;
        if (val > 100.0f) val = 100.0f;
        appsData.percent = val / 100.0f;
    } else {
        appsData.percent = 0.0f;
    }
    gtk_widget_queue_draw(area);
    return TRUE;
}

// ============================================================
// Cairo background drawing
//
// Layers drawn (back to front):
//   1. Solid dark fill
//   2. Red→dark→green horizontal gradient over the grid area
//   3. Horizontal border lines (top / bottom of grid area)
//   4. Tick-mark decals along the top border
//   5. Slanted divider from data-logger bottom-right to velocity-title
//   6. Bordered rectangle around the right BMS/VCU/AMK panel
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

    /* ---- 5. Slanted divider: data-logger → velocity title ---- */
    if (ui.dataLoggerPanel && ui.velocityTitle) {
        graphene_rect_t dlB, vtB;
        bool hasDl = gtk_widget_compute_bounds(ui.dataLoggerPanel, GTK_WIDGET(area), &dlB);
        bool hasVt = gtk_widget_compute_bounds(ui.velocityTitle,   GTK_WIDGET(area), &vtB);

        if (hasDl && hasVt) {
            float x0 = dlB.origin.x + dlB.size.width;
            float y0 = dlB.origin.y + dlB.size.height;
            float x1 = vtB.origin.x;
            float y1 = vtB.origin.y;

            cairo_set_source_rgba(cr, 0.55f, 0.55f, 0.55f, 1.0f);
            cairo_set_line_width(cr, 1.0f);
            cairo_move_to(cr, x0, y0);
            cairo_line_to(cr, x1, y1);
            cairo_stroke(cr);
        }
    }

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
// activate() — build all widgets
// ============================================================

static void activate(GtkApplication* app, gpointer title_ptr)
{
    /* ---- Window ---- */
    GtkWidget* window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), (const char*)title_ptr);
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 480);

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
        , -1);
    gtk_style_context_add_provider_for_display(
        gtk_widget_get_display(window),
        GTK_STYLE_PROVIDER(css),
        GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(css);

    /* ---- Root overlay: Cairo bg under grid ---- */
    GtkWidget* overlay = gtk_overlay_new();
    gtk_window_set_child(GTK_WINDOW(window), overlay);

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

    ui.loggerTitle = make_label("LOGGER ON", "Monospace Bold 13", 1.0f, 1.0f, 1.0f);
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
    GtkLabel* bmsTitle = make_label("BMS", "Monospace Bold 11", 1.0f, 1.0f, 1.0f);
    gtk_label_set_xalign(bmsTitle, 0.5f);
    gtk_widget_set_hexpand(GTK_WIDGET(bmsTitle), TRUE);
    gtk_widget_set_margin_bottom(GTK_WIDGET(bmsTitle), 2);
    gtk_grid_attach(GTK_GRID(ui.rightPanel), GTK_WIDGET(bmsTitle), 0, r++, 2, 1);

    GtkLabel* bmsMaxLbl = make_label("MAX:", "Monospace 9", 0.55f, 0.55f, 0.55f);
    gtk_label_set_xalign(bmsMaxLbl, 0.0f);
    gtk_widget_set_margin_start(GTK_WIDGET(bmsMaxLbl), 4);
    gtk_grid_attach(GTK_GRID(ui.rightPanel), GTK_WIDGET(bmsMaxLbl), 0, r, 1, 1);
    ui.bmsMax = make_label("25.1C", "Monospace 9", 0.90f, 0.90f, 0.90f);
    gtk_label_set_xalign(ui.bmsMax, 1.0f);
    gtk_widget_set_margin_end(GTK_WIDGET(ui.bmsMax), 4);
    gtk_grid_attach(GTK_GRID(ui.rightPanel), GTK_WIDGET(ui.bmsMax), 1, r++, 1, 1);

    GtkLabel* bmsAvgLbl = make_label("AVG:", "Monospace 9", 0.55f, 0.55f, 0.55f);
    gtk_label_set_xalign(bmsAvgLbl, 0.0f);
    gtk_widget_set_margin_start(GTK_WIDGET(bmsAvgLbl), 4);
    gtk_grid_attach(GTK_GRID(ui.rightPanel), GTK_WIDGET(bmsAvgLbl), 0, r, 1, 1);
    ui.bmsAvg = make_label("22.1C", "Monospace 9", 0.90f, 0.90f, 0.90f);
    gtk_label_set_xalign(ui.bmsAvg, 1.0f);
    gtk_widget_set_margin_end(GTK_WIDGET(ui.bmsAvg), 4);
    gtk_grid_attach(GTK_GRID(ui.rightPanel), GTK_WIDGET(ui.bmsAvg), 1, r++, 1, 1);

    /* spacer */
    GtkWidget* sp1 = gtk_label_new("");
    gtk_widget_set_size_request(sp1, 0, 8);
    gtk_grid_attach(GTK_GRID(ui.rightPanel), sp1, 0, r++, 2, 1);

    /* -- VCU -- */
    GtkLabel* vcuTitle = make_label("VCU", "Monospace Bold 11", 1.0f, 1.0f, 1.0f);
    gtk_label_set_xalign(vcuTitle, 0.5f);
    gtk_widget_set_margin_bottom(GTK_WIDGET(vcuTitle), 2);
    gtk_grid_attach(GTK_GRID(ui.rightPanel), GTK_WIDGET(vcuTitle), 0, r++, 2, 1);

    ui.vcuFaults = make_label("No faults", "Monospace 9", 0.75f, 0.75f, 0.75f);
    gtk_label_set_xalign(ui.vcuFaults, 0.5f);
    gtk_grid_attach(GTK_GRID(ui.rightPanel), GTK_WIDGET(ui.vcuFaults), 0, r++, 2, 1);

    /* spacer */
    GtkWidget* sp2 = gtk_label_new("");
    gtk_widget_set_size_request(sp2, 0, 8);
    gtk_grid_attach(GTK_GRID(ui.rightPanel), sp2, 0, r++, 2, 1);

    /* -- AMK -- */
    GtkLabel* amkTitle = make_label("AMK", "Monospace Bold 11", 1.0f, 1.0f, 1.0f);
    gtk_label_set_xalign(amkTitle, 0.5f);
    gtk_widget_set_margin_bottom(GTK_WIDGET(amkTitle), 2);
    gtk_grid_attach(GTK_GRID(ui.rightPanel), GTK_WIDGET(amkTitle), 0, r++, 2, 1);

    GtkLabel* mtrLbl = make_label("MTR:", "Monospace 9", 0.55f, 0.55f, 0.55f);
    gtk_label_set_xalign(mtrLbl, 0.0f);
    gtk_widget_set_margin_start(GTK_WIDGET(mtrLbl), 4);
    gtk_grid_attach(GTK_GRID(ui.rightPanel), GTK_WIDGET(mtrLbl), 0, r, 1, 1);
    ui.mtrTemp = make_label("91.4C", "Monospace 9", 0.90f, 0.90f, 0.90f);
    gtk_label_set_xalign(ui.mtrTemp, 1.0f);
    gtk_widget_set_margin_end(GTK_WIDGET(ui.mtrTemp), 4);
    gtk_grid_attach(GTK_GRID(ui.rightPanel), GTK_WIDGET(ui.mtrTemp), 1, r++, 1, 1);

    GtkLabel* invLbl = make_label("INV:", "Monospace 9", 0.55f, 0.55f, 0.55f);
    gtk_label_set_xalign(invLbl, 0.0f);
    gtk_widget_set_margin_start(GTK_WIDGET(invLbl), 4);
    gtk_grid_attach(GTK_GRID(ui.rightPanel), GTK_WIDGET(invLbl), 0, r, 1, 1);
    ui.invTemp = make_label("46.6C", "Monospace 9", 0.90f, 0.90f, 0.90f);
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

    const char* btn_labels[] = {"SPEED", "LAP", "ENDR", "BMS", "BACK"};
    for (int i = 0; i < 5; ++i) {
        GtkWidget* btn = gtk_button_new_with_label(btn_labels[i]);
        /* Style active/inactive pages */
        if (i == 0)
            gtk_widget_add_css_class(btn, "nav-btn-active");
        else
            gtk_widget_add_css_class(btn, "nav-btn");
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
       CAN update timers at ~30 fps
       ========================================================== */
    g_timeout_add(33, G_SOURCE_FUNC(update_bse_bar),      ui.bseBar);
    g_timeout_add(33, G_SOURCE_FUNC(update_apps_bar),     ui.appsBar);
    g_timeout_add(33, G_SOURCE_FUNC(update_speed),        ui.speedVal);
    g_timeout_add(33, G_SOURCE_FUNC(update_logger_title), ui.loggerTitle);
    g_timeout_add(33, G_SOURCE_FUNC(update_logger_stat),  ui.loggerStat);
    g_timeout_add(33, G_SOURCE_FUNC(update_bms_max),      ui.bmsMax);
    g_timeout_add(33, G_SOURCE_FUNC(update_bms_avg),      ui.bmsAvg);
    g_timeout_add(33, G_SOURCE_FUNC(update_vcu_faults),   ui.vcuFaults);
    g_timeout_add(33, G_SOURCE_FUNC(update_mtr_temp),     ui.mtrTemp);
    g_timeout_add(33, G_SOURCE_FUNC(update_inv_temp),     ui.invTemp);

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
