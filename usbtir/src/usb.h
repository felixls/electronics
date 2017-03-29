#ifndef USB_H
#define USB_H

extern void usb_poll(void);
extern void usb_init(void);
extern void usb_disconnect(void);
extern void usb_connect(void);
extern uint8_t usb_interruptisready();

extern void idle_timer_init(void);
extern void idle_timer(void);
extern void reset_timeout(void);

extern void hid_clear(void);

extern void mouse_move(int8_t, int8_t);
extern void mouse_click(void);
extern void mouse_rclick(void);

extern void kbd_send(uint8_t, uint8_t);

#endif
