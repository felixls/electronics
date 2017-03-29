all:	clean charger.c chargeprocess.c display.c dischargeprocess.c menu.c
	sdcc -mpic14 -p16f877a -V -I"./include" -c charger.c
	sdcc -mpic14 -p16f877a -V -I"./include" -c chargeprocess.c
	sdcc -mpic14 -p16f877a -V -I"./include" -c display.c
	sdcc -mpic14 -p16f877a -V -I"./include" -c dischargeprocess.c
	sdcc -mpic14 -p16f877a -V -I"./include" -c menu.c
	gplink -m charger.o chargeprocess.o display.o dischargeprocess.o menu.o -o BatteryCharger.hex -I/usr/share/sdcc/include/pic/ /usr/share/sdcc/lib/pic/pic16f877a.lib /usr/share/sdcc/lib/pic/libsdcc.lib /usr/share/sdcc/lib/pic/libm.lib ./lib/felixls.lib

sim:	
	gpsim -s BatteryCharger.cod -c BatteryCharger.stc

clean:	
	rm -f BatteryCharger.map BatteryCharger.cod BatteryCharger.hex BatteryCharger.lst \
	      charger.o charger.asm charger.lst charger.p charger.adb \
	      chargeprocess.o chargeprocess.asm chargeprocess.lst chargeprocess.p chargeprocess.adb \
	      display.o display.asm display.lst display.p display.adb \
	      dischargeprocess.o dischargeprocess.asm dischargeprocess.lst dischargeprocess.p dischargeprocess.adb \
	      menu.o menu.asm menu.lst menu.p menu.adb
	      
	      
	      
	      
	

