 diskpart /s diskpartMount.txt
 timeout /t 5 /nobreak >nul
 del T:\EFI\Bromium\shim.efi
 copy /Y E:\iso\attoxen\attoxen\boot\efi_boot\EFI\BOOT\* T:\EFI\Bromium\* 
 copy /Y T:\EFI\Bromium\BOOTX64.EFI T:\EFI\Bromium\shim.efi
 copy /Y bios\Q35_SINIT_18.BIN T:\EFI\Bromium\sinit_acm_1.bin
 timeout /t 2 /nobreak >nul
 diskpart /s diskpartUnmount.txt
 timeout /t 5 /nobreak >nul