#include "gm_plugin_lvgl_api.h"
#include "gm_plugin_libc.h"

/* Paired with PhoneSDK/examples/recipe. The phone sends the whole recipe; this
 * plugin walks the wearer through the steps hands-free: head up/down changes
 * step, and the primary button starts/pauses the per-step timer. When a timer
 * ends the step is announced and the phone is told so it can ring. */

#define RECIPE_CHANNEL UINT16_C(0x5243) /* 'RC': phone -> glasses */
#define EVENT_CHANNEL UINT16_C(0x5241)  /* 'RA': glasses -> phone */
#define PROTOCOL_VERSION 1U

#define COMMAND_RECIPE 1U
#define COMMAND_END 2U

#define EVENT_TIMER_DONE UINT8_C(1)
#define EVENT_STEP UINT8_C(2)

#define MAX_STEPS 20U
#define MAX_STEP_BYTES 160U
#define MAX_RECIPE_NAME_BYTES 40U

#define SCREEN_MARGIN 20
#define HINT_HEIGHT 28
#define TITLE_HEIGHT 34

#define DONE_HOLD_MS 3000U

typedef struct {
    char text[MAX_STEP_BYTES + 1U];
    uint16_t minutes;
} recipe_step_t;

typedef struct {
    const gm_plugin_host_api_t *host;
    const gm_plugin_lvgl_api_t *ui;
    const gm_plugin_libc_extension_api_t *libc;
    gm_plugin_lvgl_obj_t *screen;
    gm_plugin_lvgl_obj_t *title_label;
    gm_plugin_lvgl_obj_t *step_label;
    gm_plugin_lvgl_obj_t *timer_label;
    gm_plugin_lvgl_obj_t *hint_label;
    char recipe_name[MAX_RECIPE_NAME_BYTES + 1U];
    recipe_step_t steps[MAX_STEPS];
    uint16_t step_count;
    uint16_t current;
    uint16_t remaining_sec;
    bool timer_running;
    bool has_recipe;
    bool timer_done;         /* showing the "done" hold */
    uint32_t done_elapsed_ms;
    uint8_t language;        /* 0=fr 1=en 2=es 3=it 4=de 5=zh-CN */
} recipe_t;

/* Picks the string for the detected UI locale. */
#define L(fr_str, en_str, es_str, it_str, de_str, zh_str) \
    (recipe.language == 1U ? (en_str) : recipe.language == 2U ? (es_str) : \
     recipe.language == 3U ? (it_str) : recipe.language == 4U ? (de_str) : \
     recipe.language == 5U ? (zh_str) : (fr_str))

static recipe_t recipe;

static uint8_t recipe_detect_language(const char *locale)
{
    if (locale[0] == 'e' && locale[1] == 'n') return 1U;
    if (locale[0] == 'e' && locale[1] == 's') return 2U;
    if (locale[0] == 'i' && locale[1] == 't') return 3U;
    if (locale[0] == 'd' && locale[1] == 'e') return 4U;
    if (locale[0] == 'z' && locale[1] == 'h') return 5U;
    return 0U;
}

#define number gm_plugin_lvgl_style_number
#define color gm_plugin_lvgl_style_color

static void set_style(gm_plugin_lvgl_obj_t *object,
                       gm_plugin_lvgl_style_prop_t property,
                       gm_plugin_lvgl_style_value_t value)
{
    recipe.ui->style_set(object, property, value, GM_PLUGIN_LVGL_SELECTOR_MAIN);
}

static void style_panel(gm_plugin_lvgl_obj_t *object, uint8_t fill,
                         uint8_t border, uint8_t radius)
{
    set_style(object, GM_PLUGIN_LVGL_STYLE_BG_COLOR, color(fill));
    set_style(object, GM_PLUGIN_LVGL_STYLE_BG_OPA, number(255));
    set_style(object, GM_PLUGIN_LVGL_STYLE_BORDER_COLOR, color(0xA0));
    set_style(object, GM_PLUGIN_LVGL_STYLE_BORDER_OPA, number(255));
    set_style(object, GM_PLUGIN_LVGL_STYLE_BORDER_WIDTH, number(border));
    set_style(object, GM_PLUGIN_LVGL_STYLE_RADIUS, number(radius));
    recipe.ui->obj_clear_flag(object, GM_PLUGIN_LVGL_FLAG_SCROLLABLE);
}

static gm_plugin_lvgl_obj_t *make_label(gm_plugin_lvgl_obj_t *parent,
                                         const char *text, uint8_t shade)
{
    gm_plugin_lvgl_obj_t *label = recipe.ui->label_create(parent);
    if (label == 0) return 0;
    recipe.ui->label_set_text(label, text);
    recipe.ui->label_set_long_mode(label, GM_PLUGIN_LVGL_LABEL_WRAP);
    set_style(label, GM_PLUGIN_LVGL_STYLE_TEXT_COLOR, color(shade));
    set_style(label, GM_PLUGIN_LVGL_STYLE_TEXT_ALIGN,
              number(GM_PLUGIN_LVGL_TEXT_ALIGN_CENTER));
    return label;
}

static uint32_t bounded_len(const char *text, uint32_t max)
{
    uint32_t len = 0U;
    while (len < max && text[len] != '\0') len++;
    return len;
}

/* Writes the current step line ("Etape 2/5") into buffer without printf.
 * Kept out of line: inlining lets GCC fold the prefix literal copy into a
 * direct memcpy() call, which is not available in this freestanding target. */
static void format_step_line(char *buffer, uint32_t size) __attribute__((noinline));

static void format_step_line(char *buffer, uint32_t size)
{
    const char *prefix = L("Etape ", "Step ", "Paso ", "Passo ", "Schritt ",
                           "步骤 ");
    char number_buf[8];
    char total_buf[8];
    uint32_t i = 0U;
    uint32_t n = 0U;
    uint16_t value = (uint16_t)(recipe.current + 1U);
    uint16_t total = recipe.step_count;
    uint32_t pos = 0U;
    uint32_t p = 0U;

    if (value == 0U) number_buf[i++] = '0';
    while (value > 0U && i < sizeof(number_buf) - 1U) {
        number_buf[i++] = (char)('0' + (value % 10U));
        value /= 10U;
    }
    if (total == 0U) total_buf[n++] = '0';
    while (total > 0U && n < sizeof(total_buf) - 1U) {
        total_buf[n++] = (char)('0' + (total % 10U));
        total /= 10U;
    }
    while (pos < size - 1U && prefix[p] != '\0') buffer[pos++] = prefix[p++];
    while (i > 0U && pos < size - 1U) buffer[pos++] = number_buf[--i];
    if (pos < size - 1U) buffer[pos++] = '/';
    while (n > 0U && pos < size - 1U) buffer[pos++] = total_buf[--n];
    buffer[pos] = '\0';
}

static void format_timer(char *buffer, uint32_t size)
{
    uint16_t secs = recipe.remaining_sec;
    uint16_t mins = secs / 60U;
    uint16_t rem = secs % 60U;
    uint32_t pos = 0U;

    if (mins >= 10U) {
        if (pos < size - 1U) buffer[pos++] = (char)('0' + (mins / 10U));
    }
    if (pos < size - 1U) buffer[pos++] = (char)('0' + (mins % 10U));
    if (pos < size - 1U) buffer[pos++] = ':';
    if (pos < size - 1U) buffer[pos++] = (char)('0' + (rem / 10U));
    if (pos < size - 1U) buffer[pos++] = (char)('0' + (rem % 10U));
    buffer[pos] = '\0';
}

static void refresh_display(void)
{
    char title[48];
    char step_line[24];
    char timer_text[16];
    char combined[MAX_STEP_BYTES + 32U];
    uint32_t tlen;
    uint32_t slen;
    uint32_t cpos = 0U;

    if (!recipe.has_recipe) {
        recipe.ui->label_set_text(recipe.title_label,
            L("Recette", "Recipe", "Receta", "Ricetta", "Rezept", "食谱"));
        recipe.ui->label_set_text(recipe.step_label,
            L("En attente du telephone...", "Waiting for the phone...", "Esperando al teléfono...", "In attesa del telefono...", "Warte auf das Telefon...", "正在等待手机..."));
        recipe.ui->label_set_text(recipe.timer_label, "");
        recipe.ui->label_set_text(recipe.hint_label, "");
        return;
    }

    if (recipe.timer_done) {
        recipe.ui->label_set_text(recipe.title_label, recipe.recipe_name);
        recipe.ui->label_set_text(recipe.step_label,
            L("Minuteur termine !", "Timer done!", "¡Tiempo terminado!", "Tempo scaduto!", "Timer abgelaufen!", "计时结束！"));
        recipe.ui->label_set_text(recipe.timer_label, "");
        recipe.ui->label_set_text(recipe.hint_label, "");
        return;
    }

    tlen = bounded_len(recipe.recipe_name, sizeof(title) - 1U);
    recipe.libc->memcpy(title, recipe.recipe_name, tlen);
    title[tlen] = '\0';
    recipe.ui->label_set_text(recipe.title_label, title);

    format_step_line(step_line, sizeof(step_line));
    slen = bounded_len(step_line, sizeof(step_line) - 1U);
    recipe.libc->memcpy(combined, step_line, slen);
    cpos = slen;
    combined[cpos++] = '\n';
    {
        uint32_t step_text_len =
            bounded_len(recipe.steps[recipe.current].text,
                        (uint32_t)(sizeof(combined) - cpos - 1U));
        recipe.libc->memcpy(&combined[cpos], recipe.steps[recipe.current].text,
                            step_text_len);
        cpos += step_text_len;
    }
    combined[cpos] = '\0';
    recipe.ui->label_set_text(recipe.step_label, combined);

    if (recipe.steps[recipe.current].minutes > 0U) {
        format_timer(timer_text, sizeof(timer_text));
        recipe.ui->label_set_text(recipe.timer_label, timer_text);
    } else {
        recipe.ui->label_set_text(recipe.timer_label, "");
    }

    recipe.ui->label_set_text(recipe.hint_label,
        recipe.steps[recipe.current].minutes > 0U
            ? L("Tete: etape   Bouton: minuteur",
                "Head: step   Button: timer",
                "Cabeza: paso   Botón: temporizador",
                "Testa: passo   Pulsante: timer",
                "Kopf: Schritt   Taste: Timer",
                "抬头/低头：步骤   按钮：计时")
            : L("Tete haut/bas: etape",
                "Head up/down: step",
                "Cabeza arriba/abajo: paso",
                "Testa su/giù: passo",
                "Kopf hoch/runter: Schritt",
                "抬头/低头：切换步骤"));
}

static void send_event(uint8_t event, uint16_t index)
{
    uint8_t payload[4];
    payload[0] = PROTOCOL_VERSION;
    payload[1] = event;
    payload[2] = (uint8_t)(index & 0xFFU);
    payload[3] = (uint8_t)((index >> 8) & 0xFFU);
    (void)recipe.host->bt_send(EVENT_CHANNEL, payload, sizeof(payload));
}

static void load_step(uint16_t index, bool announce)
{
    if (index >= recipe.step_count) return;
    recipe.current = index;
    recipe.timer_running = false;
    recipe.timer_done = false;
    recipe.remaining_sec = (uint16_t)(recipe.steps[index].minutes * 60U);
    if (announce) send_event(EVENT_STEP, index);
    refresh_display();
}

static void move_step(int32_t delta)
{
    int32_t next = (int32_t)recipe.current + delta;
    if (next < 0) next = 0;
    if (next >= (int32_t)recipe.step_count) next = (int32_t)recipe.step_count - 1;
    load_step((uint16_t)next, true);
}

static void toggle_timer(void)
{
    if (recipe.steps[recipe.current].minutes == 0U) return;
    if (recipe.remaining_sec == 0U) recipe.remaining_sec = (uint16_t)(recipe.steps[recipe.current].minutes * 60U);
    recipe.timer_running = !recipe.timer_running;
    refresh_display();
}

static gm_plugin_result_t recipe_start(void *context)
{
    gm_plugin_display_info_t display;
    gm_plugin_lvgl_obj_t *root;
    char locale[GM_PLUGIN_LOCALE_TAG_MAX];
    (void)context;

    recipe.language = 0U;
    recipe.has_recipe = false;
    recipe.timer_running = false;
    recipe.timer_done = false;

    if (recipe.host->locale_get != 0 &&
        recipe.host->locale_get(locale) == GM_PLUGIN_OK)
        recipe.language = recipe_detect_language(locale);

    if (recipe.host->display_get_info(&display) != GM_PLUGIN_OK ||
        display.width <= SCREEN_MARGIN * 2U)
        return GM_PLUGIN_ESTATE;

    root = recipe.ui->root_get();
    if (root == 0) return GM_PLUGIN_ESTATE;
    recipe.ui->obj_clean(root);

    recipe.screen = recipe.ui->obj_create(root);
    if (recipe.screen == 0) return GM_PLUGIN_ENOMEM;
    recipe.ui->obj_set_size(recipe.screen, display.width, display.height);
    recipe.ui->obj_align(recipe.screen, GM_PLUGIN_LVGL_ALIGN_CENTER, 0, 0);
    style_panel(recipe.screen, 0x00, 0, 0);

    recipe.title_label = make_label(recipe.screen, "", 0xD0);
    if (recipe.title_label == 0) goto no_memory;
    recipe.ui->obj_set_size(recipe.title_label, display.width - SCREEN_MARGIN * 2U, TITLE_HEIGHT);
    recipe.ui->obj_align(recipe.title_label, GM_PLUGIN_LVGL_ALIGN_TOP_MID, 0, 6);

    recipe.step_label = make_label(recipe.screen, "", 0xF0);
    if (recipe.step_label == 0) goto no_memory;
    recipe.ui->obj_set_size(recipe.step_label, display.width - SCREEN_MARGIN * 2U,
                             display.height - TITLE_HEIGHT - HINT_HEIGHT - 48);
    recipe.ui->obj_align(recipe.step_label, GM_PLUGIN_LVGL_ALIGN_CENTER, 0, 0);

    recipe.timer_label = make_label(recipe.screen, "", 0xFF);
    if (recipe.timer_label == 0) goto no_memory;
    {
        gm_plugin_lvgl_style_value_t font_value = {0};
        font_value.ptr = recipe.ui->font_large;
        set_style(recipe.timer_label, GM_PLUGIN_LVGL_STYLE_TEXT_FONT, font_value);
    }
    recipe.ui->obj_set_size(recipe.timer_label, display.width - SCREEN_MARGIN * 2U, 48);
    recipe.ui->obj_align(recipe.timer_label, GM_PLUGIN_LVGL_ALIGN_BOTTOM_MID, 0, HINT_HEIGHT + 8);

    recipe.hint_label = make_label(recipe.screen, "", 0x70);
    if (recipe.hint_label == 0) goto no_memory;
    recipe.ui->obj_set_size(recipe.hint_label, display.width - SCREEN_MARGIN * 2U, HINT_HEIGHT);
    recipe.ui->obj_align(recipe.hint_label, GM_PLUGIN_LVGL_ALIGN_BOTTOM_MID, 0, -6);

    refresh_display();

    if (recipe.host->imu_enable(GM_PLUGIN_IMU_ENABLE_GESTURES) != GM_PLUGIN_OK)
        goto no_memory;
    return GM_PLUGIN_OK;

no_memory:
    recipe.ui->obj_clean(root);
    recipe.screen = 0;
    recipe.title_label = 0;
    recipe.step_label = 0;
    recipe.timer_label = 0;
    recipe.hint_label = 0;
    return GM_PLUGIN_ENOMEM;
}

static uint16_t read_u16(const uint8_t *data)
{
    return (uint16_t)((uint16_t)data[0] | ((uint16_t)data[1] << 8));
}

static void handle_recipe_message(const uint8_t *data, uint32_t length)
{
    uint32_t offset;
    uint8_t name_len;
    uint16_t count;
    uint16_t i;

    if (length < 3U || data[0] != PROTOCOL_VERSION) return;

    if (data[1] == COMMAND_END) {
        recipe.has_recipe = false;
        recipe.timer_running = false;
        recipe.timer_done = false;
        recipe.step_count = 0U;
        refresh_display();
        return;
    }
    if (data[1] != COMMAND_RECIPE || length < 5U) return;

    name_len = data[2];
    offset = 3U;
    if (name_len > MAX_RECIPE_NAME_BYTES || offset + name_len > length) return;
    recipe.libc->memcpy(recipe.recipe_name, &data[offset], name_len);
    recipe.recipe_name[name_len] = '\0';
    offset += name_len;

    if (offset + 2U > length) return;
    count = read_u16(&data[offset]);
    offset += 2U;
    if (count > MAX_STEPS) count = MAX_STEPS;

    for (i = 0U; i < count; i++) {
        uint8_t step_len;
        uint16_t minutes;
        if (offset + 4U > length) break;
        minutes = read_u16(&data[offset]);
        step_len = data[offset + 2U];
        offset += 3U;
        if (step_len > MAX_STEP_BYTES || offset + step_len > length) break;
        recipe.libc->memcpy(recipe.steps[i].text, &data[offset], step_len);
        recipe.steps[i].text[step_len] = '\0';
        recipe.steps[i].minutes = minutes;
        offset += step_len;
    }
    recipe.step_count = i;
    recipe.has_recipe = i > 0U;
    if (recipe.has_recipe) {
        load_step(0U, false);
    } else {
        refresh_display();
    }
}

static bool recipe_event(void *context, const gm_plugin_event_t *event)
{
    (void)context;
    if (event == 0) return false;

    if (event->type == GM_PLUGIN_EVENT_CONNECTION) {
        recipe.host->log("recipe: phone connected=%u",
                          (unsigned int)event->data.connection.connected);
        return true;
    }

    if (event->type == GM_PLUGIN_EVENT_BT_MESSAGE) {
        if (event->data.bt.channel != RECIPE_CHANNEL || event->data.bt.data == 0)
            return false;
        handle_recipe_message(event->data.bt.data, event->data.bt.length);
        return true;
    }

    if (!recipe.has_recipe || recipe.timer_done) return false;

    if (event->type == GM_PLUGIN_EVENT_BUTTON &&
        event->data.button.button == GM_PLUGIN_BUTTON_PRIMARY &&
        event->data.button.action == GM_PLUGIN_BUTTON_ACTION_SINGLE) {
        toggle_timer();
        return true;
    }

    if (event->type == GM_PLUGIN_EVENT_IMU_GESTURE &&
        event->data.imu_gesture.active) {
        switch (event->data.imu_gesture.gesture) {
        case GM_PLUGIN_IMU_GESTURE_HEAD_RAISE:
            move_step(-1);
            return true;
        case GM_PLUGIN_IMU_GESTURE_HEAD_LOWER:
            move_step(1);
            return true;
        default:
            break;
        }
    }
    return false;
}

static void recipe_loop(void *context, uint32_t elapsed_ms)
{
    uint32_t seconds;
    (void)context;

    if (recipe.timer_done) {
        recipe.done_elapsed_ms += elapsed_ms;
        if (recipe.done_elapsed_ms >= DONE_HOLD_MS) {
            recipe.timer_done = false;
            refresh_display();
        }
        return;
    }

    if (!recipe.has_recipe || !recipe.timer_running) return;

    seconds = elapsed_ms / 1000U;
    if (seconds == 0U) return;
    if (seconds >= recipe.remaining_sec) {
        recipe.remaining_sec = 0U;
        recipe.timer_running = false;
        recipe.timer_done = true;
        recipe.done_elapsed_ms = 0U;
        send_event(EVENT_TIMER_DONE, recipe.current);
        refresh_display();
    } else {
        recipe.remaining_sec = (uint16_t)(recipe.remaining_sec - seconds);
        refresh_display();
    }
}

static void recipe_stop(void *context)
{
    gm_plugin_lvgl_obj_t *root;
    (void)context;
    root = recipe.ui->root_get();
    (void)recipe.host->imu_enable(GM_PLUGIN_IMU_ENABLE_NONE);
    if (root != 0) recipe.ui->obj_clean(root);
    recipe.screen = 0;
    recipe.title_label = 0;
    recipe.step_label = 0;
    recipe.timer_label = 0;
    recipe.hint_label = 0;
    recipe.has_recipe = false;
    recipe.timer_running = false;
    recipe.timer_done = false;
}

gm_plugin_result_t gm_plugin_entry(const gm_plugin_host_api_t *host,
                                    gm_plugin_descriptor_t *plugin)
{
    const gm_plugin_capabilities_t required =
        GM_PLUGIN_CAP_BLUETOOTH | GM_PLUGIN_CAP_BUTTON | GM_PLUGIN_CAP_IMU_EVENTS;

    if (host == 0 || plugin == 0 || host->log == 0 || host->bt_send == 0 ||
        host->display_get_info == 0 || host->graphics.lvgl == 0 ||
        host->imu_enable == 0 ||
        !GM_PLUGIN_VERSION_COMPATIBLE(host->abi_version,
                                      GM_PLUGIN_ABI_MIN_VERSION) ||
        host->struct_size < GM_PLUGIN_HOST_API_MIN_SIZE ||
        plugin->struct_size < GM_PLUGIN_DESCRIPTOR_MIN_SIZE ||
        (host->capabilities & required) != required)
        return GM_PLUGIN_ENOTSUP;

    recipe.host = host;
    recipe.ui = host->graphics.lvgl;
    if (gm_plugin_libc_get(host, &recipe.libc) != GM_PLUGIN_OK)
        return GM_PLUGIN_ENOTSUP;
    if (recipe.ui->struct_size < GM_PLUGIN_LVGL_API_MIN_SIZE ||
        !GM_PLUGIN_VERSION_COMPATIBLE(recipe.ui->api_version,
                                      GM_PLUGIN_LVGL_API_MIN_VERSION))
        return GM_PLUGIN_EVERSION;

    plugin->abi_version = GM_PLUGIN_ABI_MIN_VERSION;
    plugin->context = &recipe;
    plugin->on_start = recipe_start;
    plugin->on_event = recipe_event;
    plugin->on_loop = recipe_loop;
    plugin->on_stop = recipe_stop;
    return GM_PLUGIN_OK;
}
