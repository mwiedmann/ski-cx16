#include <cx16.h>
#include <joystick.h>

#include "wait.h"

void waitForRelease() {
    unsigned char joy;

    while (1) {
        joy = joy_read(0);
        if (joy == 0) {
            break;
        }
        wait();
    }
}

void waitForButtonPress() {
    unsigned char joy;

    while(1) {
        joy = joy_read(0);

        if (JOY_BTN_1(joy) || JOY_BTN_2(joy)) {
            while(JOY_BTN_1(joy) || JOY_BTN_2(joy)) {
                wait();
                joy = joy_read(0);
            }
            break;
        }
    }
}