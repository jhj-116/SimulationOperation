
import sys
import subprocess
import traceback
import time
import psutil
from win32 import win32gui
from win32 import win32api
from win32 import win32process
WM_CHAR=0x0400

def handle_exception(exc_type,exc_value,exc_traceback):
    print("未处理异常: ")
    traceback.print_exception(exc_type,exc_value,exc_traceback)

def open_ue_project(project_path):
    var=0
    ue_editor_path="D:/UE4/UE_5.2/Engine/Binaries/Win64/UnrealEditor.exe"                       #sys.argv[1]
    command=[ue_editor_path,project_path]
    try:
        process=subprocess.Popen(command,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
        print("进程id :",process.pid)
    except OSError as e:
        print("not open ue project:{e}")   
        
#open_ue_project("D:/CityBase/DigitalBase-dev/DigitalBase.uproject")    #sys.argv[2]




strCmd="UnrealEditor.exe"
#map类型，键值为进程ID,value为窗口句柄。
mID2Handle={}
def get_all_hwnd(hwnd,mouse):
  if win32gui.IsWindow(hwnd) and win32gui.IsWindowEnabled(hwnd) and win32gui.IsWindowVisible(hwnd):
    nID=win32process.GetWindowThreadProcessId(hwnd)
    #print(nID,win32gui.GetWindowText(hwnd))
    del nID[0]
    for abc in nID:
      try:
        pro=psutil.Process(abc).name()
      except psutil.NoSuchProcess:
        pass
      else:
        #print(abc,win32gui.GetWindowText(hwnd))
        if pro == strCmd:
          print("进程ID：",abc,"窗口句柄: ",hwnd,"标题: ",win32gui.GetWindowText(hwnd))
          mID2Handle[abc]=hwnd
          while True:
               asc = ord('q')
               win32api.SendMessage(hwnd,WM_CHAR,asc,0)
          
win32gui.EnumWindows(get_all_hwnd, 0)

#D:/UE4/UE_5.1/Engine/Binaries/Win64/UnrealEditor.exe
#D:/UE5_Program/Pixel51/Pixel51.uproject

