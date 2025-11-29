# Bootloader Notes

## Description
- The bootloader handles erasing and programming new application firmware into the STM32.
- The `bootloader.py` script runs on the host and communicates with the bootloader over UART to send firmware data and control the flashing process.

## Bootloader Operation

The bootloader checks the state of the user button (B1) during reset to decide whether to enter bootloader mode or launch the application.

### Entering Bootloader Mode
- If the user button is held at startup, the bootloader enters update mode.
- In this mode, it listens on UART for flashing commands.
- The supported commands are:
    - `0x11`: Erase application sectors (sectors 2–7).
    - `0x21`: Program flash. The command includes the target address, data length, and payload.

### Jumping to Application
- If the user button is not pressed, the bootloader attempts to start the main application.
- It first checks if a valid application exists by checking the content of the address `0x08008004` (the application's reset handler address). If the value is `0xFFFFFFFF`, the memory is considered erased and there is no application to start.
- If a valid application is found, the bootloader performs the following steps to jump to it:
    1.  **Set the Main Stack Pointer (MSP):** The bootloader reads the application's initial MSP value from the start of the application's vector table, located at `0x08008000`. It then sets the microcontroller's MSP register to this value using the `__set_MSP()` intrinsic function. This is critical for the application to function correctly as it sets up its own stack.
    2.  **Jump to Application:** The bootloader gets the application's entry point address (the address of its Reset Handler) from `0x08008004`. It then calls this address as a function pointer, effectively transferring control to the main application.

## Application Code Configuration
- The application starts at address 0x08008000 (Flash Sector 2), change the start address in the project settings.
- This ensures the application’s vector table and code are placed at the correct offset and do not overlap with the bootloader.

- The linker script *STM32F446RETX_FLASH.ld* should look like this:
    ```sh
    MEMORY
    {
    RAM    (xrw)    : ORIGIN = 0x20000000,   LENGTH = 128K
    /*FLASH    (rx)    : ORIGIN = 0x8000000,   LENGTH = 512K*/
    FLASH    (rx)    : ORIGIN = 0x8008000,   LENGTH = 480K
    }
    ```
