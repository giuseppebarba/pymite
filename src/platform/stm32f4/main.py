## pymite for stm32f4discovery eva-kit. demo-program
## this demo program description in japanese:
## 起動時にUSERボタンが解放されていると対話pymite(ipm)を起動します。
## 起動時にUSERボタンを押しておいて解放すると，デモプログラムを実行します。
## STM32F4discoveryの出荷時に書き込まれているデモプログラムのようなものです。
## 実行するとLEDが順次または同時に明滅します。
## USERボタンを押して離すと，ボードの傾き(加速度センサの出力値)により
## 対応するLEDの明るさを変えて表示します。
## もう一度USERボタンを押して離すと，最初に戻ります。
## メモ：メモリが少ないときはwait_a_bit()の中などでsys.gc()の実行頻度を上げます。


import sys
import ipm
import stm32f4discovery
from pin import PE7

x=stm32f4discovery
i=0
def wait_a_bit():
  global i;
  i= (i+1) & 0xfff
  if(15==i & 15):
    sys.gc()     # clean up
  else:
    sys.wait(4)  # wait a bit

def wait_BTN_release():
  while 1==x.BTN(): # waiting for release USER BUTTON
    sys.gc()     # clean up
    sys.wait(100)

def LED_all(v=0):
    x.LED(0,v)
    x.LED(1,v)
    x.LED(2,v)
    x.LED(3,v)

def LED_glow_seq(a):
    if(0 == (a & 0x800)):
      if(a & 0x100):
        LED_all((0x100-(a&0xff)))
      else:
        LED_all((a&0xff))
    else:
      if(a & 0x100):
        x.LED( (a>>9) & 3, (0x100-(a&0xff)))
      else:
        x.LED( (a>>9) & 3, (a&0xff))

def LED_axel_indicate(ax):
    LED_all()
    if(ax[1]>0): x.LED(0,ax[1]<<1 )
    if(ax[0]>0): x.LED(1,ax[0]<<1 )
    if(ax[1]<0): x.LED(2,(1-ax[1])<<1 )
    if(ax[0]<0): x.LED(3,(1-ax[0])<<1 )

sys.gc()

def gpio_demo() :
    """import gpio
    print "gpio imported"
    gpio.pin_mode(PE7, gpio.OUTPUT)
    print "looping"
    while True:
        gpio.digital_write(PE7, gpio.HIGH)
        gpio.digital_write(PE7, gpio.LOW)"""
    
    from gpio import pin_mode, digital_write, OUTPUT, INPUT, HIGH, LOW
    print "gpio imported"
    pin_mode("E", 7, OUTPUT)
    print "looping"
    while True:
        digital_write(PE7, HIGH)
        digital_write(PE7, LOW)

def launch_ipm():
    """
    """
    print "launching ipm"
    ipm.ipm()

def default_demo():
    """
    """
    while True:
        wait_BTN_release()
        x.AXEL(0)
        LED_all();
        while 0==x.BTN(): # until push USER BUTTON
          LED_glow_seq(i)
          wait_a_bit()
        wait_BTN_release()
        x.AXEL(1)
        while 0==x.BTN(): # until push USER BUTTON
          axel= x.AXEL()
          LED_axel_indicate(axel)
          wait_a_bit()

def main():
    """
    """
    launch_ipm()
    #default_demo()
    #gpio_demo()

main()
