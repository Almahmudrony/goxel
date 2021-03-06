/* Goxel 3D voxels editor
 *
 * copyright (c) 2017 Guillaume Chereau <guillaume@noctua-software.com>
 *
 * Goxel is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.

 * Goxel is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.

 * You should have received a copy of the GNU General Public License along with
 * goxel.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "goxel.h"
#include "ini.h"

theme_group_info_t THEME_GROUP_INFOS[THEME_GROUP_COUNT] = {
    [THEME_GROUP_BASE] = {
        .name = "base",
        .colors = {
            [THEME_COLOR_BACKGROUND] = true,
            [THEME_COLOR_OUTLINE] = true,
            [THEME_COLOR_INNER] = true,
            [THEME_COLOR_INNER_SELECTED] = true,
            [THEME_COLOR_TEXT] = true,
            [THEME_COLOR_TEXT_SELECTED] = true,
        },
    },
    [THEME_GROUP_WIDGET] = {
        .name = "widget",
        .parent = THEME_GROUP_BASE,
        .colors = {
            [THEME_COLOR_OUTLINE] = true,
            [THEME_COLOR_INNER] = true,
            [THEME_COLOR_INNER_SELECTED] = true,
            [THEME_COLOR_TEXT] = true,
            [THEME_COLOR_TEXT_SELECTED] = true,
        },
    },
    [THEME_GROUP_TAB] = {
        .name = "tab",
        .parent = THEME_GROUP_BASE,
        .colors = {
            [THEME_COLOR_BACKGROUND] = true,
            [THEME_COLOR_INNER] = true,
            [THEME_COLOR_INNER_SELECTED] = true,
            [THEME_COLOR_TEXT] = true,
            [THEME_COLOR_TEXT_SELECTED] = true,
        },
    },
    [THEME_GROUP_MENU] = {
        .name = "menu",
        .parent = THEME_GROUP_BASE,
        .colors = {
            [THEME_COLOR_BACKGROUND] = true,
            [THEME_COLOR_INNER] = true,
            [THEME_COLOR_TEXT] = true,
        },
    },
};

theme_color_info_t THEME_COLOR_INFOS[THEME_COLOR_COUNT] = {
    [THEME_COLOR_BACKGROUND] = {.name = "background"},
    [THEME_COLOR_OUTLINE] = {.name = "outline"},
    [THEME_COLOR_INNER] = {.name = "inner"},
    [THEME_COLOR_INNER_SELECTED] = {.name = "inner_selected"},
    [THEME_COLOR_TEXT] = {.name = "text"},
    [THEME_COLOR_TEXT_SELECTED] = {.name = "text_selected"},
};

static theme_t g_default_theme = {
    .name = "default",
    .sizes = {
        .item_height = 18,
        .icons_height = 32,
        .item_padding_h = 4,
        .item_rounding = 4,
        .item_spacing_h = 4,
        .item_spacing_v = 4,
        .item_inner_spacing_h = 6,
    },

    .groups = {
        [THEME_GROUP_BASE] = {
            .colors = {
                [THEME_COLOR_BACKGROUND] = VEC4(96, 114, 114, 255),
                [THEME_COLOR_OUTLINE] = VEC4(77, 77, 77, 255),
                [THEME_COLOR_INNER] = VEC4(161, 161, 161, 255),
                [THEME_COLOR_TEXT] = VEC4(17, 17, 17, 255),
                [THEME_COLOR_INNER_SELECTED] = VEC4(102, 102, 204, 255),
                [THEME_COLOR_TEXT_SELECTED] = VEC4(17, 17, 17, 255),
            },
        },
        [THEME_GROUP_TAB] = {
            .colors = {
                [THEME_COLOR_BACKGROUND] = VEC4(48, 66, 66, 255),
                [THEME_COLOR_INNER] = VEC4(69, 86, 86, 255),
                [THEME_COLOR_INNER_SELECTED] = VEC4(96, 114, 114, 255),
            },
        },
        [THEME_GROUP_MENU] = {
            .colors = {
                [THEME_COLOR_INNER] = VEC4(90, 110, 110, 255),
            },
        },
    },
};

static theme_t g_theme = {};

static uint32_t tohex(uvec4b_t c)
{
    return (int)c.r << 24 |
           (int)c.g << 16 |
           (int)c.b <<  8 |
           (int)c.a <<  0;
}

static uvec4b_t parse_color(const char *s)
{
    uint32_t v;
    uvec4b_t ret;
    v = strtol(s + 1, NULL, 16);
    ret.r = (v >> 24) & 0xff;
    ret.g = (v >> 16) & 0xff;
    ret.b = (v >>  8) & 0xff;
    ret.a = (v >>  0) & 0xff;
    return ret;
}

static int theme_ini_handler(void *user, const char *section,
                             const char *name, const char *value, int lineno)
{
    int group, i;
    if (strcmp(section, "sizes") == 0) {
        #define X(n) \
        if (strcmp(name, #n) == 0) g_theme.sizes.n = atoi(value);
        THEME_SIZES(X)
        #undef X
    }
    for (group = 0; group < THEME_GROUP_COUNT; group++) {
        if (strcmp(section, THEME_GROUP_INFOS[group].name) != 0) continue;
        for (i = 0; i < THEME_COLOR_COUNT; i++) {
            if (strcmp(name, THEME_COLOR_INFOS[i].name) == 0) {
                g_theme.groups[group].colors[i] = parse_color(value);
            }
        }
    }

    return 0;
}

void theme_set(const char *name)
{
    char *path;
    FILE *file;

    name = name ?: "default";
    asprintf(&path, "%s/themes/%s.ini", sys_get_user_dir(), name);
    file = fopen(path, "r");
    free(path);
    g_theme = g_default_theme;

    if (file) {
        ini_parse_file(file, theme_ini_handler, NULL);
        fclose(file);
    }
}

theme_t *theme_get(void)
{
    if (!(*g_theme.name)) theme_set(NULL);
    return &g_theme;
}

void theme_revert_default(void)
{
    g_theme = g_default_theme;
}

void theme_save(void)
{
    char *path;
    FILE *file;
    const theme_t *t = &g_theme;
    int group, i;
    asprintf(&path, "%s/themes/default.ini", sys_get_user_dir());
    sys_make_dir(path);
    file = fopen(path, "w");

    fprintf(file, "[sizes]\n");
    #define X(n) \
    fprintf(file, #n "=%d\n", t->sizes.n);
    THEME_SIZES(X)
    #undef X

    for (group = 0; group < THEME_GROUP_COUNT; group++) {
        fprintf(file, "\n");
        fprintf(file, "[%s]\n", THEME_GROUP_INFOS[group].name);
        for (i = 0; i < THEME_COLOR_COUNT; i++) {
            if (!t->groups[group].colors[i].a) continue;
            fprintf(file, "%s=#%X\n", THEME_COLOR_INFOS[i].name, tohex(
                    t->groups[group].colors[i]));
        }
    }

    fclose(file);
    free(path);
}

uvec4b_t theme_get_color(int g, int color, bool sel)
{
    const theme_t *theme = theme_get();
    if (sel) color++;
    while (g && !theme->groups[g].colors[color].a)
        g = THEME_GROUP_INFOS[g].parent;
    return theme->groups[g].colors[color];
}
