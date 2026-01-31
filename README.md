# Simple snake implementation using UEFI 

## Ways to run :
  1. qemu vm
  2. actual machine

### qemu
  1. make sure you have gnu-efi library in your system  
  2. make sure to install qemu on your system (ex for arch pacman -Sy qemu-base)
  3. make sure you have OVMF firmware to emulate a UEFI bios
  4. Run the sc script which builds the code into a efi file and run it in a vm

### Real Hardware
  1. make sure you have gnu-efi library in your system
  2. Run the make command to build the efi program
  3. You will need a esp (EFI system partition) to run this (for linux & windows their is already one for linux mount point is generally /boot/efi)
  4. create a new folder inside EFI directory and copy your game.efi file there
  5. now using efibootmgr create a boot record and make it at top priority

    sudo efibootmgr --create --disk /dev/nvme1n1 --part 5 --label "snake-game" --loader /EFI/BOOT/game.efi
  6. simply restart the pc and now it will boot into the SNAKE :)


 #### to remve boot entry
    
   1. delete the unneccessry folder in efi partionn
   2. use following command to delete the boot entry
        ```
        sudo efibootmgr -b <XXXX> -B
        ```
      where <XXXX> is boot id 
