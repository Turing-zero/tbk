import os
import signal
signal.signal(signal.SIGINT, signal.SIG_DFL)
import gi
gi.require_version('Gtk', '3.0')
gi.require_version('AppIndicator3', '0.1')
gi.require_version('Notify', '0.7')
from gi.repository import Gtk as gtk
from gi.repository import AppIndicator3 as appindicator
from gi.repository import GLib as glib
from gi.repository import Notify as notify

APPINDICATOR_ID = 'indicatorfromtbk'

import time
from threading import Thread
import json
from urllib.request import urlopen, Request

class MainWindow(gtk.Window):
    def __init__(self):
        super().__init__(title="tbkadm")
        self.set_default_size(800, 600)
        self.set_position(gtk.WindowPosition.CENTER)
        self.button = gtk.Button(label="Click Me")
        self.button.connect("clicked", self.on_button_clicked)
        self.add(self.button)
    
    def on_button_clicked(self, widget):
        print(f"Hello World, {widget}")

class Indicator():
    def __init__(self):
        self.iconpath = os.path.abspath("tbk_dark.png")
        self.indicator = appindicator.Indicator.new(APPINDICATOR_ID, self.iconpath, appindicator.IndicatorCategory.SYSTEM_SERVICES)
        self.indicator.set_status(appindicator.IndicatorStatus.ACTIVE)
        self.indicator.set_menu(self._create_menu())
        self.indicator.set_label("TBK", APPINDICATOR_ID)
    def _create_menu(self):
        menu = gtk.Menu()

        item_getIP = gtk.MenuItem(label='Get IP')
        item_getIP.connect('activate', self._test)
        menu.append(item_getIP)

        item_window = gtk.MenuItem(label='Open Window')
        item_window.connect('activate', self._create_win)
        menu.append(item_window)

        item_quit = gtk.MenuItem(label='Quit')
        item_quit.connect('activate', self._quit)
        menu.append(item_quit)

        menu.show_all()
        return menu
    
    def _quit(self, source):
        notify.uninit()
        gtk.main_quit()

    def _test(self, source):
        request = Request('https://ipinfo.io/')
        response = urlopen(request)
        ipinfo = json.loads(response.read())
        notify.Notification.new("IP Info", f"IP: {ipinfo['ip']}\nCity: {ipinfo['city']}\nRegion: {ipinfo['region']}\nCountry: {ipinfo['country']}\nLocation: {ipinfo['loc']}\nOrganization: {ipinfo['org']}", None).show()

    def resetLabel(self,label,iconpath=None):
        glib.idle_add(
            self.indicator.set_label,
            label, APPINDICATOR_ID,
            priority=glib.PRIORITY_DEFAULT
        )
        if iconpath:
            glib.idle_add(
                self.indicator.set_icon,
                iconpath,
                priority=glib.PRIORITY_DEFAULT
            )
    def _create_win(self, source):
        self.window = MainWindow()
        self.window.show_all()
def main():
    indicator = Indicator()
    notify.init(APPINDICATOR_ID)
    Thread(target=run, args=(indicator,)).start()
    gtk.main()

def run(indicator):
    count = 0
    while True:
        time.sleep(1)
        iconpath = os.path.abspath("tbk_dark.png") if count % 2 == 0 else os.path.abspath("tbk_light.png")
        indicator.resetLabel(f"TBK-{count}",iconpath)
        count += 1

if __name__ == "__main__":
    main()