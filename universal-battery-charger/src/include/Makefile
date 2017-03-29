all:	clean adc.c delay.c lcd.c serie.c i2c.c m24xx256.c utilities.c
	sdcc -mpic14 -p16f877a -V -I"./include" -c adc.c
	sdcc -mpic14 -p16f877a -V -I"./include" -c delay.c
	sdcc -mpic14 -p16f877a -V -I"./include" -c lcd.c
	sdcc -mpic14 -p16f877a -V -I"./include" -c serie.c
	sdcc -mpic14 -p16f877a -V -I"./include" -c i2c.c
	sdcc -mpic14 -p16f877a -V -I"./include" -c m24xx256.c
	sdcc -mpic14 -p16f877a -V -I"./include" -c utilities.c
	gplib -c ../lib/felixls.lib adc.o delay.o lcd.o serie.o i2c.o m24xx256.o utilities.o

clean:	
	rm -f adc.o adc.lst adc.asm \
        delay.o delay.lst delay.asm \
        lcd.o lcd.lst lcd.asm \
        serie.o serie.lst serie.asm \
        i2c.o i2c.lst i2c.asm \
        m24xx256.o m24xx256.lst m24xx256.asm \
        utilities.o utilities.lst utilities.asm
        
        
	

