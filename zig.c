#include <yed/plugin.h>
#include <yed/highlight.h>

#define ARRAY_LOOP(a)                             \
    for (__typeof((a)[0])* it = (a);              \
         it < (a) + (sizeof(a) / sizeof((a)[0])); \
         ++it)

highlight_info hinfo;

void
unload(yed_plugin* self);
void
syntax_zig_line_handler(yed_event* event);
void
syntax_zig_frame_handler(yed_event* event);
void
syntax_zig_buff_mod_pre_handler(yed_event* event);
void
syntax_zig_buff_mod_post_handler(yed_event* event);

int
yed_plugin_boot(yed_plugin* self)
{
    yed_event_handler frame, line, buff_mod_pre, buff_mod_post;

    char*             kwds[] = {
        "asm",
        "comptime",
        "enum",
        "export",
        "extern",
        "fn",
        "pub",
        "struct",
        "test",
        "union",
        "usingnamespace",
        "volatile",
        "var",
        "const",

    };
    char* special_kwds[] = {
        "defer",     "callconv",  "linksection", "catch",       "resume",
        "try",       "async",     "await",       "suspend",     "inline",
        "noalias",   "nosuspend", "noinline",    "opaque",      "packed",
        "errdefer",  "error",     "threadlocal", "unreachable", "align",
        "allowzero", "anyframe",
    };
    char* control_flow[] = {
        "continue",
        "break",
        "for",
        "if",
        "else",
        "or",
        "orelse",
        "switch",
        "and",
        "while",
        "return",
    };
    char* typenames[] = {
        "i8",         "u8",           "i16",
        "u16",        "i32",          "u32",
        "i64",        "u64",          "i128",
        "u128",       "isize",        "usize",
        "c_short",    "c_ushort",     "c_int",
        "c_uint",     "c_long",       "c_ulong",
        "c_longlong", "c_ulonglong",  "c_longdouble",
        "c_void",     "f16",          "f32",
        "f64",        "f128",         "bool",
        "void",       "noreturn",     "type",
        "anyerror",   "comptime_int", "comptime_float",
        "anytype",
    };

    YED_PLUG_VERSION_CHECK();

    yed_plugin_set_unload_fn(self, unload);

    frame.kind         = EVENT_FRAME_PRE_BUFF_DRAW;
    frame.fn           = syntax_zig_frame_handler;
    line.kind          = EVENT_LINE_PRE_DRAW;
    line.fn            = syntax_zig_line_handler;
    buff_mod_pre.kind  = EVENT_BUFFER_PRE_MOD;
    buff_mod_pre.fn    = syntax_zig_buff_mod_pre_handler;
    buff_mod_post.kind = EVENT_BUFFER_POST_MOD;
    buff_mod_post.fn   = syntax_zig_buff_mod_post_handler;

    yed_plugin_add_event_handler(self, frame);
    yed_plugin_add_event_handler(self, line);
    yed_plugin_add_event_handler(self, buff_mod_pre);
    yed_plugin_add_event_handler(self, buff_mod_post);

    highlight_info_make(&hinfo);

    ARRAY_LOOP(kwds)
    highlight_add_kwd(&hinfo, *it, HL_KEY);
    ARRAY_LOOP(special_kwds)
    highlight_add_kwd(&hinfo, *it, HL_PP);
    ARRAY_LOOP(control_flow)
    highlight_add_kwd(&hinfo, *it, HL_CF);
    ARRAY_LOOP(typenames)
    highlight_add_kwd(&hinfo, *it, HL_TYPE);
    highlight_add_kwd(&hinfo, "null", HL_CON);
    highlight_add_kwd(&hinfo, "undefined", HL_CON);
    highlight_add_kwd(&hinfo, "true", HL_CON);
    highlight_add_kwd(&hinfo, "false", HL_CON);
    highlight_add_kwd(&hinfo, "stdin", HL_CON);
    highlight_add_kwd(&hinfo, "stdout", HL_CON);
    highlight_add_kwd(&hinfo, "stderr", HL_CON);
    highlight_prefixed_words(&hinfo, '@', HL_PP);
    highlight_suffixed_words(&hinfo, '(', HL_CALL);
    highlight_numbers(&hinfo);
    highlight_within(&hinfo, "\"", "\"", '\\', -1, HL_STR);
    highlight_within(&hinfo, "'", "'", '\\', 1, HL_CHAR);
    highlight_to_eol_from(&hinfo, "//", HL_COMMENT);
    highlight_within_multiline(&hinfo, "/*", "*/", 0, HL_COMMENT);

    ys->redraw = 1;

    return 0;
}

void
unload(yed_plugin* self)
{
    highlight_info_free(&hinfo);
    ys->redraw = 1;
}

void
syntax_zig_frame_handler(yed_event* event)
{
    yed_frame* frame;

    frame = event->frame;

    if (!frame || !frame->buffer || frame->buffer->kind != BUFF_KIND_FILE ||
        frame->buffer->ft != yed_get_ft("Zig"))
    {
        return;
    }

    highlight_frame_pre_draw_update(&hinfo, event);
}

void
syntax_zig_line_handler(yed_event* event)
{
    yed_frame* frame;

    frame = event->frame;

    if (!frame || !frame->buffer || frame->buffer->kind != BUFF_KIND_FILE ||
        frame->buffer->ft != yed_get_ft("Zig"))
    {
        return;
    }

    highlight_line(&hinfo, event);
}

void
syntax_zig_buff_mod_pre_handler(yed_event* event)
{
    if (event->buffer == NULL || event->buffer->kind != BUFF_KIND_FILE ||
        event->buffer->ft != yed_get_ft("Zig"))
    {
        return;
    }

    highlight_buffer_pre_mod_update(&hinfo, event);
}

void
syntax_zig_buff_mod_post_handler(yed_event* event)
{
    if (event->buffer == NULL || event->buffer->kind != BUFF_KIND_FILE ||
        event->buffer->ft != yed_get_ft("Zig"))
    {
        return;
    }

    highlight_buffer_post_mod_update(&hinfo, event);
}
