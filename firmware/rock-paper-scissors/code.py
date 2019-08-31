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

def show_frame(group, refresh = True):
    display.show(group)
    while refresh:
        if display.time_to_refresh == 0:
            display.refresh()
            break

def large_label(text, x = 0, y = 0, scale = 2):
    group = displayio.Group(scale=scale, x = x, y = y)
    group.append(Label(terminalio.FONT, text=text))
    return group

def wait_for_button_release():
    while pad.get_pressed():
        pass

def create_controls(controls, base_x = 30, stride_x = 100, base_y = 120):
    frame = displayio.Group(max_size=3, x = base_x, y = base_y)
    for idx, label in enumerate(controls):
        frame.append(Label(terminalio.FONT, text=label, x = stride_x * idx))
    return frame

def create_menu(menu_name, options, selected_index):
    option_base_x = 100
    option_stride_x = 95
    option_base_y = 60
    option_stride_y = 15

    controls = ['UP', 'SELECT', 'DOWN']

    frame = displayio.Group(max_size=13)

    frame.append(large_label(menu_name, x=35, y=20))

    number_of_options = len(options)
    extended_menu = 0
    if number_of_options > 3:
        extended_menu = 1
        option_base_x = 30
        
    for i in range(number_of_options):
        option_text = '-> ' if i == selected_index else ''
        option_text += options[i]
        option_label = Label(terminalio.FONT, text=option_text)
        x = i // 3
        y = i % 3
        option_label.x = option_base_x + option_stride_x * x * extended_menu
        option_label.y = option_base_y + option_stride_y * y
        frame.append(option_label)

    frame.append(create_controls(controls))
    return frame

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
            
            if selection_complete:
                wait_for_button_release()
                return selected_index

        if should_refresh:
            wait_for_button_release()
            show_frame(create_menu(menu_name, options, selected_index))
            should_refresh = False

def create_status(status, controls = None):
    frame = displayio.Group()

    connected_label = Label(terminalio.FONT, text=status)
    connected_label.x = 80
    connected_label.y = 60
    frame.append(connected_label)

    if controls:
        frame.append(create_controls(controls))

    return frame

device_name = 'rps-{}'.format(str(binascii.hexlify(bytearray(reversed(bleio.adapter.address.address_bytes[0:2]))), 'utf-8'))
uart_server = UARTServer(name=device_name)

def host_game():
    uart_server.start_advertising()
    show_frame(create_status('Hosting on {}'.format(device_name), [' ', 'CANCEL']))
    while not uart_server.connected:
        if pad.get_pressed() & BUTTON_MIDDLE:
            uart_server.stop_advertising()
            wait_for_button_release()
            return False
    uart_server.stop_advertising()
    return uart_server

uart_client = UARTClient()

def join_game(game_host):
    uart_client.connect(game_host.address, 5)
    while not uart_client.connected:
        pass
    return uart_client

def run_game(uart):
    show_frame(create_game_menu())
    selected_game_option = run_game_menu()
    badge.pixels.fill((0, 10, 0))
    badge.vibration = True
    send_choice(uart, selected_game_option)
    time.sleep(0.25)
    badge.vibration = False
    other_player_choice = receive_choice(uart)
    badge.pixels.fill((0, 0, 0))
    return (selected_game_option, other_player_choice)

def create_game_menu():
    frame = displayio.Group(max_size = 6)

    base_x = 30
    base_y = 34
    current_x = base_x
    for i in range(len(icons)):
        grid = displayio.TileGrid(icons[i], pixel_shader=palette, x=current_x, y=base_y)
        frame.append(grid)
        current_x += 90

    return frame

def create_game_over_menu(game_result, my_choice, opponent_choice):
    frame = displayio.Group(max_size=10)

    game_options = ['Rock', 'Paper', 'Scissors']
    game_summary = '{} vs {}'.format(game_options[my_choice], game_options[opponent_choice])

    frame.append(large_label(game_result, x=90, y=20))
    frame.append(Label(terminalio.FONT, text=game_summary, x=94, y=50))
    frame.append(large_label('Rematch?', x=90, y=80))
    frame.append(create_controls(['YES', ' ', 'NO']))
    return frame

def run_game_over(game_result, my_choice, opponent_choice):
    show_frame(create_game_over_menu(game_result, my_choice, opponent_choice))

    while True:
        buttons = pad.get_pressed()
        if buttons & BUTTON_LEFT:
            wait_for_button_release()
            return True
        elif buttons & BUTTON_RIGHT:
            wait_for_button_release()
            return False

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

        wait_for_button_release()

        if selection != -1:
            return selection

def send_choice(uart, index):
    buttons = [ButtonPacket.BUTTON_1, ButtonPacket.BUTTON_2, ButtonPacket.BUTTON_3]
    button_packet = ButtonPacket(buttons[index], True)
    for i in range(3):
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

def setup_game():
    selected_host = None
    while selected_host is None:
        game_host_options = ['Host game', 'Rescan']
        game_hosts = scan_for_games()
        if not game_hosts is None:
            game_host_options = [str(game_host.name, 'utf-8') for game_host in game_hosts] + game_host_options
        
        selected_host_option = run_menu(menu_name='Rock Paper Scissors', options=game_host_options)
        host_option_name = game_host_options[selected_host_option]
        if host_option_name == 'Host game':
            game = host_game()
            if game:
                return game
        elif host_option_name == 'Rescan':
            continue
        else:
            host_name = next(game_host for game_host in game_hosts if str(game_host.name, 'utf-8') == host_option_name)
            return join_game(host_name)

def main():
    while True:
        uart = setup_game()
        while True:
            my_choice, opponent_choice = run_game(uart)
            game_result = resolve_game(str([my_choice, opponent_choice]))
            play_again = run_game_over(game_result, my_choice, opponent_choice)
            send_choice(uart, play_again)
            if play_again:
                play_again = receive_choice(uart)
            if not play_again:
                show_frame(create_status('Disconnecting...'))
                break
        uart.disconnect()
        time.sleep(2)

if __name__ == '__main__':
    main()
