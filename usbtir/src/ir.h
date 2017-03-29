#ifndef IR_H
#define IR_H

#if IRDEBUG_LEVEL > 0
extern void inc_protocol();
extern void ir_record();
#endif

extern void ir_init(uint8_t);
extern void ir_decode();

#endif
