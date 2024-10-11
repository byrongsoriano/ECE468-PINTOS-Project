# VirtualBox Ubuntu VM Setup for PINTOS ECE468 by Compiler Comrades

This guide outlines the steps to set up a VirtualBox environment running Ubuntu for your PINTOS project. Please note that some operations may take time; do not panic if things seem slow.


## Setup VirtualBox

1. Install VirtualBox: [VirtualBox Installation](https://www.virtualbox.org/)
2. Download page: [VirtualBox Downloads](https://www.virtualbox.org/wiki/Downloads)

## Setting Up Your Ubuntu Virtual Machine

### Step 2a: Download Ubuntu ISO

- Download the ISO file of Ubuntu and save it in a folder for your PINTOS project files.
- **Warning:** Do not click the ISO file; it may replace your current operating system.

### Step 2b: Create a New Virtual Machine

1. Start VirtualBox.
2. Click **New**.
3. In the window that opens:
   - **Name:** Enter your preferred name (e.g., *Galen’s ECE 468*).
   - **Type:** Select *Linux*.
   - **Version:** Leave at the default of *Ubuntu (64-bit)*.
4. Click **Next**.

### Step 2c: Configure Memory Size

- Select **4 GB (or 4096 MB)** of RAM.
- Click **Next**.

### Step 2d: Configure Hard Disk

1. Leave at default: **Create a virtual hard disk now**.
2. Click **Create**.
3. For hard disk file type, leave at default: **VDI (VirtualBox Disk Image)**.
4. Click **Next**.

### Step 2e: Storage on Physical Hard Disk

- Leave at default: **Dynamically allocated**.
- Click **Next**.

### Step 2f: File Location and Size

1. Leave at default (e.g., *Galen ECE468*).
2. Set size to **32 GB**.
3. Click **Create**.

## Starting Your VM

### Step 3a: Start the VM

1. Select **Start** to initiate the VM.
2. When prompted, locate the Ubuntu ISO file you downloaded in Step 2a.
3. Ubuntu should now be running in your VM.

### Step 3b: Install Ubuntu

1. In the Welcome window, select **Install** and ensure *English* is chosen as your language.
2. Follow the default prompts until reaching the **Installation type**.

### Step 3c: Confirm Installation Type

- You may see a message stating “Erase disk and install Ubuntu.” This will **not** affect your Windows host operating system. It installs in the VM only.
- Proceed with defaults, select to login automatically, and create a user (e.g., *Galen kaimuki*).

## Setting Up PINTOS in the VM

### Step 4a: Start VM and Open Terminal

1. Start the VM.
2. Right-click and select **Terminal**.

### Step 4b: Install Software Packages

Follow the instructions in *EE468PintOS-2024.pdf* Appendix PINTOS setup to install software packages. Use the following commands to install them individually to verify each installation:

commands to run in bash:
sudo apt-get update
sudo apt-get install gcc
sudo apt-get install qemu-system-common
sudo apt-get install make
sudo apt-get install binutils
sudo apt-get install perl
sudo apt-get install gdb

Additionally, install the following QEMU packages:

sudo apt install qemu-kvm libvirt-daemon-system libvirt-clients bridge-utils
To verify QEMU installation:

qemu-system-x86_64 --version
Step 4c: Install PINTOS
Download the PINTOS tarball:

wget https://web.stanford.edu/class/cs140/projects/pintos/pintos.tar.gz
Extract it to your home directory:

tar -xvzf pintos.tar.gz -C $HOME
Add the directory to your PATH:


echo PATH="$PATH:$HOME/pintos/src/utils" >> ~/.bashrc
source ~/.bashrc
Modify the pintos script:

Navigate to src/utils.
Open the pintos script using a text editor (e.g., vi pintos).
Change line 103 from $sim = "bochs" to $sim = "qemu".
Change line 623 from my (@cmd) = ('qemu'); to my (@cmd) = ('qemu-system-i386');.
Modify the shutdown device:

Go to src/devices.
Open shutdown.c.
After printf ("Powering off...\n");, insert the line:
c

outw(0x604, 0x0 | 0x2000);
Change the simulator in the Make.vars file:

Go to the src/threads directory.
Open the Make.vars file.
Change the simulator to qemu:
makefile
Copy code
SIMULATOR = --qemu
Compile everything:


make
To test, navigate into the new build directory (e.g., threads or userprog) and run:

### Notes
You should be getting a repeated loop of something like "make[1] entering directory ... /userprog/build'
pintos -v -k -T 60 --qemu --" Which on average takes 10~30 minutes.

If it shows something in the end like # of # tests failed, you will know that the initial setup and compiling works correctly. Please do it carefully!

make check
Step 4d: Additional Project B Testing
Note that Project B is located in the userprog directory. Perform the same make and make check commands there.
You are now set up with Ubuntu and PINTOS on your VirtualBox VM!
