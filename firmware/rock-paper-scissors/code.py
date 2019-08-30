"""
Used with ble_demo_central.py. Receives Bluefruit LE ColorPackets from a central,
and updates a NeoPixel FeatherWing to show the history of the received packets.
"""
from badgeio import badge
import time
import binascii

import gamepad

import bleio
from adafruit_ble.scanner import Scanner, ScanEntry
from adafruit_ble.uart import NUS_SERVICE_UUID
from adafruit_ble.uart_client import UARTClient
from adafruit_ble.uart_server import UARTServer
from adafruit_bluefruit_connect.packet import Packet
from adafruit_bluefruit_connect.button_packet import ButtonPacket

import displayio
from adafruit_display_text.label import Label
from adafruit_display_shapes.rect import Rect
import terminalio

display = badge.display

BUTTON_LEFT = 1 << 0
BUTTON_MIDDLE = 1 << 1
BUTTON_RIGHT = 1 << 2

pad = gamepad.GamePad(
    badge._left,
    badge._middle,
    badge._right
)

palette = displayio.Palette(2)
palette[0] = 0xffffff
palette[1] = 0

icons = [
    displayio.OnDiskBitmap(open("/res/hand-rock-regular.bmp", "rb")),
    displayio.OnDiskBitmap(open("/res/hand-paper-regular.bmp", "rb")),
    displayio.OnDiskBitmap(open("/res/hand-scissors-regular.bmp", "rb"))]

def render_menu(menu_name, options, selected_index):
    g = displayio.Group()

    base_x = 100
    base_y = 60
    stride_y = 20
    menu_name_label = Label(terminalio.FONT, text=menu_name)
    menu_name_label.x = 80
    menu_name_label.y = 20
    g.append(menu_name_label)

    for i in range(len(options)):
        option_text = '-> ' if i == selected_index else ''
        option_text += options[i]
        label = Label(terminalio.FONT, text=option_text)
        label.x = base_x
        label.y = base_y + stride_y * i
        g.append(label)
    display.show(g)

def run_menu(menu_name, options):
    should_refresh = True
    selection_complete = False
    selected_index = 0
    menu_size = len(options)

    while True:
        if not should_refresh:
            buttons = pad.get_pressed()
            if buttons & BUTTON_RIGHT:
                selected_index += 1
                selected_index = selected_index % menu_size
                should_refresh = True
            elif buttons & BUTTON_LEFT:
                selected_index -= 1
                selected_index = selected_index % menu_size
                should_refresh = True
            elif buttons & BUTTON_MIDDLE:
                selection_complete = True

            while buttons:
                # Wait for all buttons to be released.
                buttons = pad.get_pressed()
                time.sleep(0.1)
            
            if selection_complete:
                return selected_index

        if should_refresh and (display.time_to_refresh == 0):
            render_menu(menu_name, options, selected_index)
            display.refresh()
            should_refresh = False

def render_status(status):
    g = displayio.Group()

    connected_label = Label(terminalio.FONT, text=status)
    connected_label.x = 80
    connected_label.y = 60
    g.append(connected_label)
    display.show(g)
    while True:
        if display.time_to_refresh == 0:
            display.refresh()
            break

device_name = 'rps-{}'.format(str(binascii.hexlify(bytearray(reversed(bleio.adapter.address.address_bytes[0:2]))), 'utf-8'))
uart_server = UARTServer(name=device_name)

def host_game():
    
    uart_server.start_advertising()
    render_status('Hosting on {}'.format(device_name))
    while not uart_server.connected:
        pass
    uart_server.stop_advertising()
    run_game(uart_server)
    uart_server.disconnect()
    time.sleep(5)

uart_client = UARTClient()

def join_game(game_host):
    uart_client.connect(game_host.address, 5)
    if uart_client.connected:
        run_game(uart_client)
        uart_client.disconnect()
        time.sleep(5)

def run_game(uart):
    game_options = ['Rock', 'Paper', 'Scissors']
    render_game_menu()
    selected_game_option = run_game_menu()
    send_choice(uart, selected_game_option)
    other_player_choice = receive_choice(uart)
    game_result = resolve_game(str([selected_game_option, other_player_choice]))
    render_status('{} |{} vs {}|'.format(game_result, game_options[selected_game_option], game_options[other_player_choice]))

def render_game_menu():
    g = displayio.Group(max_size = 6)

    base_x = 30
    base_y = 34
    current_x = base_x
    for i in range(len(icons)):
        grid = displayio.TileGrid(icons[i], pixel_shader=palette, x=current_x, y=base_y)
        g.append(grid)
        current_x += 90

    display.show(g)

    while True:
        if display.time_to_refresh == 0:
            display.refresh()
            break
        time.sleep(1)

def run_game_menu():
    selection = -1
    while True:
        buttons = pad.get_pressed()
        if buttons & BUTTON_LEFT:
            selection = 0
        elif buttons & BUTTON_MIDDLE:
            selection = 1
        elif buttons & BUTTON_RIGHT:
            selection = 2

        while buttons:
            # Wait for all buttons to be released.
            buttons = pad.get_pressed()
            time.sleep(0.1)

        if selection != -1:
            return selection

def send_choice(uart, index):
    buttons = [ButtonPacket.BUTTON_1, ButtonPacket.BUTTON_2, ButtonPacket.BUTTON_3]
    button_packet = ButtonPacket(buttons[index], True)
    while True:
        try:
            uart.write(button_packet.to_bytes())
            break
        except OSError:
            pass
        time.sleep(0.3)

def receive_choice(uart):
    while True:
        packet = Packet.from_stream(uart)
        if isinstance(packet, ButtonPacket):
            return [ButtonPacket.BUTTON_1, ButtonPacket.BUTTON_2, ButtonPacket.BUTTON_3].index(packet.button)

WIN = 'You won'
LOSE = 'You Lost'
TIE =  "It's a tie"

def resolve_game(choices):
    options = {
        '[0, 0]': TIE,
        '[0, 1]': LOSE,
        '[0, 2]': WIN,
        '[1, 0]': WIN,
        '[1, 1]': TIE,
        '[1, 2]': LOSE,
        '[2, 0]': LOSE,
        '[2, 1]': WIN,
        '[2, 2]': TIE}
    return options[choices]

def scan_for_games():
    scanner = Scanner()
    uart_addresses = []

    uart_addresses = ScanEntry.with_service_uuid(scanner.scan(2), NUS_SERVICE_UUID)
    if uart_addresses is None:
        return None
    return [se for se in uart_addresses if str(se.name, 'utf-8').startswith('rps')]

def play_game():
    while True:
        game_host_options = ['Host game']
        game_hosts = scan_for_games()
        if not game_hosts is None:
            game_host_options += [str(game_host.name, 'utf-8') for game_host in game_hosts]
        
        selected_host_option = run_menu(menu_name='Rock Paper Scissors', options=game_host_options)
        host_option_name = game_host_options[selected_host_option]
        print(host_option_name)

        
        if host_option_name == 'Host game':
            host_game()
        else:
            host_name = next(game_host for game_host in game_hosts if str(game_host.name, 'utf-8') == host_option_name)
            join_game(host_name)

def main():
    play_game()

if __name__ == '__main__':
    main()
