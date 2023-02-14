/**
 * Mappings: (actual input => OG print statement => transmitted value)
 * left d-pad => Axis 2 at (-32767, 0) => 1
 * up d-pad => Axis 2 at (0, -32767) => 2
 * left d-pad => Axis 2 at (32767, 0) => 3
 * left d-pad => Axis 2 at (0, 32767) => 4
 * Triangle => button 0 => 5
 * O => button 1 => 6
 * X => button 2 => 7
 * Square => button 3 => 8
 * L1 => button 4 => 9
 * R1 => button 5 => 10
 * L2 => button 6 => 11
 * R2 => button 7 => 12
 */
#include <iostream>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/joystick.h>

/**
 * Reads a joystick event from the joystick device.
 *
 * Returns 0 on success. Otherwise -1 is returned.
 */
int read_event(int fd, struct js_event *event)
{
    ssize_t bytes;

    bytes = read(fd, event, sizeof(*event));

    if (bytes == sizeof(*event))
        return 0;

    /* Error, could not read full event. */
    return -1;
}

/**
 * Returns the number of axes on the controller or 0 if an error occurs.
 */
size_t get_axis_count(int fd)
{
    __u8 axes;

    if (ioctl(fd, JSIOCGAXES, &axes) == -1)
        return 0;

    return axes;
}

/**
 * Returns the number of buttons on the controller or 0 if an error occurs.
 */
size_t get_button_count(int fd)
{
    __u8 buttons;
    if (ioctl(fd, JSIOCGBUTTONS, &buttons) == -1)
        return 0;

    return buttons;
}

/**
 * Current state of an axis.
 */
struct axis_state {
    short x, y;
};

/**
 * Keeps track of the current axis state.
 *
 * NOTE: This function assumes that axes are numbered starting from 0, and that
 * the X axis is an even number, and the Y axis is an odd number. However, this
 * is usually a safe assumption.
 *
 * Returns the axis that the event indicated.
 */
size_t get_axis_state(struct js_event *event, struct axis_state axes[3])
{
    size_t axis = event->number / 2;

    if (axis < 3)
    {
        if (event->number % 2 == 0)
            axes[axis].x = event->value;
        else
            axes[axis].y = event->value;
    }

    return axis;
}

int lastDirectionalPad = 0;

int main(int argc, char *argv[]) {
    const char *device= "/dev/input/js0";
    int js;
    struct js_event event;
    struct axis_state axes[3] = {0};
    size_t axis;

    js = open(device, O_RDONLY);

    if (js == -1)
        perror("Could not open joystick");

    /* This loop will exit if the controller is unplugged. */
    while (read_event(js, &event) == 0) {
        switch (event.type) {
            case JS_EVENT_BUTTON:
                printf("%03u", ((event.number + 5) * 10) + event.value);
                break;
            case JS_EVENT_AXIS:
                axis = get_axis_state(&event, axes);
                // Directional pad input is axis 2.
                if (axis == 2) {
                  if (axes[axis].x < 0 && axes[axis].y == 0) {
                    lastDirectionalPad = 3;
                    printf("%03u", (lastDirectionalPad * 10) + 1);
                  } else if (axes[axis].x == 0 && axes[axis].y < 0) {
                    lastDirectionalPad = 4;
                    printf("%03u", (lastDirectionalPad * 10) + 1);
                  } else if (axes[axis].x > 0 && axes[axis].y == 0) {
                    lastDirectionalPad = 1;
                    printf("%03u", (lastDirectionalPad * 10) + 1);
                  } else if (axes[axis].x == 0 && axes[axis].y > 0) {
                    lastDirectionalPad = 2;
                    printf("%03u", (lastDirectionalPad * 10) + 1);
                  } else if (axes[axis].x == 0 && axes[axis].y == 0) {
                    // d-pad button release. Does not allow for diagonal presses this way.
                    printf("%03u", lastDirectionalPad * 10);
                    lastDirectionalPad = 0;
                  }
                  //printf("%zu%u", )
                   // printf("Axis %zu at (%6d, %6d)\n", axis, axes[axis].x, axes[axis].y);
                }
                break;
            default:
                /* Ignore init events. */
                break;
        }

        fflush(stdout);
    }

    close(js);
    return 0;
}
