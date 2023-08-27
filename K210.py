import sensor, image, time, lcd, math

from pid import PID
from machine import Timer,UART
from modules import ybrgb,ybserial
from board import board_info
from fpioa_manager import fm
#from pyb import Servo

#pan_servo=Servo(1)
#tilt_servo=Servo(2)

#pan_servo.calibration(500,2500,500)
#tilt_servo.calibration(500,2500,500)

lcd.init()
RGB = ybrgb()
## maixduino board_info PIN10/PIN11/PIN12/PIN13 or other hardware IO 10/11/4/3
fm.register(8, fm.fpioa.UART1_TX, force=True)
fm.register(6, fm.fpioa.UART1_RX, force=True)

uart=UART(UART.UART1, 115200, 8, 0, 0, timeout=1000, read_buf_len=4096)
#IMAGE_WIDTH=sensor.snapshot().width()
#IMAGE_HEIGHT=sensor.snapshot().height()
#IMAGE_DIS_MAX=(int)(math.sqrt(IMAGE_WIDTH*IMAGE_WIDTH+IMAGE_HEIGHT*IMAGE_HEIGHT)/2)

class target_check(object):
    p=0          #int16_t
    t=0          #int16_t
    flag=0       #uint8_t
    state=0      #uint8_t
    img_width=0  #uint16_t
    img_height=0 #uint16_t
    reserved1=0  #uint8_t
    reserved2=0  #uint8_t
    reserved3=0  #uint8_t
    reserved4=0  #uint8_t
    fps=0        #uint8_t
    range_sensor1=0
    range_sensor2=0
    range_sensor3=0
    range_sensor4=0
    camera_id=0
    reserved1_int32=0
    reserved2_int32=0
    reserved3_int32=0
    reserved4_int32=0

class rgb(object):
    def __init__(self):
        self.red=RGB.set(1, 0, 0)
        self.green=RGB.set(0, 1, 0)
        self.blue=RGB.set(1, 1, 0)



class uart_buf_prase(object):
    uart_buf = []
    _data_len = 0
    _data_cnt = 0
    state = 0

class mode_ctrl(object):
    work_mode = 0x01 #工作模式.默认是点检测，可以通过串口设置成其他模式
    check_show = 1   #开显示，在线调试时可以打开，离线使用请关闭，可提高计算速度

ctr=mode_ctrl()


#rgb=rgb()
R=uart_buf_prase()
target=target_check()
target.camera_id=0x01
target.reserved1_int32=0
target.reserved2_int32=0
target.reserved3_int32=0
target.reserved4_int32=0

HEADER=[0xFF,0xFC]
MODE=[0xF1,0xF2,0xF3]
#__________________________________________________________________
def package_blobs_data(mode):
    #数据打包封装
    data=bytearray([HEADER[0],HEADER[1],0xC0+mode,0x00,
                   target.p>>8,target.p,        #将整形数据拆分成两个8位
                   target.t>>8,target.t,        #将整形数据拆分成两个8位
                   target.flag,                 #数据有效标志位
                   target.state,                #数据有效标志位
                   target.img_width>>8,target.img_width,    #将整形数据拆分成两个8位
                   target.img_height>>8,target.img_height,  #将整形数据拆分成两个8位
                   target.fps,      #数据有效标志位
                   target.reserved1,#数据有效标志位
                   target.reserved2,#数据有效标志位
                   target.reserved3,#数据有效标志位
                   target.reserved4,#数据有效标志位
                   target.range_sensor1>>8,target.range_sensor1,
                   target.range_sensor2>>8,target.range_sensor2,
                   target.range_sensor3>>8,target.range_sensor3,
                   target.range_sensor4>>8,target.range_sensor4,
                   target.camera_id,
                   target.reserved1_int32>>24&0xff,target.reserved1_int32>>16&0xff,
                   target.reserved1_int32>>8&0xff,target.reserved1_int32&0xff,
                   target.reserved2_int32>>24&0xff,target.reserved2_int32>>16&0xff,
                   target.reserved2_int32>>8&0xff,target.reserved2_int32&0xff,
                   target.reserved3_int32>>24&0xff,target.reserved3_int32>>16&0xff,
                   target.reserved3_int32>>8&0xff,target.reserved3_int32&0xff,
                   target.reserved4_int32>>24&0xff,target.reserved4_int32>>16&0xff,
                   target.reserved4_int32>>8&0xff,target.reserved4_int32&0xff,
                   0x00])
    #数据包的长度
    data_len=len(data)
    data[3]=data_len-5#有效数据的长度
    #和校验
    sum=0
    for i in range(0,data_len-1):
        sum=sum+data[i]
    data[data_len-1]=sum
    #返回打包好的数据
    return data
#__________________________________________________________________
#red_threshold  = (13, 49, 18, 61, 6, 47)
green_threshold = (55, 99, -30, -4, 0, 25) #(50, 100, -65, -1,
red_threshold = (0, 70, -3, 18, -21, 10)#(16, 70, 2, 79, 4, 26)#(45, 88, 17, 62, -20, 12)#(16, 70, 2, 79, 4, 26)#(49, 90, 18, 66, -17, 18)#(7, 79, 15, 127, -128, 127)#(30, 95, 10, 53, -11, 10)#(48, 100, 37, 52, -12, -1)#(42, 48, 28, 37, -14, 3)#(8, 44, 27, 52, -3, 19)#haode#(18, 28, -5, 0, 1, 14)#(0, 89, 21, 85, 4, 37)(30, 75, 30, , 0,

pan_pid = PID(p=0.07, i=0, imax=90) #脱机运行或者禁用图像传输，使用这个PID
tilt_pid = PID(p=0.05, i=0, imax=90) #脱机运行或者禁用图像传输，使用这个PID
#pan_pid = PID(p=0.1, i=0, imax=90)#在线调试使用这个PID
#tilt_pid = PID(p=0.1, i=0, imax=90)#在线调试使用这个PID

#sensor.reset() # Initialize the camera sensor.
#sensor.set_pixformat(sensor.RGB565) # use RGB565.
#sensor.set_framesize(sensor.QQVGA) # use QQVGA for speed.
#sensor.skip_frames(10) # Let new settings take affect.
#sensor.set_auto_whitebal(False) # turn this off.
#sensor.set_brightness(-2)
#sensor.set_contrast(3)
#sensor.set_auto_gain(False, gain_db=5)
#clock = time.clock() # Tracks FPS.

sensor.reset()
sensor.set_pixformat(sensor.GRAYSCALE)
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time = 2000)
sensor.set_auto_gain(False) # must be turned off for color tracking
sensor.set_auto_whitebal(False) # must be turned off for color tracking
clock = time.clock()

def find_max(blobs):
    max_size=0
    for blob in blobs:
        if blob[2]*blob[3] > max_size:
            max_blob=blob
            max_size = blob[2]*blob[3]
    return max_blob

#thresholds = (90, 100, -128, 127, -128, 127)
#while(True):
    #clock.tick() # Track elapsed milliseconds between snapshots().
    #img = sensor.snapshot() # Take a picture and return the image.
    ##img = sensor.snapshot().binary([thresholds], invert=False, zero=True)

    #blobs = img.find_blobs([red_threshold])
    #if blobs:
        #max_blob = find_max(blobs)
        #pan_error = max_blob.cx()-img.width()/2
        #tilt_error = img.height()/2-max_blob.cy()

        #print("pan+_error: ", pan_error)

        #img.draw_rectangle(max_blob.rect()) # rect
        #img.draw_cross(max_blob.cx(), max_blob.cy()) # cx, cy

        #pan_output=pan_pid.get_pid(pan_error,1)/2
        #tilt_output=tilt_pid.get_pid(tilt_error,1)
        #target.p = int(pan_output)
        #target.t = int(tilt_output)
        #uart.write(package_blobs_data(0x13))
        #print([pan_error, tilt_error])
    #lcd.display(img)
        ##print("pan_output",pan_output)
        ##pan_servo.angle(pan_servo.angle()+pan_output)
        ##tilt_servo.angle(tilt_servo.angle()-tilt_output)
threshold = (190, 255)
while(True):
    clock.tick() # Track elapsed milliseconds between snapshots().
    img = sensor.snapshot() # Take a picture and return the image.
    #img = sensor.snapshot().binary([thresholds], invert=False, zero=True)

    blobs = img.find_blobs([threshold], pixels_threshold=2, area_threshold=2, merge=False)
    if blobs:
        max_blob = find_max(blobs)
        img.draw_rectangle(max_blob.rect())
        img.draw_cross(max_blob.cx(), max_blob.cy())
        pan_error = max_blob.cx()-img.width()/2
        tilt_error = img.height()/2-max_blob.cy() + 8
        print([pan_error, tilt_error])
        img.draw_rectangle(max_blob.rect()) # rect
        img.draw_cross(max_blob.cx(), max_blob.cy()) # cx, cy
        #if pan_error > 3 or tilt_error > 3:
        pan_output=pan_pid.get_pid(pan_error,1)/2
        tilt_output=tilt_pid.get_pid(tilt_error,1)
        target.p = int(pan_output * 1000)
        target.t = int(tilt_output * 1000)
        RGB.set(1, 1, 0)
        uart.write(package_blobs_data(0x13))
    lcd.display(img)
