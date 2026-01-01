
## 依赖安装

sudo apt install python3-gpiozero

这一步会给设备节点赋予权限，重要

安装完成后可以使用 pinout 查看引脚编号，方便开发

## 也可以使用虚拟环境

```
sudo apt install python3-pip python3-venv

pip config set global.index-url https://mirrors.aliyun.com/pypi/simple/
pip config set global.extra-index-url https://git.spacemit.com/api/v4/projects/33/packages/pypi/simple
```

```
python3 -m venv .venv
source .venv/bin/activate 
pip install gpiozero gpiod==2.4.0
```
