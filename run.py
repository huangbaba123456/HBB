# coding=utf-8
import subprocess
import time
def run_servers():
    # 定义可执行文件及其顺序
    executables = [
        "./bin/log_server",       # log server
        "./bin/register_server",   # register server
        "./bin/gateway_server"        # gateway server
    ]

    processes = []

    try:
        # 按顺序启动每个可执行文件
        for executable in executables:
            process = subprocess.Popen([executable], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            processes.append(process)
            time.sleep(1)  # 等待 1 秒让服务初始化（根据需要调整）

        print("All servers are running. Press Ctrl+C to stop.")

        # 等待所有进程执行完毕
        for process in processes:
            process.wait()

    except KeyboardInterrupt:
        print("\nStopping all servers...")
        for process in processes:
            process.terminate()  # 终止进程
            process.wait()  # 等待进程完全退出
        print("All servers stopped.")

if __name__ == "__main__":
    run_servers()