#! /usr/bin/env python

"""
 +-----------------------------------+
 | Jetpack Cognition Lab, Inc.       |
 | Supreme Machines Sensorimotor     |
 | Sensorimotor Firmware Updater     |
 | Author: Matthias Kubisch          |
 | kubisch@informatik.hu-berlin.de   |
 | Last Update: January 2021         |
 +-----------------------------------+

 for usage help, type --help

 supported devices:
  + sensorimotor rev1.1 (atmega328p)
  + sensorimotor rev1.2 (atmega328pb)

"""

import serial, argparse, sys, os, shlex
from subprocess import Popen, PIPE
from time import sleep
from struct import *
from array import array

default_port = '/dev/ttyUSB0'
default_baud = 1000000 # 1MBaud
default_dev  = 'sensorimotor_rev1.1'

debug = False

devicelist = {
    'sensorimotor_rev1.1': {
                                'mcu_name' : "atmega328p",
                                'signature': "\x1E\x95\x0F",
                                'pagesize' : 128,
                           }
                           ,
    'sensorimotor_rev1.2': {
                                'mcu_name' : "atmega328pb",
                                'signature': "\x1E\x95\x16",
                                'pagesize' : 128,
                           }
}

logo = r"""
    jetpack cognition lab's
    _______  ______  ________  ____ ___  ___
   / ___/ / / / __ \/ ___/ _ \/ __ `__ \/ _ \
  /__  / /_/ / /_/ / /  /  __/ / / / / /  __/
 /____/\__,_/ .___/_/   \___/_/ /_/ /_/\___/
           /_/sensorimotor firmware updater
"""

usage = " Type --help to display instructions.\n___\n"


def response(mgs):
    print('\033[1m> {}\033[0m'.format(mgs))


def dbg_msg(mgs):
    if debug:
        print("\033[95m> {}\033[0m".format(mgs))


def wrn_msg(mgs):
    print("\033[93m> {}\033[0m".format(mgs))


def progressbar(dc):
    color = "\033[93m" if (dc < 1) else "\033[1m"
    sys.stdout.write("\r{2}[{0:40}] {1:5.1f}% \033[0m".format('/'*int(round(40*dc)), round(100*dc,1), color))
    sys.stdout.flush()


def checksum(data):
    chksum = 0
    words = array('H', data)
    for w in words:
        chksum = chksum + w

    # fold the sum into 16 bits
    while (chksum >> 16):
        chksum = (chksum & 0xffff) + (chksum >> 16)

    return pack('<H', chksum)


def get_byte(ser, data):
    c = ser.read()
    if len(c) == 0:
        dbg_msg("timeout")
        return None
    resp = ord(c) # read response byte
    data.append(resp) # add to list
    #print(resp)
    return resp


def recv_packaged(ser, bid):
    data = []
    trials = 0
    #TODO try/except KeyboardInterrupt
    while (trials < 3):
        trials += 1
        if (255 != get_byte(ser, data)): continue
        if (255 != get_byte(ser, data)): continue
        if ( 86 != get_byte(ser, data)): continue
        if (bid != get_byte(ser, data)): continue

        for i in range(0, get_byte(ser, data)):
            get_byte(ser, data)

        get_byte(ser, data) # get checksum itself
        cs = sum(data) % 256
        if (cs == 0):
            break # received valid package?


    result = data[5:-1]
    dbg_msg(data)
    return result # without 2*sync, cmd, id, len and cs


def send_packaged(ser, bid, msg):
    sendbuf = [255,255,85,bid,len(msg)] + [ord(c) for c in msg]
    checksum = (~sum(sendbuf) + 1) % 256
    sendbuf.append(checksum)
    assert(sum(sendbuf) % 256 == 0)
    dbg_msg(sendbuf)
    ser.write("".join(chr(e) for e in sendbuf))


def send_cmd(ser, bid, msg, expect_reply=True, verbose=True):
    send_packaged(ser, bid, msg)
    if not expect_reply:
        return

    reply = recv_packaged(ser, bid)
    if (len(reply) == 0):
        response("Failed. (Timeout)")
        sys.exit(1)

    if (reply == [ord('Y')]):
        if verbose:
            response("OK")
        return

    if (reply == [ord('N')]):
        response("Failed. Request rejected by device.")
    else:
        wrn_msg("Error: Unknown Response: {}".format(reply))
    sys.exit(1)


def verify_signature(ser, bid, device):
    hexsig = " ".join(hex(ord(n)) for n in device['signature'])
    print("Verifying device signature.\n\tmcu = {0}\n\tID = {1}"
         .format(device['mcu_name'], hexsig))
    send_cmd(ser, bid, 'I' + device['signature'])


def flash_file(ser, bid, device, filename):
    print("Now flashing file {0} to device: ".format(filename))

    try:
        f = open(filename, "rb")
    except:
        wrn_msg("Error opening file: {}".format(filename))
        sys.exit(3)

    eof = False
    bytes_written = 0
    binary_size = os.path.getsize(filename)
    expected_size = binary_size + (128 - binary_size%128)

    while (not eof):
        buffer = array('c')
        bytecount = 0
        while (bytecount < device['pagesize']):
            data = f.read(1)
            if (len(data) == 0):
                eof = True
                break
            buffer.append(data)
            bytecount = bytecount + 1

        # fill remaining part of page with zeros
        if (bytecount != device['pagesize']):
            while (bytecount < device['pagesize']):
                buffer.append(chr(0))
                bytecount = bytecount + 1

        bufferstr = buffer.tostring()
        chksum = checksum(bufferstr)
        dbg_msg("Writing {0} bytes to device flash.".format(bytecount))
        send_cmd(ser, bid, 'W' + chksum + bufferstr, verbose = False)
        bytes_written += bytecount
        progressbar(bytes_written/float(expected_size))
    response("Done.")


def reset(ser, bid):
    print("Resetting device.")
    send_cmd(ser, bid, 'R', False)


def start_bootloader(ser, bid):
    print("Starting bootloader of board with ID {}.".format(bid))
    send_cmd(ser, bid, 'S')
    sleep(.1) # remove/adjust when ACK from bootloader is ready


def execute_shell_command(command):
    args = shlex.split(command)
    proc = Popen(args, stdout=PIPE, stderr=PIPE)
    out, err = proc.communicate()
    exitcode = proc.returncode
    return exitcode, out, err


def convert_elf(filename):
    # avr-objcopy -R .fuse -R .eeprom -R .lock -R .signature -S -O binary sensorimotor.elf sensorimotor.bin
    if filename.endswith('.elf'):
        wrn_msg("Automatically converting ELF ({0}) to binary file format.".format(filename))
        name = filename[:-4]
        try:
            [c,_,_] = execute_shell_command("avr-objcopy -R .fuse -R .eeprom -R .lock -R .signature -S -O binary {0}.elf {0}.bin".format(name))
            if c == 0:
                wrn_msg("Succeeded.")
                return name+".bin"
        except:
            wrn_msg("Failed. No avr-objcopy.")
        wrn_msg("Failed to convert, ELF-file rejected.")
        return ""
    return filename


def main():
    print("\033[93m"+logo+"\033[0m")
    print(usage)

    parser = argparse.ArgumentParser( description ="Sensorimotor Firmware-Updater"
                                    #, epilog = "..."
                                    #, formatter_class=argparse.ArgumentDefaultsHelpFormatter
                                    )
    parser.add_argument('-b', '--board_id', help = "board id = 0..127, take 127 if no ID is set yet")
    parser.add_argument('-p', '--port'    , help = "serial port"          , default=default_port    )
    parser.add_argument('-d', '--device'  , help = "sensorimotor version" , default=default_dev     )
    parser.add_argument('-f', '--flash'   , help = "flash firmware binary"                          )
    parser.add_argument('-v', '--verify'  , help = "verify device by signature", action='store_true')
    parser.add_argument('-r', '--reset'   , help = "reset device"              , action='store_true')
    parser.add_argument('-x', '--debug'   , help = "show debug output"         , action='store_true')

    args = parser.parse_args()
    #parser.print_help()

    global debug
    debug = args.debug

    if args.board_id:
        bid = int(args.board_id)
        if 0 <= bid <= 127:
            pass
        else:
            sys.exit("Error invalid board id {}".format(args.board_id))
    else:
        wrn_msg("No Board ID provided. Check --help for usage details.")
        sys.exit(3)


    try:
        dev = devicelist[args.device]
    except KeyError:
        wrn_msg("Error: Device '{0}' is not supported.\n\nSupported devices:\n\t{1}".format(args.device,"\n\t".join(devicelist.keys())))
        sys.exit(3)

    try:
        ser = serial.Serial(port=args.port, baudrate=default_baud, timeout=1)
        ser.flushInput()
        ser.flushOutput()
    except:
        wrn_msg("Error: Serial device {} not found/connected.".format(args.port))
        sys.exit(3)

    if args.verify:
        start_bootloader(ser, bid)
        verify_signature(ser, bid, dev)

    if args.flash:
        start_bootloader(ser, bid)
        verify_signature(ser, bid, dev)
        flash_file(ser, bid, dev, convert_elf(args.flash))
        reset(ser, bid)

    if args.reset:
        start_bootloader(ser, bid)
        reset(ser, bid)

    ser.close()
    print("\n____\nDONE.")


if __name__ == "__main__": main()
# end sensorimotor bootload script

