 diskpart /s diskpartMount.txt
 timeout /t 5 /nobreak >nul
 del T:\EFI\Bromium\shim.efi
 copy /Y efi_boot\EFI\BOOT\* T:\EFI\Bromium\* 
 timeout /t 2 /nobreak >nul
 diskpart /s diskpartUnmount.txt
 timeout /t 5 /nobreak >nul