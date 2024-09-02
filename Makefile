DEVICE = attiny13a
AVRDUDE = avrdude -p $(DEVICE) -c avrispv2 -P /dev/cu.usbmodem001432501 -b 38400
CLOCK      = 9600000


# fuse settings:
# use http://www.engbedded.com/fusecalc

# Not tested, but should be reasonable for ATtiny13A

fuses : ; $(AVRDUDE) -B 100kHz -U lfuse:w:0x6a:m -U hfuse:w:0xff:m
