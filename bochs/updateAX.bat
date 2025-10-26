 diskpart /s diskpartMount.txt
 timeout /t 5 /nobreak >nul
 del T:\EFI\Bromium\shim.efi
 copy /Y efi_boot\EFI\BOOT\* T:\EFI\Bromium\* 
 copy T:\EFI\Bromium\BOOTX64.EFI T:\EFI\Bromium\shim.efi
 timeout /t 2 /nobreak >nul
 diskpart /s diskpartUnmount.txt
 timeout /t 5 /nobreak >nul