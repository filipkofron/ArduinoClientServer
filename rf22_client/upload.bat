avrdude -P COM3 -b 19200 -c avrisp -p m328p -U flash:w:rf22_client.cpp.eightanaloginputs.hex -C "C:\Program Files (x86)\Arduino\hardware\tools\avr\etc\avrdude.conf"
pause