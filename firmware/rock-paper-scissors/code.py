from badgeio import badge
import time
import binascii

import gamepad

from adafruit_ble import BLERadio
from adafruit_ble.advertising import Advertisement
from adafruit_ble.advertising.standard import ProvideServicesAdvertisement
from adafruit_ble.services.nordic import UARTService

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

WIN = 'You won'
LOSE = 'You Lost'
TIE =  "It's a tie"

palette = displayio.Palette(2)
palette[0] = 0xffffff
palette[1] = 0

icons = [
    displayio.OnDiskBitmap(open("/res/hand-rock-regular.bmp", "rb")),
    displayio.OnDiskBitmap(open("/res/hand-paper-regular.bmp", "rb")),
    displayio.OnDiskBitmap(open("/res/hand-scissors-regular.bmp", "rb"))]

pad = gamepad.GamePad(
    badge._left,
    badge._middle,
    badge._right)

ble = BLERadio()
uart_service = UARTService()
host_advertisement = ProvideServicesAdvertisement(uart_service)
device_name = 'rps-{}'.format(str(binascii.hexlify(bytearray(reversed(ble.address_bytes[0:2]))), 'utf-8'))
host_advertisement.complete_name = device_name

def create_large_label(text, x = 0, y = 0, scale = 2):
    group = displayio.Group(scale=scale, x = x, y = y)
    group.append(Label(terminalio.FONT, text=text))
    return group

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

    frame.append(create_large_label(menu_name, x=35, y=20))

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

def create_game_over_menu(match_results, game_result, my_choice, opponent_choice):
    frame = displayio.Group(max_size=10)

    game_options = ['Rock', 'Paper', 'Scissors']
    game_summary = '{} vs {}'.format(game_options[my_choice], game_options[opponent_choice])

    frame.append(create_large_label(game_result, x=90, y=20))
    frame.append(create_large_label(text=game_summary, x=50, y=50))
    frame.append(create_large_label('Rematch?', x=90, y=80))
    match_results_label = Label(
        terminalio.FONT,
        text='W-{}/L-{}/T-{}'.format(match_results[WIN], match_results[LOSE], match_results[TIE]))
    match_results_label.x = 110
    match_results_label.y = 110
    frame.append(match_results_label)
    frame.append(create_controls(['YES',' ','NO']))
    return frame

def create_status(status, controls = None):
    frame = displayio.Group()

    connected_label = Label(terminalio.FONT, text=status)
    connected_label.x = 80
    connected_label.y = 60
    frame.append(connected_label)

    if controls:
        frame.append(create_controls(controls))

    return frame

def show_frame(group, refresh = True):
    display.show(group)
    while refresh:
        if display.time_to_refresh == 0:
            display.refresh()
            time.sleep(0.1)
            break

def wait_for_button_release():
    while pad.get_pressed():
        pass

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

def host_game():
    show_frame(create_status('Hosting on {}'.format(device_name), [' ', 'CANCEL']))
    ble.start_advertising(host_advertisement)
    while not ble.connected:
        if pad.get_pressed() & BUTTON_MIDDLE:
            ble.stop_advertising()
            wait_for_button_release()
            return False
    return uart_service

def join_game(host_adv):
    uart_connection = ble.connect(host_adv)
    while not uart_connection.connected:
        pass
    return uart_connection[UARTService]

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

def run_game(uart):
    show_frame(create_game_menu())
    selected_game_option = run_game_menu()
    badge.pixels.fill((10, 0, 10))
    badge.vibration = True
    send_choice(uart, selected_game_option)
    time.sleep(0.25)
    badge.vibration = False
    other_player_choice = receive_choice(uart)
    badge.pixels.fill((0, 0, 0))
    return (selected_game_option, other_player_choice)

def run_game_over_menu(match_results, game_result, my_choice, opponent_choice):
    game_results_colors = {
        WIN: (0, 10, 0),
        LOSE: (10, 0, 0),
        TIE: (10, 10, 0)
    }
    play_again = False
    selection_complete = False

    show_frame(create_game_over_menu(match_results, game_result, my_choice, opponent_choice))
    badge.pixels.fill(game_results_colors[game_result])

    while True:
        buttons = pad.get_pressed()
        if buttons & BUTTON_LEFT:
            selection_complete = True
            play_again = True
        elif buttons & BUTTON_RIGHT:
            selection_complete = True
            play_again = False

        if selection_complete:
            wait_for_button_release()
            badge.pixels.fill((0,0,0))
            return play_again

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
    hosts = {}
    for adv in ble.start_scan(ProvideServicesAdvertisement, timeout=5):
        if UARTService in adv.services:
            if not adv.complete_name:
                continue
            name = adv.complete_name
            if name.startswith('rps') and name not in hosts:
                hosts[name] = adv
                print("Found host {}".format(name))
    return hosts

def setup_game():
    selected_host = None
    while selected_host is None:
        game_host_options = ['Host game', 'Rescan']
        game_hosts = scan_for_games()
        if not game_hosts is None:
            game_host_options = [host_name for host_name in game_hosts] + game_host_options
        
        selected_host_option = run_menu(menu_name='Rock Paper Scissors', options=game_host_options)
        host_option_name = game_host_options[selected_host_option]
        if host_option_name == 'Host game':
            game = host_game()
            if game:
                return game
        elif host_option_name == 'Rescan':
            continue
        else:
            host_adv = game_hosts[host_option_name]
            return join_game(host_adv)

def main():
    while True:
        uart = setup_game()
        match_results = {WIN :0, LOSE: 0, TIE: 0}
        while True:
            my_choice, opponent_choice = run_game(uart)
            game_result = resolve_game(str([my_choice, opponent_choice]))
            match_results[game_result] += 1
            play_again = run_game_over_menu(match_results, game_result, my_choice, opponent_choice)
            send_choice(uart, play_again)
            if play_again:
                play_again = receive_choice(uart)
            if not play_again:
                show_frame(create_status('Disconnecting...'))
                break
        for connection in ble.connections:
            connection.disconnect()
        time.sleep(2)

if __name__ == '__main__':
    main()
