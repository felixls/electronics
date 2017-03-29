PROG_NAME = fuente.hex
OBJS = fuente.o delay.o lcd.o serie.o adc.o
sdcc_PIC_TYPE = 16f877a

TOOLSDIR = /usr
DEBUG = -DDEBUG_UART #-DDEBUG -DDEBUG_UART -DDEBUG -DDEBUG_PRINT 
CFLAGS = -mpic14 -p$(sdcc_PIC_TYPE) $(DEBUG) #--denable-peeps --opt-code-size --optimize-cmp --optimize-df
INC = -I.

all:	clean $(PROG_NAME)

.c.o:
	sdcc  $(CFLAGS) $(INC) -c $<

$(PROG_NAME): $(OBJS)
	sdcc $(DEBUG) -Wl,-m -mpic14 -p$(sdcc_PIC_TYPE) $+ -o $(@) #-llibio$(sdcc_PIC_TYPE).lib -llibc18f.lib

clean:	
	rm -f *.o *.lst *.cod *.hex *.map *.asm	      

