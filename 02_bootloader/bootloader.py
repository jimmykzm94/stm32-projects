import intelhex
import binascii
import serial
import serial.tools.list_ports

file = 'example.hex'

def main():
    ports = serial.tools.list_ports.comports()
    if len(ports) == 0:
        print('No ports')
        return
    
    # check stm32 port
    port = [p[0] for p in ports if 'STM32' in p[1]]
    if len(port) == 0:
        print('No STM32 port is connected')
        return

    ser = serial.Serial(port[0], 115200)
    try:
        while True:
            # user input
            cmd = input('Insert command: ')
            if cmd == 'q':
                print('Exit...')
                break

            elif cmd == 'help':
                print('Help command:')
                print('0x11: Erase appplication.')
                print('0x21: Flashing application')

            elif cmd == '0x11':
                print('Sending command to MCU')

                ser.write(cmd.encode())
                res = ser.readline()
                msg = res.decode().strip()
                if msg:
                    print(f'Response: {msg}')
            
            elif cmd == '0x21':
                print('Start flashing')

                ih = intelhex.IntelHex(file)
                
                i = 0
                addrs = ih.addresses()
                ser.flush()
                while i < len(addrs):
                    # Max length available
                    size = 180
                    max_transfer_len = len(addrs) - size
                    if max_transfer_len > size:
                        max_transfer_len = size
                    data = bytearray(cmd.encode())
                    hex_addr = binascii.unhexlify(format(addrs[i], '#010x')[2:])
                    data.extend(hex_addr)
                    data.append(max_transfer_len)
                    data.extend(ih.tobinstr(start=addrs[i],size=max_transfer_len))
                    i += max_transfer_len
                    
                    ser.write(data)
                    res = ser.readline()
                    msg = res.decode().strip()
                    if msg:
                        if 'ACK' in msg:
                            print('.', end='', flush=True)
                        else:
                            print('Error flashing.')
                            break
         
                print('Done, please reset the board.')
    except Exception as e:
        print(e)
        exit()
    finally:
        ser.close()

if __name__ == '__main__':
    main()
