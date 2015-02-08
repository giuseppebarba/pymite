#-----------------------------------------------------------------------------
# Name:        lcd.py
# Purpose:     LiquidCrystal library (this is a python version of the 
#              LiquidCrystal.cpp file of arduino project)
#             
#             If you're looking for detail :
#             http://code.google.com/p/arduino/source/browse/trunk/libraries/LiquidCrystal/LiquidCrystal.cpp
#
#
#                    # import module
#                    from lcd import LiquidCrystal
#                    from pin import *
#                    # specify lcd pin out
#                    screen = LiquidCrystal(PE7, None, PE8, PE9, PE10, PE11, PE12)
#                    # specify lcd size : 2 rows x 16 cols
#                    screen.begin(16,2)
#                    # print TEOMAN on the first line
#                    screen.prnt("TEOMAN")
#                    screen.setCursor(0,1)
#                    # print Hello World on the second line
#                    screen.prnt("Hello World!")
#              
#             
#  
#             WARNING, THIS IS STILL EXPERIMENTAL
#  
#  thx :     arduino LiquidCrystal.cpp
#
# Author:      Stéphane Bard
#
# Created:     2012/10/25
# RCS-ID:      $Id: lcd.py $
# Licence:     BSD
#-----------------------------------------------------------------------------

import gpio
import sys

LCD_CLEARDISPLAY = 0x01
LCD_RETURNHOME = 0x02
LCD_ENTRYMODESET = 0x04
LCD_DISPLAYCONTROL = 0x08
LCD_CURSORSHIFT = 0x10
LCD_FUNCTIONSET = 0x20
LCD_SETCGRAMADDR = 0x40
LCD_SETDDRAMADDR = 0x80
LCD_ENTRYRIGHT = 0x00
LCD_ENTRYLEFT = 0x02
LCD_ENTRYSHIFTINCREMENT = 0x01
LCD_ENTRYSHIFTDECREMENT = 0x00
LCD_DISPLAYON = 0x04
LCD_DISPLAYOFF = 0x00
LCD_CURSORON = 0x02
LCD_CURSOROFF = 0x00
LCD_BLINKON = 0x01
LCD_BLINKOFF = 0x00
LCD_DISPLAYMOVE = 0x08
LCD_CURSORMOVE = 0x00
LCD_MOVERIGHT = 0x04
LCD_MOVELEFT = 0x00
LCD_8BITMODE = 0x10
LCD_4BITMODE = 0x00
LCD_2LINE = 0x08
LCD_1LINE = 0x00
LCD_5x10DOTS = 0x04
LCD_5x8DOTS = 0x00

# TODO: rewrite example for STM32F4
# TODO: write int method in builtin (this is not possible !!!!)

class LiquidCrystal(object):
    """
    A liquidCrystal for STM32F4. This class is greatly inspired of
    the famous library arduino / LiquidCrystal.cpp

        When the display powers up, it is configured as follows:

        1. Display clear
        2. Function set:
           DL = 1; 8-bit interface data
           N = 0; 1-line display
           F = 0; 5x8 dot character font
        3. Display on/off control:
           D = 0; Display off
           C = 0; Cursor off
           B = 0; Blinking off
        4. Entry mode set:
           I/D = 1; Increment by 1
           S = 0; No shift

        Note, however, that resetting the Arduino doesn't reset the LCD, so we
        can't assume that its in that state when a sketch starts (and the
        LiquidCrystal constructor is called).

    """

    def __init__(self, rs, rw, enable, d0, d1, d2, d3, d4=0, d5=0, d6=0, d7=0):
        """
        each pin should be string like ('E',7), you can use pin.py module instead
        and use PE7 declaration

        possible usage :

                LiquidCrystal(rs, None, enable, d0, d1, d2, d3)
                LiquidCrystal(rs, rw, enable, d0, d1, d2)
                LiquidCrystal(rs, None, enable, d0, d1, d2, d3, d4, d5, d6, d7)
                LiquidCrystal(rs, rw, enable, d0, d1, d2, d3, d4, d5, d6, d7) 
                
                    # RW => PE7
                    # E => PE8
                    # D4 => PE9
                    # D5 => PE10
                    # D6 => PE11
                    # D7 => PE12
                from lcd import LiquidCrystal
                screen = LiquidCrystal(PE7, None, PE8, PE9, PE10, PE11, PE12)
                screen.
       """
        # value authorized for rw are string 'E7' or None !
        if rw is None :
            rw = 255
        # four_bit_mode is fixed by non present keywords ...
        four_bit_mode = 0
        if not d4 or not d5 or not d6 or not d7 :
            four_bit_mode = 1

        self.rs_pin = rs
        self.rw_pin = rw
        self.enable_pin = enable

        self.data_pins = [d0, d1, d2, d3, d4, d5, d6, d7]

        self.set_output(self.rs_pin)

        # we can save 1 pin by not using RW. Indicate by passing 255 instead of pin#
        if self.rw_pin!= 255 :
            self.set_output(self.rw_pin)

        self.set_output(self.enable_pin)

        if four_bit_mode :
            self._displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS
        else :
            self._displayfunction = LCD_8BITMODE | LCD_1LINE | LCD_5x8DOTS
        self.begin(16, 1)

    def dg_write(self, s_pin, value):
        """
        """
        gpio.digital_write(s_pin, value)

    def pn_mode(self, s_pin, value):
        """
        """
        gpio.pin_mode(s_pin, value)

    def begin(self, cols, lines, dotsize=0):
        """
        """
        if lines > 1 :
            self._displayfunction |= LCD_2LINE

        self._numlines = lines
        self._currline = 0
        
        if dotsize != 0 and lines == 1 :
            self._displayfunction |= LCD_5x10DOTS

        # SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
        # according to datasheet, we need at least 40ms after power rises above 2.7V
        # before sending commands. Arduino can turn on way befer 4.5V so we'll wait 50
        # delayMicroseconds(50000); 
        sys.wait(50)

        # Now we pull both RS and R/W low to begin commands
        self.dg_write(self.rs_pin, gpio.LOW)
        self.dg_write(self.enable_pin, gpio.LOW)

        if self.rw_pin != 255 : 
              self.dg_write(self.rw_pin, gpio.LOW)

        # //put the LCD into 4 bit or 8 bit mode
        if not (self._displayfunction & LCD_8BITMODE) :
           # this is according to the hitachi HD44780 datasheet
           # figure 24, pg 46
           # we start in 8bit mode, try to set 4 bit mode
           self.write4bits(0x03)
           # wait min 4.1ms
           sys.wait(4)
           # second try
           self.write4bits(0x03)
           # wait min 4.1ms
           sys.wait(4)
           # third go!
           self.write4bits(0x03) 
           sys.wait(2)

           # finally, set to 4-bit interface
           self.write4bits(0x02) 
        else :
           # this is according to the hitachi HD44780 datasheet
           # page 45 figure 23

           # Send function set command sequence
           self.command(LCD_FUNCTIONSET | self._displayfunction)
           # wait more than 4.1ms
           sys.wait(5)
           # second try
           self.command(LCD_FUNCTIONSET | self._displayfunction)
           sys.wait(1)

           # third go
           self.command(LCD_FUNCTIONSET | self._displayfunction)

        # finally, set # lines, font size, etc.
        self.command(LCD_FUNCTIONSET | self._displayfunction)

        # turn the display on with no cursor or blinking default
        self._displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF

        self.display()

        # clear it off
        self.clear()

        # Initialize to default text direction (for romance languages)
        self._displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT

        # set the entry mode
        self.command(LCD_ENTRYMODESET | self._displaymode)

        # clean up
        sys.gc()
    
    def set_output(self, pin):
        """
        """
        self.pn_mode(pin, gpio.OUTPUT)

    def clear(self):
        # clear display, set cursor position to zero
        self.command(LCD_CLEARDISPLAY)
        # this command takes a long time!
        sys.wait(2)  

        # clean up
        sys.gc()

    def home(self):
        """
        """
        # set cursor position to zero
        self.command(LCD_RETURNHOME)
        # this command takes a long time!
        sys.wait(2)  

    def setCursor(self, col, row):
        row_offsets = [ 0x00, 0x40, 0x14, 0x54 ]
        if ( row >= self._numlines ) :
            # we count rows starting w/0
            row = self._numlines-1
        self.command(LCD_SETDDRAMADDR | (col + row_offsets[row]))

    def noDisplay(self):
        """
        Turn the display on/off (quickly)
        """
        self._displaycontrol &= ~LCD_DISPLAYON
        self.command(LCD_DISPLAYCONTROL | self._displaycontrol)

    def display(self):
        """
        """
        self._displaycontrol |= LCD_DISPLAYON
        self.command(LCD_DISPLAYCONTROL | self._displaycontrol)

    def noCursor(self):
        """
        Turns the underline cursor on/off
        """
        self._displaycontrol &= ~LCD_CURSORON
        self.command(LCD_DISPLAYCONTROL | self._displaycontrol)

    def cursor(self):
        """
        """
        self._displaycontrol |= LCD_CURSORON
        self.command(LCD_DISPLAYCONTROL | self._displaycontrol)

    def noBlink(self):
        """
        Turn on and off the blinking cursor
        """
        self._displaycontrol &= ~LCD_BLINKON
        self.command(LCD_DISPLAYCONTROL | self._displaycontrol)

    def blink(self):
        self._displaycontrol |= LCD_BLINKON
        self.command(LCD_DISPLAYCONTROL | self._displaycontrol)

    def scrollDisplayLeft(self) :
        """
        These commands scroll the display without changing the RAM
        """
        self.command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT)

    def scrollDisplayRight(self) :
        """
        """
        self.command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT)

    def leftToRight(self) :
        """
        This is for text that flows Left to Right
        """
        self._displaymode |= LCD_ENTRYLEFT
        self.command(LCD_ENTRYMODESET | self._displaymode)

    def rightToLeft(self) :
        """
        This is for text that flows Right to Left
        """
        self._displaymode &= ~LCD_ENTRYLEFT
        self.command(LCD_ENTRYMODESET | self._displaymode)

    def autoscroll(self) :
        """
        This will 'right justify' text from the cursor
        """
        self._displaymode |= LCD_ENTRYSHIFTINCREMENT
        self.command(LCD_ENTRYMODESET | self._displaymode)

    def noAutoscroll(self):
        """
        This will 'left justify' text from the cursor
        """
        self._displaymode &= ~LCD_ENTRYSHIFTINCREMENT
        self.command(LCD_ENTRYMODESET | self._displaymode)

    def createChar(self, location, charmap):
        """
        Allows us to fill the first 8 CGRAM locations
        with custom characters
        """
        # we only have 8 locations 0-7
        location &= 0x7 
        self.command(LCD_SETCGRAMADDR | (location << 3))

        for i in range(8) :
            self.write(charmap[i])

    def prnt(self, value):
        """
        print chars on lcd ...
        """
        for c in value :
            self.write(ord(c))

    def write(self, value):
        """
        """
        self.send(value, gpio.HIGH);

    def command(self, value):
        """
        mid level commands, for sending data/cmds 
        """
        self.send(value, gpio.LOW)

    def send(self, value, mode) :
        """
        """
        self.dg_write(self.rs_pin, mode)
    
        # if there is a RW pin indicated, set it low to Write
        if (self.rw_pin != 255) :
           self.dg_write(self.rw_pin, gpio.LOW)
      
        if (self._displayfunction & LCD_8BITMODE):
           self.write8bits(value)
        else :
           self.write4bits(value>>4)
           self.write4bits(value)
    
    def pulseEnable(self) :
        """
        """
        self.dg_write(self.enable_pin, gpio.LOW)
        sys.wait(1)
        self.dg_write(self.enable_pin, gpio.HIGH)
        # enable pulse must be >450ns
        sys.wait(1)
        self.dg_write(self.enable_pin, gpio.LOW)
        # commands need > 37us to settle
        sys.wait(1)
    
    def write4bits(self, value) :
        """
        """
        for i in range(4) :
            self.pn_mode(self.data_pins[i], gpio.OUTPUT)
            self.dg_write(self.data_pins[i], (value >> i) & 0x01)
        self.pulseEnable()
    
    def write8bits(self, value) :
        """
        """
        for i in range(8) :
            self.pn_mode(self.data_pins[i], gpio.OUTPUT)
            self.dg_write(self.data_pins[i], (value >> i) & 0x01)
        self.pulseEnable()
