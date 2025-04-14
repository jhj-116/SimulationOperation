
import sys
import subprocess
import traceback

def handle_exception(exc_type,exc_value,exc_traceback):
    print("未处理异常: ")
    traceback.print_exception(exc_type,exc_value,exc_traceback)

def open_ue_project(project_path):
    ue_editor_path=sys.argv[1] #"D:/UE4/UE_5.1/Engine/Binaries/Win64/UnrealEditor.exe"                       #sys.argv[1]
    command=[ue_editor_path,project_path]
    try:
        process=subprocess.Popen(command,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
        while True:
            output=process.stdout.readline()
            if output==b'' and process.poll() is not None:
                break
            if output:
                print(output.decode().strip())       
        if process.poll()==0:
            print("进程成功退出")
        else:
            print(" 进程崩溃:")
            handle_exception(*sys.exc_info())
    except OSError as e:
        print("not open ue project:{e}")   
        
        
open_ue_project(sys.argv[2])    #sys.argv[2]



#D:/UE4/UE_5.1/Engine/Binaries/Win64/UnrealEditor.exe
#D:/UE5_Program/Pixel51/Pixel51.uproject



LogTemp: Warning: HWD :0x00000000003A0FD8
LogTemp: Warning: HWD :0x00000000003A0FD8
LogTemp: Warning: HWD :0x00000000003A0FD8
LogTemp: Warning: HWD :0x00000000003A0FD8
LogTemp: Warning: HWD :0x00000000003A0FD8
LogTemp: Warning: HWD :0x0000000000430CE6
LogTemp: Warning: HWD :0x00000000003A0FD8
LogTemp: Warning: HWD :0x00000000003A0FD8
LogTemp: Warning: HWD :0x00000000003A0FD8
LogTemp: Warning: HWD :0x00000000003A0FD8
LogTemp: Warning: HWD :0x00000000003A0FD8
LogTemp: Warning: HWD :0x00000000003A0FD8
LogTemp: Warning: HWD :0x00000000003A0FD8
LogTemp: Warning: HWD :0x00000000003A0FD8
LogTemp: Warning: [Mouse Button Down] Scre