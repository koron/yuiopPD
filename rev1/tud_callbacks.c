#include <stdio.h>
#include <tusb.h>

// Invoked when received GET_REPORT control request
//
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request.
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
    printf("unhandle HID get report: instance=%d id=%d type=%d buf[0]=%02x len=%d\n", instance, report_id, report_type, buffer[0], reqlen);
    return reqlen;
}

// Invoked when received SET_REPORT control request or received data on OUT
// endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {
    // dump unknown reports.
    printf("unknown HID set report: instance=%d id=%d type=%d size=%d\n", instance, report_id, report_type, bufsize);
    for (uint16_t i = 0; i < bufsize; i++) {
        printf(" %02x", buffer[i]);
        if ((i+1) % 16 == 0 || i == bufsize-1) {
            printf("\n");
        }
    }
}
