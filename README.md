# stm32-charger
Repurposes the openinverter main board as a buck or boost mode charger

# Disclaimer
This project is not intended for newbies. You are working with grid voltages and that means if you touch ONE live part you get electrocuted. That means you might die. That means you might go to hell if you believe in it. The source code is presented only for educational purposes and whatever you do with it happens on you own risk!

# Features
- PI control of charger output current
- Controller is quick enough to optimize power factor at least in boost mode
- Control of either input or output relay
- Control of an additional relay for precharging output caps

# Pin map
- Start input (PA2): start/stop charging
- Mprot input (PA3): tie to 12V, measure you logic supply voltage
- Emcystop input (PC7): tie to 12V optionally via an emergeny stop switch
- BMS input (PC8): currently unused, might be used to stop charging via ChaDeMo signal in future
- il1, il2 (PB0, PA5): output current sensor, connect them together on your board to keep the current limiter happy
- udc (PC3): input or output voltage. Used for precharge in the former case
- tmphs (PC4): heat sink temperature, used for fan control and over temperature shutdown
- dcsw (PC13): closes when you hit start and udc >= udcsw
- outc (formerly precharge) output (PB1): closes 4s after dcsw after precharging output caps to battery voltage
- temp output (PB9): outputs PWM proportional to heatsink temperature
- PWM1 positive (PA8): IGBT gate drive signal
- PWM2 positive (PA9): PWM proportional to output current, I use it with a cheap modified voltmeter that displays current now

# Operation
- Configuration as always, go figure ;)
- Start via button or "run" parameter
- idclim is the maximum charge current
- idcspnt is the requested charge current and is always capped at idclim
- Current is set to 0 when tmphsmax is hit and restored when tmphsmax-5 is reached
- When you hit start again, PWM is stopped and 1s later all relays are opened

# Minimal ChaDeMo setup
The board uses the known CAN module from the inverter so you can map anything everywhere. Minimal ChaDeMo setup is thus:
- can tx maxvtg 265 8 16 1
- can tx opmode 265 40 3 2
- can tx idclim 264 24 8 1
- can rx idcspnt 258 24 8 32
- can rx soc 258 48 16 16

Obviously this takes none of the safety measures into account

# Compiling
You will need the arm-none-eabi toolchain: https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads
The only external depedencies are libopencm3 and libopeninv. You can download and build this dependency by typing

`make get-deps`

Now you can compile stm32-sine by typing

`make`

And upload it to your board using a JTAG/SWD adapter, the updater.py script or the esp8266 web interface
