/**
 * @file window.c
 * @brief X C Bindings window provider module for VLC media player
 */
/*****************************************************************************
 * Copyright © 2009 Rémi Denis-Courmont
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2.0
 * of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 ****************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdarg.h>
#include <assert.h>
#include <poll.h>

#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>

#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_window.h>

#include "xcb_vlc.h"

#define DISPLAY_TEXT N_("X11 display")
#define DISPLAY_LONGTEXT N_( \
    "X11 hardware display to use. By default VLC will " \
    "use the value of the DISPLAY environment variable.")

static int  Open (vlc_object_t *);
static void Close (vlc_object_t *);

/*
 * Module descriptor
 */
vlc_module_begin ()
    set_shortname (N_("XCB window"))
    set_description (N_("(Experimental) XCB video window"))
    set_category (CAT_VIDEO)
    set_subcategory (SUBCAT_VIDEO_VOUT)
    set_capability ("xwindow", 10)
    set_callbacks (Open, Close)

    add_string ("x11-display", NULL, NULL,
                DISPLAY_TEXT, DISPLAY_LONGTEXT, true)
vlc_module_end ()

static int Control (vout_window_t *, int, va_list ap);
static void *Thread (void *);

struct vout_window_sys_t
{
    xcb_connection_t *conn;
    key_handler_t *keys;
    vlc_thread_t thread;
};

/**
 * Create an X11 window.
 */
static int Open (vlc_object_t *obj)
{
    vout_window_t *wnd = (vout_window_t *)obj;
    vout_window_sys_t *p_sys = malloc (sizeof (*p_sys));
    xcb_generic_error_t *err;
    xcb_void_cookie_t ck;

    if (p_sys == NULL)
        return VLC_ENOMEM;

    /* Connect to X */
    char *display = var_CreateGetNonEmptyString (wnd, "x11-display");
    int snum;

    xcb_connection_t *conn = xcb_connect (display, &snum);
    free (display);
    if (xcb_connection_has_error (conn) /*== NULL*/)
        goto error;

    /* Create window */
    xcb_screen_t *scr = xcb_aux_get_screen (conn, snum);
    const uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    uint32_t values[2] = {
        /* XCB_CW_BACK_PIXEL */
        scr->black_pixel,
        /* XCB_CW_EVENT_MASK */
        XCB_EVENT_MASK_KEY_PRESS,
    };

    xcb_window_t window = xcb_generate_id (conn);
    ck = xcb_create_window_checked (conn, scr->root_depth, window, scr->root,
                                    0, 0, wnd->width, wnd->height, 0,
                                    XCB_WINDOW_CLASS_INPUT_OUTPUT,
                                    scr->root_visual, mask, values);
    err = xcb_request_check (conn, ck);
    if (err)
    {
        msg_Err (wnd, "creating window: X11 error %d", err->error_code);
        goto error;
    }

    wnd->handle.xid = window;
    wnd->p_sys = p_sys;
    wnd->control = Control;

    p_sys->conn = conn;
    p_sys->keys = CreateKeyHandler (obj, conn);

    if ((p_sys->keys != NULL)
     && vlc_clone (&p_sys->thread, Thread, wnd, VLC_THREAD_PRIORITY_LOW))
        DestroyKeyHandler (p_sys->keys);

    /* Make sure the window is ready */
    xcb_map_window (conn, window);
    xcb_flush (conn);

    return VLC_SUCCESS;

error:
    xcb_disconnect (conn);
    free (p_sys);
    return VLC_EGENERIC;
}


/**
 * Destroys the X11 window.
 */
static void Close (vlc_object_t *obj)
{
    vout_window_t *wnd = (vout_window_t *)obj;
    vout_window_sys_t *p_sys = wnd->p_sys;
    xcb_connection_t *conn = p_sys->conn;
    xcb_window_t window = wnd->handle.xid;

    if (p_sys->keys)
    {
        vlc_cancel (p_sys->thread);
        vlc_join (p_sys->thread, NULL);
        DestroyKeyHandler (p_sys->keys);
    }
    xcb_unmap_window (conn, window);
    xcb_destroy_window (conn, window);
    xcb_disconnect (conn);
    free (p_sys);
}


static void *Thread (void *data)
{
    vout_window_t *wnd = data;
    vout_window_sys_t *p_sys = wnd->p_sys;
    xcb_connection_t *conn = p_sys->conn;

    int fd = xcb_get_file_descriptor (conn);
    if (fd == -1)
        return NULL;

    for (;;)
    {
        xcb_generic_event_t *ev;
        struct pollfd ufd = { .fd = fd, .events = POLLIN, };

        poll (&ufd, 1, -1);

        int canc = vlc_savecancel ();
        while ((ev = xcb_poll_for_event (conn)) != NULL)
        {
            if (ProcessKeyEvent (p_sys->keys, ev) == 0)
                continue;
            free (ev);
        }
        vlc_restorecancel (canc);

        if (xcb_connection_has_error (conn))
        {
            msg_Err (wnd, "X server failure");
            break;
        }
    }
    return NULL;
}


static int Control (vout_window_t *wnd, int cmd, va_list ap)
{
    msg_Err (wnd, "request %d not implemented", cmd);
    (void)ap;
    return VLC_EGENERIC;
}
