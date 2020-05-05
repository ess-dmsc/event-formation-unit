#!/usr/local/bin/python3.7

from PyQt5.QtWidgets import *
from PyQt5 import QtCore
from os.path import expanduser
import sys, os, re, subprocess, signal, configparser

#
# #
#
class Configuration:
    def __init__(self):
        self.options = {}
        self.config = configparser.RawConfigParser()
        self.config.read(os.path.join(expanduser("~"), ".efucfg"))
        self._load('directories', 'basedir', expanduser("~"))
        self._load('directories', 'efudir', '/essproj/event-formation-unit')
        self._load('directories', 'datadir', '/ownCloud/DM/data/EFU_reference')
        self._load('servers', 'grafana', '172.17.12.31')
        self._load('servers', 'kafka', '172.17.5.38:9092')
        self._load('servers', 'efu', '127.0.0.1')
        self._load('datagen', 'throttle', '100')

    # load option or use defult value
    def _load(self, group, opt, default):
        if self.config.has_option(group, opt):
            self.options[opt] = self.config.get(group, opt)
        else:
            self.options[opt] = default

#
# #
#
class Directories:
    def __init__(self, base, efu, data):
        self.set_dirs(base, efu, data)

    def set_dirs(self, base, efu, data):
        self.basedir = base
        self.efudir = efu
        self.datadir = data
        self.searchdirs = [self.basedir + self.datadir, self.basedir + self.efudir]

#
# #
#
class Searcher:
    def __init__(self, dirs):
        self.dirs = dirs

    def find_files(self, dirs, extension, exclude):
        results = []
        for dir in dirs:
            for r, s, f in os.walk(dir):
                if re.search(exclude, r):
                    continue
                for file in f:
                    if re.search(extension, file):
                        filepath = os.path.join(r, file)
                        results += [filepath]
        return results

    def get_values(self):
        data = []
        data += self.find_files(self.dirs.searchdirs, '\.h5', '-X--xXX')
        data += self.find_files(self.dirs.searchdirs, '\.pcap', '-X--xXX')

        return [ self.find_files(self.dirs.searchdirs, 'gen_', '-X--xXX'), data]

#
# #
#
class Dialog(QDialog): #WMainWindow
    def __init__(self, configuration, directories):
        super(QDialog, self).__init__()
        self.dirs = directories
        self.cfg = configuration
        self.create_layout()

        buttonBox = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        buttonBox.accepted.connect(self.on_accepted)
        buttonBox.rejected.connect(self.reject)

        mainLayout = QVBoxLayout()
        mainLayout.addWidget(self.config_group_box)
        mainLayout.addWidget(self.files_group_box)
        mainLayout.addWidget(self.options_box)
        mainLayout.addWidget(buttonBox)
        self.setLayout(mainLayout)

        self.setWindowTitle("Launch Event Formation Unit")

    # Add a 'row' consisting of a label and a text box widget
    def add_row(self, layout, label, type):
        lbl = QLabel(label)
        type.setMinimumWidth(600)
        layout.addRow(lbl, type)

    def create_layout(self):
        self.config_group_box = QGroupBox("Directories")
        toplayout = QFormLayout()

        self.basedirle = QLineEdit(self.dirs.basedir)
        self.add_row(toplayout, "basedir:", self.basedirle)
        self.efudirle = QLineEdit(self.dirs.efudir)
        self.add_row(toplayout, "efu dir:", self.efudirle)
        self.datadirle = QLineEdit(self.dirs.datadir)
        self.add_row(toplayout, "data dir:", self.datadirle)
        updateb = QPushButton("Reload")
        updateb.setMaximumWidth(100)
        updateb.clicked.connect(self.update)
        toplayout.addRow(updateb)
        self.config_group_box.setLayout(toplayout)

        self.files_group_box = QGroupBox("Select configuration")
        fileslayout = QFormLayout()
        self.gencb = QComboBox()
        self.add_row(fileslayout, "EFU:", self.gencb)
        self.datacb = QComboBox()
        self.add_row(fileslayout, "Detector:", self.datacb)
        self.files_group_box.setLayout(fileslayout)

        self.options_box = QGroupBox("Options")
        optslayout = QFormLayout()
        self.efuiple = QLineEdit(cfg.options['efu'])
        self.add_row(optslayout, "EFU:", self.efuiple)
        self.throttlele = QLineEdit(cfg.options['throttle'])
        self.add_row(optslayout, "throttle:", self.throttlele)
        self.options_box.setLayout(optslayout)

    def _populate_field(self, field, list):
        for name in list:
            field.addItem(name)

    def populate(self, e, d):
        self._populate_field(self.gencb, e)
        self._populate_field(self.datacb, d)

    def get_selection(self):
        return self._gen, self._data, self._efuip, self._throttle

    # Override builtin on_accepted method for pressing OK button
    def on_accepted(self):
        self._gen = self.gencb.itemText(self.gencb.currentIndex())
        self._data = self.datacb.itemText(self.datacb.currentIndex())
        self._efuip = self.efuiple.text()
        self._throttle = self.throttlele.text()
        self.accept()

    def update(self):
        self.efucb.clear()
        self.detcb.clear()
        #self.cfgcb.clear()

        self.dirs.set_dirs(self.basedirle.text(), self.efudirle.text(), self.datadirle.text())
        search = Searcher(self.dirs)
        g, d = search.get_values()
        self.populate(g, d)

#
#
if __name__ == '__main__':
    signal.signal(signal.SIGINT, signal.SIG_DFL)
    app = QApplication(sys.argv)

    cfg = Configuration()
    dirs = Directories(cfg.options['basedir'], cfg.options['efudir'], cfg.options['datadir'])
    searcher = Searcher(dirs)
    dialog = Dialog(cfg, dirs)

    e, d = searcher.get_values()
    dialog.populate(e, d)

    retval = dialog.exec_()
    if retval != 0:
        cmdlopts = []
        generator, data, efuip, throttle = dialog.get_selection()
        if efuip != "":
            cmdlopts += ['-i', efuip]
        if throttle != "":
            cmdlopts += ['-t', throttle]
        subprocess.call([generator, '-f', data ] + cmdlopts)
    sys.exit()
