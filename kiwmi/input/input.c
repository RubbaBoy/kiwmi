/* Copyright (c), Niclas Meyer <niclas@countingsort.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "input/input.h"

#include <wayland-server.h>
#include <wlr/backend.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/util/log.h>

#include "desktop/desktop.h"
#include "input/cursor.h"
#include "input/keyboard.h"
#include "server.h"

static void
new_pointer(struct kiwmi_input *input, struct wlr_input_device *device)
{
    wlr_cursor_attach_input_device(input->cursor->cursor, device);
}

static void
new_keyboard(struct kiwmi_input *input, struct wlr_input_device *device)
{
    struct kiwmi_server *server = wl_container_of(input, server, input);

    struct kiwmi_keyboard *keyboard = keyboard_create(server, device);
    if (!keyboard) {
        return;
    }

    wl_list_insert(&input->keyboards, &keyboard->link);
}

static void
new_input_notify(struct wl_listener *listener, void *data)
{
    struct kiwmi_input *input = wl_container_of(listener, input, new_input);
    struct wlr_input_device *device = data;

    wlr_log(WLR_DEBUG, "New input %p: %s", device, device->name);

    switch (device->type) {
    case WLR_INPUT_DEVICE_POINTER:
        new_pointer(input, device);
        break;
    case WLR_INPUT_DEVICE_KEYBOARD:
        new_keyboard(input, device);
        break;
    default:
        // NOT HANDLED
        break;
    }
}

bool
input_init(struct kiwmi_input *input)
{
    struct kiwmi_server *server = wl_container_of(input, server, input);

    input->cursor = cursor_create(server->desktop.output_layout);
    if (!input->cursor) {
        wlr_log(WLR_ERROR, "Failed to create cursor");
        return false;
    }

    wl_list_init(&input->keyboards);

    input->new_input.notify = new_input_notify;
    wl_signal_add(&server->backend->events.new_input, &input->new_input);

    return true;
}
